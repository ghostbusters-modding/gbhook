// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "HookBroker.h"
#include "Framework.h"
#include "registry/PatchRegistry.h"
#include "ProcessMemory.h"
#include "registry/VtableRegistry.h"

#include <cstdio>
#include <cstring>
#include <vector>
#include <MinHook.h>

namespace
{
    // Enough to cover the longest prologue MinHook relocates on x64.
    constexpr int kFingerprint = 20;

    struct Entry
    {
        void*   target;
        void*   detour;
        char    owner[64];
        uint8_t bytes[kFingerprint];   // as written by MinHook, post-enable
        bool    enabled;
        bool    exclusive;
        bool    baselined;
    };

    std::vector<Entry>* g_hooks   = nullptr;
    ProcessMemory*      g_mem     = nullptr;
    PatchRegistry*      g_patches = nullptr;
    VtableRegistry*     g_vtables = nullptr;
    CRITICAL_SECTION    g_lock;
    bool                g_ready   = false;

    const char* Who(const char* owner) { return (owner && *owner) ? owner : "gbhook"; }

    // "ghost+0x1F1210" inside the image, an absolute address anywhere else.
    const char* Where(const void* p, char* buf, size_t cap)
    {
        const uintptr_t at = (uintptr_t)p, base = (uintptr_t)gameBase;
        if (base && at >= base && at - base < 0x8000000)
            snprintf(buf, cap, "ghost+0x%llX", (unsigned long long)(at - base));
        else
            snprintf(buf, cap, "%p", p);
        return buf;
    }

    Entry* Find(void* target)
    {
        for (auto& e : *g_hooks)
            if (e.target == target) return &e;
        return nullptr;
    }

    // Handles are 1-based indices: a zeroed handle is never valid, and the vector may reallocate freely.
    Entry* FromHandle(GbhHook h)
    {
        size_t i = reinterpret_cast<size_t>(h);
        if (i == 0 || i > g_hooks->size()) return nullptr;
        return &(*g_hooks)[i - 1];
    }

    GbhHook ToHandle(size_t index1) { return reinterpret_cast<GbhHook>(index1); }

    void Baseline(Entry& e)
    {
        e.baselined = g_mem->Read((uintptr_t)e.target, e.bytes, kFingerprint);
    }

    // The detour whose relocated prologue touches [at, at + n), or nullptr.
    const Entry* DetourOverlapping(uintptr_t at, size_t n)
    {
        for (auto& e : *g_hooks)
        {
            const uintptr_t t = (uintptr_t)e.target;
            if (at < t + kFingerprint && t < at + n) return &e;
        }
        return nullptr;
    }

    void OnPatchDrift(const PatchRegistry::Patch& p, const uint8_t* now, void* ctx)
    {
        char w[48];
        Log::Writef("HOOK", "TAMPER patch %s (owner '%s') reads %02X, wrote %02X -- checked after %s",
                    Where((void*)p.at, w, sizeof w), p.owner, now[0], p.written[0], (const char*)ctx);
    }

    void OnVtableDrift(const char* what, uintptr_t at, const char* owner, void* ctx)
    {
        char w[48];
        Log::Writef("HOOK", "TAMPER vtable %s at %s (owner '%s') changed since install -- checked after %s",
                    what, Where((void*)at, w, sizeof w), owner, (const char*)ctx);
    }
}

namespace HookBroker
{
    bool Init()
    {
        if (g_ready) return true;

        MH_STATUS st = MH_Initialize();
        if (st != MH_OK && st != MH_ERROR_ALREADY_INITIALIZED)
        {
            Log::Writef("HOOK", "MH_Initialize failed (%d) -- no hooks will install", (int)st);
            return false;
        }

        InitializeCriticalSection(&g_lock);
        g_hooks = new std::vector<Entry>();
        g_hooks->reserve(32);
        g_mem     = new ProcessMemory();
        g_patches = new PatchRegistry(*g_mem);
        g_vtables = new VtableRegistry(*g_mem);
        g_ready   = true;
        Log::Write("HOOK", "MinHook initialised (gbhook is the sole owner)");
        return true;
    }

