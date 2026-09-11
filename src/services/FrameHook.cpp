// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "FrameHook.h"
#include "Events.h"
#include "Pump.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "gb/HookTargets.h"

#include <windows.h>

namespace
{
    volatile LONG g_gameThread = 0;
    volatile LONG g_lastTickMs = 0;
    volatile LONG g_firstSeen  = 0;
    bool          g_installed  = false;

    // Decompile-verified to read one argument; four are forwarded as the hedge the reference took.
    typedef void (__fastcall* tTick)(void*, void*, void*, void*);
    tTick oTick = nullptr;

    void __fastcall TickDetour(void* a, void* b, void* c, void* d)
    {
        InterlockedExchange(&g_gameThread, (LONG)GetCurrentThreadId());
        InterlockedExchange(&g_lastTickMs, (LONG)GetTickCount());
        if (InterlockedCompareExchange(&g_firstSeen, 1, 0) == 0)
            Log::Writef("FRAME", "first frame on thread %lu -- the frame bus is live", GetCurrentThreadId());

        Events::FireFrame();
        if (oTick) oTick(a, b, c, d);
    }
}

namespace FrameHook
{
    void Install()
    {
        if (g_installed || !gameBase) return;
        GbhHook h = HookBroker::Install(nullptr, gameBase + HookTargets::gameTick, (void*)&TickDetour,
                                        (void**)&oTick, GBH_HOOK_EXCLUSIVE);
        if (!h) { Log::Write("FRAME", "tick hook FAILED to install; on_frame will never fire"); return; }
        g_installed = true;
        Log::Write("FRAME", "per-level tick hooked (frame bus)");
    }

    bool Installed() { return g_installed; }
    bool Active()    { return g_installed && InterlockedCompareExchange(&g_firstSeen, 1, 1) == 1; }

    bool TickRecently()
    {
        if (!Active()) return false;
        const DWORD last = (DWORD)InterlockedCompareExchange(&g_lastTickMs, 0, 0);
        return (GetTickCount() - last) < 250;
    }

    unsigned long GameThreadId() { return (unsigned long)InterlockedCompareExchange(&g_gameThread, 0, 0); }

    bool IsGameThread()
    {
        const unsigned long id = GameThreadId();
        return id != 0 && id == GetCurrentThreadId();
    }

    bool IsEngineThread() { return IsGameThread() || Pump::IsEngineThread(); }
}
