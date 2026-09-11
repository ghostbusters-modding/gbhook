// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The reference pump detour without its bus, queue drain and front-end loads: those arrive with 4.3 and 4.4.

#include <windows.h>
#include <cstring>

#include "Pump.h"
#include "Commands.h"
#include "Events.h"
#include "FrameHook.h"
#include "LevelFlow.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "../core/Seh.h"
#include "gb/HookTargets.h"

namespace
{
    constexpr int      kMaxJobs = 8;
    constexpr unsigned kGiveUp  = 600;   // ticks before an unfinished job is dropped, about ten seconds of frames

    struct Parked { Pump::Job fn; void* user; const char* what; unsigned ticks; };

    CRITICAL_SECTION g_lock;
    bool             g_lockReady = false;
    Parked           g_jobs[kMaxJobs];
    volatile LONG    g_jobCount  = 0;
    bool             g_installed = false;
    volatile DWORD   g_thread    = 0;

    // Four pointer-sized arguments forwarded whatever the true signature is, the hedge the reference took.
    typedef void (__fastcall* tPump)(void*, void*, void*, void*);
    tPump oPump = nullptr;

    void EnsureLock()
    {
        if (g_lockReady) return;
        InitializeCriticalSection(&g_lock);
        g_lockReady = true;
    }

    // A plain-C frame for the guard. 1 done, 0 again next tick, -1 faulted.
    int CallJob(Pump::Job fn, void* user)
    {
        GBH_SEH_TRY { return fn(user) ? 1 : 0; }
        GBH_SEH_EXCEPT { return -1; }
    }

    // Jobs run outside the lock, so one may park another.
    void RunJobs()
    {
        Parked batch[kMaxJobs];
        int    n;
        EnterCriticalSection(&g_lock);
        n = g_jobCount;
        memcpy(batch, g_jobs, sizeof(Parked) * (size_t)n);
        g_jobCount = 0;
        LeaveCriticalSection(&g_lock);

        for (int i = 0; i < n; ++i)
        {
            Parked&   j = batch[i];
            const int r = CallJob(j.fn, j.user);
            if (r == 1) continue;
            if (r == -1) { Log::Writef("PUMP", "%s faulted on the main thread; dropped", j.what); continue; }
            if (++j.ticks >= kGiveUp)
            {
                Log::Writef("PUMP", "%s still not done after %u ticks; dropped", j.what, j.ticks);
                continue;
            }
            EnterCriticalSection(&g_lock);
            const LONG k = g_jobCount;
            if (k < kMaxJobs) { g_jobs[k] = j; g_jobCount = k + 1; }
            LeaveCriticalSection(&g_lock);
        }
    }

    void __fastcall PumpDetour(void* a, void* b, void* c, void* d)
    {
        if (oPump) oPump(a, b, c, d);
        if (!g_thread)
        {
            g_thread = GetCurrentThreadId();
            Log::Writef("PUMP", "first tick, main thread %lu", (unsigned long)g_thread);
        }
        if (g_jobCount) RunJobs();
        Events::FirePump();

        // While the game tick is parked this is the main thread's only visit, so it runs what the tick would have.
        if (!FrameHook::TickRecently())
        {
            Commands::Drain();
            LevelFlow::RunFrontEndLoadIfPending();
        }
    }
}

namespace Pump
{
    void Install()
    {
        EnsureLock();
        if (g_installed || !gameBase) return;

        GbhHook h = HookBroker::Install(nullptr, gameBase + HookTargets::pump, (void*)&PumpDetour,
                                        (void**)&oPump, GBH_HOOK_EXCLUSIVE);
        if (!h) { Log::Write("PUMP", "pump hook FAILED to install; nothing parked on it will run"); return; }
        g_installed = true;
        Log::Write("PUMP", "engine pump hooked: main thread, front end included");
    }

    bool Installed() { return g_installed; }

    bool Park(Job fn, void* user, const char* what)
    {
        EnsureLock();
        if (!what) what = "job";
        if (!fn) return false;
        if (!g_installed)
        {
            Log::Writef("PUMP", "%s cannot run: the pump is not hooked", what);
            return false;
        }

        bool ok = false;
        EnterCriticalSection(&g_lock);
        const LONG k = g_jobCount;
        if (k < kMaxJobs)
        {
            Parked& p = g_jobs[k];
            p.fn = fn; p.user = user; p.what = what; p.ticks = 0;
            g_jobCount = k + 1;
            ok = true;
        }
        LeaveCriticalSection(&g_lock);

        if (!ok) Log::Writef("PUMP", "%s refused: %d jobs already parked", what, kMaxJobs);
        return ok;
    }

    unsigned long ThreadId()  { return g_thread; }
    bool          IsEngineThread() { return g_thread != 0 && GetCurrentThreadId() == g_thread; }
}