    GbhHook Install(const char* owner, void* target, void* detour, void** original, uint32_t flags)
    {
        if (!g_ready || !target || !detour) return nullptr;
        const char* who = Who(owner);
        char w[48];

        EnterCriticalSection(&g_lock);

        if (Entry* prior = Find(target))
        {
            // Either side asserting exclusivity makes this a conflict. Naming both parties is the point.
            if (prior->exclusive || (flags & GBH_HOOK_EXCLUSIVE))
                Log::Writef("HOOK", "CONFLICT %s: '%s' wants it, '%s' already owns it -- refusing "
                                    "(declare it in your manifest)", Where(target, w, sizeof w), who, prior->owner);
            else
                Log::Writef("HOOK", "%s already hooked by '%s'; '%s' refused (one detour per address -- "
                                    "subscribe to an event instead)", Where(target, w, sizeof w), prior->owner, who);
            LeaveCriticalSection(&g_lock);
            return nullptr;
        }

        if (const PatchRegistry::Patch* p = g_patches->Overlapping((uintptr_t)target, kFingerprint))
        {
            Log::Writef("HOOK", "CONFLICT %s: '%s' wants a detour where '%s' holds a byte patch -- refusing",
                        Where(target, w, sizeof w), who, p->owner);
            LeaveCriticalSection(&g_lock);
            return nullptr;
        }

        MH_STATUS st = MH_CreateHook(target, detour, original);
        if (st != MH_OK)
        {
            Log::Writef("HOOK", "MH_CreateHook %s failed (%d) for '%s'", Where(target, w, sizeof w), (int)st, who);
            LeaveCriticalSection(&g_lock);
            return nullptr;
        }

        Entry e;
        memset(&e, 0, sizeof e);
        e.target    = target;
        e.detour    = detour;
        e.exclusive = (flags & GBH_HOOK_EXCLUSIVE) != 0;
        strncpy_s(e.owner, who, _TRUNCATE);

        if (!(flags & GBH_HOOK_DEFERRED))
        {
            st = MH_EnableHook(target);
            if (st != MH_OK)
            {
                MH_RemoveHook(target);
                Log::Writef("HOOK", "MH_EnableHook %s failed (%d) for '%s'", Where(target, w, sizeof w), (int)st, who);
                LeaveCriticalSection(&g_lock);
                return nullptr;
            }
            e.enabled = true;
            Baseline(e);
        }

        g_hooks->push_back(e);
        size_t index1 = g_hooks->size();

        Log::Writef("HOOK", "%s %s by '%s'%s",
                    e.enabled ? "installed" : "created (deferred)",
                    Where(target, w, sizeof w), who, e.exclusive ? " [exclusive]" : "");

        LeaveCriticalSection(&g_lock);
        return ToHandle(index1);
    }

    bool Enable(GbhHook h)
    {
        if (!g_ready) return false;
        EnterCriticalSection(&g_lock);
        Entry* e = FromHandle(h);
        bool ok = false;
        if (e && !e->enabled && MH_EnableHook(e->target) == MH_OK)
        {
            e->enabled = true;
            Baseline(*e);
            ok = true;
        }
        LeaveCriticalSection(&g_lock);
        return ok;
    }

    bool Disable(GbhHook h)
    {
        if (!g_ready) return false;
        EnterCriticalSection(&g_lock);
        Entry* e = FromHandle(h);
        bool ok = false;
        if (e && e->enabled && MH_DisableHook(e->target) == MH_OK)
        {
            e->enabled   = false;
            e->baselined = false;
            ok = true;
        }
        LeaveCriticalSection(&g_lock);
        return ok;
    }

    bool EnableBatch(const GbhHook* hooks, int count)
    {
        if (!g_ready || !hooks || count <= 0) return false;
        EnterCriticalSection(&g_lock);

        int queued = 0;
        for (int i = 0; i < count; ++i)
        {
            Entry* e = FromHandle(hooks[i]);
            if (e && !e->enabled && MH_QueueEnableHook(e->target) == MH_OK) ++queued;
        }

        bool ok = (MH_ApplyQueued() == MH_OK);
        if (ok)
        {
            for (int i = 0; i < count; ++i)
                if (Entry* e = FromHandle(hooks[i]))
                    if (!e->enabled) { e->enabled = true; Baseline(*e); }
        }
        Log::Writef("HOOK", "batch enable: %d queued, %s", queued, ok ? "applied" : "FAILED");

        LeaveCriticalSection(&g_lock);
        return ok;
    }

    const char* OwnerOf(void* target)
    {
        if (!g_ready) return nullptr;
        EnterCriticalSection(&g_lock);
        Entry* e = Find(target);
        const char* r = e ? e->owner : nullptr;
        LeaveCriticalSection(&g_lock);
        return r;
    }

    int Count() { return g_ready ? (int)g_hooks->size() : 0; }

    GbhPatch PatchWrite(const char* owner, void* at, const void* bytes, size_t n)
    {
        if (!g_ready || !at) return nullptr;
        const char* who = Who(owner);
        char w[48];
        EnterCriticalSection(&g_lock);

        if (const Entry* e = DetourOverlapping((uintptr_t)at, n))
        {
            Log::Writef("HOOK", "CONFLICT patch %s: '%s' wants it inside the prologue '%s' detoured at %p -- refusing",
                        Where(at, w, sizeof w), who, e->owner, e->target);
            LeaveCriticalSection(&g_lock);
            return nullptr;
        }

        std::string why;
        int h = g_patches->Write(who, (uintptr_t)at, bytes, n, &why);
        if (h)
            Log::Writef("HOOK", "patched %s, %u byte(s), by '%s'", Where(at, w, sizeof w), (unsigned)n, who);
        else
            Log::Writef("HOOK", "patch %s refused for '%s': %s", Where(at, w, sizeof w), who, why.c_str());

        LeaveCriticalSection(&g_lock);
        return reinterpret_cast<GbhPatch>((size_t)h);
    }

