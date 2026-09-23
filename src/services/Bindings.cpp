// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// A press is queued on the message thread and fired from the pump, so an action runs where engine calls are safe.
// A fault turns that one action off for the process, the same policy as an event subscription.
#include "Bindings.h"
#include "Commands.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "input/BindTable.h"

#include <windows.h>
#include <cstring>
#include <string>

namespace
{
    constexpr int kRing = 64;

    BindTable::Table* g_table = nullptr;
    CRITICAL_SECTION  g_lock;
    bool              g_ready = false;
    int               g_ring[kRing];
    int               g_head  = 0;
    int               g_count = 0;
    bool              g_dropLogged = false;

    void EnsureReady()
    {
        if (g_ready) return;
        InitializeCriticalSection(&g_lock);
        g_table = new BindTable::Table();
        g_ready = true;
    }

    const char* Shown(const char* owner) { return (owner && *owner) ? owner : "gbhook"; }

    // A plain-C frame for the guard.
    bool CallAction(GbhActionFn fn, const char* name, void* user)
    {
        GBH_SEH_TRY { fn(name, user); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    int CmdBinds(int, const char* const*, const char**, void*)
    {
        EnterCriticalSection(&g_lock);
        const int n = g_table->Count();
        Log::Writef("RES", "%d action(s):", n);
        for (int i = 0; i < n; ++i)
        {
            const BindTable::Claim* c = g_table->At(i);
            const std::string chord = KeyChord::ToText(c->key);
            const bool off = !c->enabled || c->faulted;
            Log::Writef("RES", "  %-24s %-20s %-18s%s %s", Shown(c->owner), c->name,
                        chord.empty() ? "(unbound)" : chord.c_str(), off ? " off" : "    ", c->help);
        }
        LeaveCriticalSection(&g_lock);
        return GBH_OK;
    }
}

namespace Bindings
{
    void Init() { EnsureReady(); }

    int Register(const char* owner, const char* name, GbhActionFn fn, void* user, const char* help)
    {
        EnsureReady();
        if (!fn || !BindTable::ValidName(name)) return GBH_ERR_ARG;

        const std::string     key  = std::string("bind.") + name;
        const char*           text = Settings::GetFor(owner, key.c_str(), "");
        KeyChord::Key         chord;
        const KeyChord::Parse p    = KeyChord::FromText(text, &chord);

        EnterCriticalSection(&g_lock);
        const BindTable::AddResult r = g_table->Add(owner, name, help, chord, (void*)fn, user);
        char holder[BindTable::kOwnerCap + BindTable::kNameCap + 1] = { 0 };
        if (r.clashWith >= 0)
        {
            const BindTable::Claim* c = g_table->At(r.clashWith);
            _snprintf_s(holder, sizeof holder, _TRUNCATE, "%s %s", Shown(c->owner), c->name);
        }
        LeaveCriticalSection(&g_lock);

        switch (r.status)
        {
        case BindTable::Add::Taken:
            Log::WritefFrom(owner, "BIND", "%s is already registered; the second registration is refused", name);
            return GBH_ERR_CONFLICT;
        case BindTable::Add::Full:
            Log::WritefFrom(owner, "BIND", "%s refused: all %d action slots are taken", name, BindTable::kMaxClaims);
            return GBH_ERR;
        case BindTable::Add::BadName:
            return GBH_ERR_ARG;
        case BindTable::Add::Ok:
            break;
        }

        if (p == KeyChord::Parse::Bad)
            Log::WritefFrom(owner, "BIND", "%s: '%s' is not a key or chord; unbound", name, text);
        else if (p == KeyChord::Parse::None && !*text)
            Log::WritefFrom(owner, "BIND", "%s unbound (no bind.%s)", name, name);
        else if (r.clashWith >= 0)
            Log::WritefFrom(owner, "BIND", "%s wants %s, held by %s; unbound", name, KeyChord::ToText(chord).c_str(), holder);
        else if (p == KeyChord::Parse::Ok)
            Log::WritefFrom(owner, "BIND", "%s = %s", name, KeyChord::ToText(chord).c_str());
        return GBH_OK;
    }

    int Binding(const char* owner, const char* name, char* out, int cap)
    {
        if (!name || !out || cap <= 0) return GBH_ERR_ARG;
        EnsureReady();
        EnterCriticalSection(&g_lock);
        const int i = g_table->Find(owner, name);
        const std::string text = i >= 0 ? KeyChord::ToText(g_table->At(i)->key) : std::string();
        LeaveCriticalSection(&g_lock);
        if (i < 0) return GBH_ERR_NOT_FOUND;
        strncpy_s(out, (size_t)cap, text.c_str(), _TRUNCATE);
        return (int)text.size() < cap ? GBH_OK : GBH_ERR_TRUNCATED;
    }

    int Held(const char* owner, const char* name)
    {
        if (!name) return GBH_ERR_ARG;
        EnsureReady();
        EnterCriticalSection(&g_lock);
        const int  i    = g_table->Find(owner, name);
        const bool held = i >= 0 && g_table->Held(i);
        LeaveCriticalSection(&g_lock);
        if (i < 0) return GBH_ERR_NOT_FOUND;
        return held ? 1 : 0;
    }

    int Enable(const char* owner, const char* name, bool on)
    {
        if (!name) return GBH_ERR_ARG;
        EnsureReady();
        EnterCriticalSection(&g_lock);
        const int i = g_table->Find(owner, name);
        if (i >= 0) g_table->Enable(i, on);
        LeaveCriticalSection(&g_lock);
        return i >= 0 ? GBH_OK : GBH_ERR_NOT_FOUND;
    }

    int Capture(const char* owner, bool on)
    {
        EnsureReady();
        EnterCriticalSection(&g_lock);
        const bool had = g_table->CapturedBy(owner);
        const bool ok  = g_table->Capture(owner, on);
        LeaveCriticalSection(&g_lock);
        if (!ok) return GBH_ERR_CONFLICT;
        if (had != on) Log::WriteFrom(owner, "BIND", on ? "keyboard captured" : "keyboard released");
        return GBH_OK;
    }

    bool OnKey(int vk, bool down)
    {
        if (!g_ready) return false;
        bool dropped = false, consume;
        EnterCriticalSection(&g_lock);
        if (down)
        {
            const BindTable::Edge e = g_table->Down(vk);
            consume = e.consume;
            if (e.fired >= 0)
            {
                if (g_count < kRing) { g_ring[(g_head + g_count) % kRing] = e.fired; ++g_count; }
                else if (!g_dropLogged) { g_dropLogged = true; dropped = true; }
            }
        }
        else consume = g_table->Up(vk);
        LeaveCriticalSection(&g_lock);
        if (dropped) Log::Writef("BIND", "%d presses queued and the pump has not run; further presses dropped", kRing);
        return consume;
    }

    void OnFocusLost()
    {
        if (!g_ready) return;
        EnterCriticalSection(&g_lock);
        g_table->ReleaseAll();
        LeaveCriticalSection(&g_lock);
    }

    void Flush()
    {
        if (!g_ready || !g_count) return;
        struct Press { int index; GbhActionFn fn; void* user; char owner[BindTable::kOwnerCap]; char name[BindTable::kNameCap]; };
        Press batch[kRing];
        int   n = 0;
        EnterCriticalSection(&g_lock);
        for (; g_count > 0; --g_count, g_head = (g_head + 1) % kRing)
        {
            const int i = g_ring[g_head];
            if (!g_table->Live(i)) continue;   // switched off or captured away since the press
            const BindTable::Claim* c = g_table->At(i);
            Press& p = batch[n++];
            p.index = i;
            p.fn    = (GbhActionFn)c->fn;
            p.user  = c->user;
            memcpy(p.owner, c->owner, sizeof p.owner);
            memcpy(p.name, c->name, sizeof p.name);
        }
        LeaveCriticalSection(&g_lock);

        for (int k = 0; k < n; ++k)
        {
            if (CallAction(batch[k].fn, batch[k].name, batch[k].user)) continue;
            EnterCriticalSection(&g_lock);
            g_table->Kill(batch[k].index);
            LeaveCriticalSection(&g_lock);
            Log::Writef("BIND", "FAULT in '%s' action %s; off for this session", Shown(batch[k].owner), batch[k].name);
        }
    }

    void RegisterCommands()
    {
        Commands::Register(nullptr, "binds", CmdBinds, nullptr, "every action, its chord and its help", Commands::kAnyThread);
    }
}
