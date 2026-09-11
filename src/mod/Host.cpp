// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Host.h"
#include "Api.h"
#include "Discovery.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "../core/Seh.h"
#include "../services/Events.h"
#include "../services/NativeMenu.h"

#include <windows.h>
#include <vector>

namespace
{
    struct Entry
    {
        const ModSet::Record* rec;
        std::string           dir;      // <root>\<folder>\gbhook
        HMODULE               module;
        Host::Status          status;
    };

    std::vector<Entry>* g_entries = nullptr;

    void Ensure()
    {
        if (g_entries) return;
        g_entries = new std::vector<Entry>();
        for (const ModSet::Record& r : Mods::Result().records)
        {
            Entry e;
            e.rec       = &r;
            e.dir       = Mods::GbhookDir(r);
            e.module    = nullptr;
            e.status.id = r.mod.id;
            if (r.disabled)      { e.status.state = Host::State::Off;    e.status.note = "disabled in mod.ini"; }
            else if (!r.accepted) { e.status.state = Host::State::Failed; e.status.note = r.refusal; }
            g_entries->push_back(e);
        }
    }

    Entry* ById(const char* id)
    {
        if (!id) return nullptr;
        for (Entry& e : *g_entries)
            if (_stricmp(e.status.id.c_str(), id) == 0) return &e;
        return nullptr;
    }

    Entry* ByModule(const void* addr)
    {
        if (!addr || !g_entries) return nullptr;
        HMODULE m = nullptr;
        if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                (LPCSTR)addr, &m) || !m)
            return nullptr;
        for (Entry& e : *g_entries)
            if (e.module && e.module == m) return &e;
        return nullptr;
    }

    // A plain-C frame for the guard: a fault inside a mod's init disables that mod, not the game.
    int CallInit(GbhPluginInitFn fn, const GbhApi* api, bool* faulted)
    {
        *faulted = false;
        GBH_SEH_TRY { return fn(api); }
        GBH_SEH_EXCEPT { *faulted = true; return GBH_ERR; }
    }

    void Fail(Entry& e, const std::string& why)
    {
        e.status.state = Host::State::Failed;
        e.status.note  = why;
        Log::Writef("MODS", "FAIL %s: %s -- the mod is inert", e.status.id.c_str(), why.c_str());
    }
}

namespace Host
{
    int Init(GbhStage stage)
    {
        Ensure();
        int up = 0, seen = 0;
        for (Entry& e : *g_entries)
        {
            const ModSet::Record& r = *e.rec;
            if (!r.accepted || r.mod.stage != stage || e.status.state != State::Pending) continue;
            ++seen;

            if (r.mod.plugin.empty())
            {
                e.status.state = State::NoCode;
                ++up;
                continue;
            }

            const std::string path = e.dir + "\\" + r.mod.plugin;
            // Altered search path, so a helper DLL beside the mod's own resolves before anything in the game folder.
            e.module = LoadLibraryExA(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (!e.module)
            {
                char b[96];
                _snprintf_s(b, sizeof b, _TRUNCATE, "LoadLibrary failed (error %lu; a missing dependency?)", GetLastError());
                Fail(e, b);
                continue;
            }

            GbhPluginInitFn init = (GbhPluginInitFn)GetProcAddress(e.module, GBH_EXPORT_INIT);
            if (!init) { Fail(e, "the DLL has no " GBH_EXPORT_INIT " export"); continue; }

            bool faulted = false;
            const int rc = CallInit(init, Api::Table(), &faulted);
            if (faulted) { Fail(e, "faulted inside " GBH_EXPORT_INIT); continue; }
            if (rc != GBH_OK)
            {
                char b[64];
                _snprintf_s(b, sizeof b, _TRUNCATE, GBH_EXPORT_INIT " returned %d", rc);
                Fail(e, b);
                continue;
            }

            e.status.state = State::Loaded;
            ++up;
            Log::Writef("MODS", "loaded %s %s at %s", r.mod.id.c_str(), r.mod.version.c_str(), ModIni::StageName(stage));

            // A mod that brought its own MinHook, or patched a target we own, shows up here as byte drift.
            HookBroker::VerifyAll(r.mod.id.c_str());
        }
        Log::Writef("MODS", "stage %s: %d of %d mod(s) up", ModIni::StageName(stage), up, seen);
        return up;
    }

    void Disable(const char* id, const char* why)
    {
        Ensure();
        Entry* e = ById(id);
        if (!e || e->status.state == State::Failed) return;
        e->status.state = State::Failed;
        e->status.note  = why ? why : "disabled";
        Events::DisableOwner(e->status.id.c_str());
        NativeMenu::DropOwner(e->status.id.c_str());
        Log::Writef("MODS", "DISABLED %s: %s", e->status.id.c_str(), e->status.note.c_str());
    }

    const char* OwnerOfAddress(const void* addr)
    {
        Entry* e = ByModule(addr);
        return e ? e->status.id.c_str() : nullptr;
    }

    const char* DirOfAddress(const void* addr)
    {
        Entry* e = ByModule(addr);
        return e ? e->dir.c_str() : nullptr;
    }

    int Count() { Ensure(); return (int)g_entries->size(); }

    const char* IdAt(int i)
    {
        Ensure();
        if (i < 0 || i >= (int)g_entries->size()) return nullptr;
        return (*g_entries)[(size_t)i].status.id.c_str();
    }

    const Status* StatusOf(const char* id)
    {
        Ensure();
        Entry* e = ById(id);
        return e ? &e->status : nullptr;
    }

    bool IsLoaded(const char* id)
    {
        Ensure();
        Entry* e = ById(id);
        return e && (e->status.state == State::Loaded || e->status.state == State::NoCode);
    }

    void LogStatus()
    {
        Ensure();
        Log::Writef("MODS", "gbhook %s, %d hook(s), %d mod(s)", Framework::kVersionString, HookBroker::Count(),
                    (int)g_entries->size());
        for (const Entry& e : *g_entries)
        {
            const char* state = e.status.state == State::Loaded  ? "loaded"  :
                                e.status.state == State::NoCode  ? "no code" :
                                e.status.state == State::Off     ? "off"     :
                                e.status.state == State::Failed  ? "FAILED"  : "pending";
            Log::Writef("MODS", "  %-24s %-8s %-8s %s", e.status.id.c_str(), e.rec->mod.version.c_str(), state,
                        e.status.note.c_str());
        }
    }
}