    bool PatchRevert(GbhPatch p)
    {
        if (!g_ready) return false;
        EnterCriticalSection(&g_lock);
        std::string why;
        const int h = (int)reinterpret_cast<size_t>(p);
        const PatchRegistry::Patch* rec = g_patches->Get(h);
        const bool ok = g_patches->Revert(h, &why);
        char w[48];
        if (rec)
            Log::Writef("HOOK", "revert %s by '%s': %s", Where((void*)rec->at, w, sizeof w), rec->owner,
                        ok ? "done" : why.c_str());
        LeaveCriticalSection(&g_lock);
        return ok;
    }

    void* VtableClone(const char* owner, void* original, int slots)
    {
        if (!g_ready) return nullptr;
        const char* who = Who(owner);
        char w[48];
        EnterCriticalSection(&g_lock);
        std::string why;
        const int before = g_vtables->CloneCount();
        uintptr_t c = g_vtables->Clone(who, (uintptr_t)original, slots, &why);
        if (!c)
            Log::Writef("HOOK", "vtable %s not cloned for '%s': %s", Where(original, w, sizeof w), who, why.c_str());
        else if (g_vtables->CloneCount() != before)
            Log::Writef("HOOK", "vtable %s cloned (%d slots) for '%s' at %p", Where(original, w, sizeof w), slots, who, (void*)c);
        LeaveCriticalSection(&g_lock);
        return (void*)c;
    }

    bool VtableSlot(const char* owner, void* clone, int slot, void* fn, void** originalFn)
    {
        if (!g_ready) return false;
        const char* who = Who(owner);
        EnterCriticalSection(&g_lock);
        std::string why;
        uintptr_t orig = 0;
        const bool ok = g_vtables->SetSlot(who, (uintptr_t)clone, slot, (uintptr_t)fn, &orig, &why);
        if (ok)
        {
            if (originalFn) *originalFn = (void*)orig;
            Log::Writef("HOOK", "vtable copy %p slot %d -> %p by '%s'", clone, slot, fn, who);
        }
        else
            Log::Writef("HOOK", "vtable copy %p slot %d refused for '%s': %s", clone, slot, who, why.c_str());
        LeaveCriticalSection(&g_lock);
        return ok;
    }

    bool VtableApply(const char* owner, void* object, void* clone)
    {
        if (!g_ready) return false;
        const char* who = Who(owner);
        char w[48];
        EnterCriticalSection(&g_lock);
        std::string why;
        const bool ok = g_vtables->Apply(who, (uintptr_t)object, (uintptr_t)clone, &why);
        if (ok)
            Log::Writef("HOOK", "vtable of %s -> copy %p by '%s'", Where(object, w, sizeof w), clone, who);
        else
            Log::Writef("HOOK", "vtable of %s not swapped for '%s': %s", Where(object, w, sizeof w), who, why.c_str());
        LeaveCriticalSection(&g_lock);
        return ok;
    }

    void* VtableOriginal(void* clone, int slot)
    {
        if (!g_ready) return nullptr;
        EnterCriticalSection(&g_lock);
        void* r = (void*)g_vtables->Original((uintptr_t)clone, slot);
        LeaveCriticalSection(&g_lock);
        return r;
    }

    int VerifyAll(const char* why)
    {
        if (!g_ready) return 0;
        const char* when = why ? why : "?";
        EnterCriticalSection(&g_lock);

        int drift = 0;
        for (auto& e : *g_hooks)
        {
            if (!e.enabled || !e.baselined) continue;
            uint8_t now[kFingerprint];
            const bool readable = g_mem->Read((uintptr_t)e.target, now, kFingerprint);
            if (!readable || memcmp(now, e.bytes, kFingerprint) != 0)
            {
                ++drift;
                char w[48];
                Log::Writef("HOOK", "TAMPER %s (owner '%s') changed since install -- checked after %s. "
                                    "Something else is patching this address; expect undefined behaviour.",
                            Where(e.target, w, sizeof w), e.owner, when);
            }
        }
        drift += g_patches->Verify(OnPatchDrift, (void*)when);
        drift += g_vtables->Verify(OnVtableDrift, (void*)when);

        if (drift == 0)
            Log::Writef("HOOK", "integrity ok: %d hook(s), %d patch(es), %d vtable copy(ies) verified after %s",
                        (int)g_hooks->size(), g_patches->Count(), g_vtables->CloneCount(), when);

        LeaveCriticalSection(&g_lock);
        return drift;
    }
}
