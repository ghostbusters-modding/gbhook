// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The framework's detour on the engine's per-frame pump, the main thread's only per-frame visit at the front end.
// Work that has to wait for the engine, such as the content mount, is parked here and run from the first ticks on.

namespace Pump
{
    // Needs gameBase and HookBroker::Init(). If the detour will not install it logs, and nothing parked here runs.
    void Install();
    bool Installed();

    // Called on the main thread every tick until it returns true. After Install. `what` names it in the log.
    typedef bool (*Job)(void* user);
    bool Park(Job fn, void* user, const char* what);

    // The main thread's id once the detour has seen a tick, zero before. The game tick runs on this thread too.
    unsigned long ThreadId();
    bool          IsEngineThread();
}
