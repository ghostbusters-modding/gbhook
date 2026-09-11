// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The per-level tick, hooked once: it defines the game thread and fires the frame bus. Not at the front end.
// The pump (services/Pump) is the same thread's visit when no level is live, which is why IsEngineThread covers both.

namespace FrameHook
{
    // Needs gameBase and HookBroker::Init(). Logs and leaves the engine alone if the detour cannot install.
    void Install();
    bool Installed();

    // True once the detour has seen a frame: a game thread exists to queue onto.
    bool Active();
    // True while the tick has run within the last 250 ms: a level is live right now. Paused levels still tick.
    bool TickRecently();

    unsigned long GameThreadId();     // 0 until the first frame
    bool          IsGameThread();
    bool          IsEngineThread();   // the game thread, or the pump's main thread before any level
}
