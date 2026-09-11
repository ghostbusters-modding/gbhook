// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The first fault in a callback names the mod and the event, drops that one subscription for good, and the game
// continues. Permanent on purpose: a callback that faulted on a null pointer faults again next frame.
#include "Events.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "bus/Bus.h"

#include <windows.h>
#include <string>

namespace
{
    constexpr long long kFrameBudgetUs = 250;   // per subscriber, averaged: the frame bus is on the engine's critical path
    constexpr int       kBudgetWindow  = 600;   // frames between reports

    const char* const kNames[Events::KindCount] = { "frame", "pump", "level", "actor" };

    Bus::Table*      g_bus     = nullptr;
    CRITICAL_SECTION g_lock;
    bool             g_ready   = false;
    long long        g_qpcFreq = 1;
    int              g_frames  = 0;

    void EnsureReady()
    {
        if (g_ready) return;
        InitializeCriticalSection(&g_lock);
        g_bus = new Bus::Table(Events::KindCount, kNames);
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        g_qpcFreq = f.QuadPart ? f.QuadPart : 1;
        g_ready = true;
    }

    // The table is read under the lock and dispatched outside it, so a callback may subscribe, unsubscribe or log.
    int Snapshot(int kind, Bus::Snap* out)
    {
        if (!g_ready) return 0;
        EnterCriticalSection(&g_lock);
        const int n = g_bus->Snapshot(kind, out, Bus::kMaxSubs);
        LeaveCriticalSection(&g_lock);
        return n;
    }

    void Kill(int kind, Bus::Handle h)
    {
        EnterCriticalSection(&g_lock);
        const std::string owner = g_bus->OwnerOf(h);
        g_bus->Kill(h);
        LeaveCriticalSection(&g_lock);
        Log::Writef("EVT", "FAULT in '%s' %s callback -- that subscription is off for this session; the game continues",
                    owner.c_str(), kNames[kind]);
    }

    void Charge(Bus::Handle h, long long ticks)
    {
        EnterCriticalSection(&g_lock);
        g_bus->Charge(h, ticks);
        LeaveCriticalSection(&g_lock);
    }

    void ReportBudget(int kind)
    {
        if (!g_ready) return;
        Bus::Over over[Bus::kMaxSubs];
        const long long budgetTicks = kFrameBudgetUs * g_qpcFreq / 1000000LL;
        EnterCriticalSection(&g_lock);
        const int n = g_bus->OverBudget(kind, budgetTicks, over, Bus::kMaxSubs);
        LeaveCriticalSection(&g_lock);
        for (int i = 0; i < n; ++i)
            Log::Writef("EVT", "BUDGET '%s' averages %lldus per %s over %d calls (soft limit %lldus) -- this is on the "
                               "engine's critical path",
                        over[i].owner, over[i].avgTicks * 1000000LL / g_qpcFreq, kNames[kind], over[i].calls,
                        kFrameBudgetUs);
    }

    // Plain-C frames for the guard.
    bool CallFrame(void* fn, void* user)
    {
        GBH_SEH_TRY { ((GbhFrameFn)fn)(user); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool CallLevel(void* fn, int phase, const char* level, int ok, void* user)
    {
        GBH_SEH_TRY { ((GbhLevelFn)fn)(phase, level, ok, user); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool CallActor(void* fn, const char* cls, const char* name, void* ptr, void* user)
    {
        GBH_SEH_TRY { ((GbhActorFn)fn)(cls, name, ptr, user); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // The two per-frame buses are timed; the others fire a few times per level and skip the accounting.
    void FireTimed(int kind)
    {
        Bus::Snap snap[Bus::kMaxSubs];
        const int n = Snapshot(kind, snap);
        for (int i = 0; i < n; ++i)
        {
            LARGE_INTEGER a, b;
            QueryPerformanceCounter(&a);
            const bool ok = CallFrame(snap[i].fn, snap[i].user);
            QueryPerformanceCounter(&b);
            if (!ok) Kill(kind, snap[i].h);
            else     Charge(snap[i].h, b.QuadPart - a.QuadPart);
        }
    }
}

namespace Events
{
    void Init() { EnsureReady(); }

    GbhSub Subscribe(Kind kind, const char* owner, void* fn, void* user)
    {
        EnsureReady();
        std::string why;
        EnterCriticalSection(&g_lock);
        Bus::Handle h = g_bus->Add(kind, owner, fn, user, &why);
        LeaveCriticalSection(&g_lock);
        if (!h) Log::Writef("EVT", "'%s' not subscribed: %s", (owner && *owner) ? owner : "gbhook", why.c_str());
        return static_cast<GbhSub>(h);
    }

    void Unsubscribe(GbhSub s)
    {
        if (!g_ready || !s) return;
        EnterCriticalSection(&g_lock);
        g_bus->Remove(s);
        LeaveCriticalSection(&g_lock);
    }

    void DisableOwner(const char* owner)
    {
        if (!g_ready || !owner) return;
        EnterCriticalSection(&g_lock);
        const int n = g_bus->DisableOwner(owner);
        LeaveCriticalSection(&g_lock);
        if (n) Log::Writef("EVT", "'%s' disabled: %d subscription(s) dropped", owner, n);
    }

    void FireFrame()
    {
        if (!g_ready) return;
        FireTimed(Frame);
        if (++g_frames >= kBudgetWindow) { g_frames = 0; ReportBudget(Frame); }
    }

    void FirePump() { FireTimed(Pump); }

    void FireLevel(GbhLevelPhase phase, const char* level, bool ok)
    {
        Bus::Snap snap[Bus::kMaxSubs];
        const int n = Snapshot(Level, snap);
        for (int i = 0; i < n; ++i)
            if (!CallLevel(snap[i].fn, (int)phase, level ? level : "", ok ? 1 : 0, snap[i].user)) Kill(Level, snap[i].h);
    }

    void FireActor(const char* cls, const char* name, void* ptr)
    {
        Bus::Snap snap[Bus::kMaxSubs];
        const int n = Snapshot(Actor, snap);
        for (int i = 0; i < n; ++i)
            if (!CallActor(snap[i].fn, cls, name, ptr, snap[i].user)) Kill(Actor, snap[i].h);
    }

    int LiveCount()
    {
        if (!g_ready) return 0;
        int n = 0;
        EnterCriticalSection(&g_lock);
        for (int k = 0; k < KindCount; ++k) n += g_bus->CountOf(k).live;
        LeaveCriticalSection(&g_lock);
        return n;
    }

    void LogSummary()
    {
        if (!g_ready) return;
        EnterCriticalSection(&g_lock);
        for (int k = 0; k < KindCount; ++k)
        {
            const Bus::Count c = g_bus->CountOf(k);
            if (c.live || c.faulted) Log::Writef("EVT", "%-6s %d live, %d faulted", kNames[k], c.live, c.faulted);
        }
        LeaveCriticalSection(&g_lock);
    }
}
