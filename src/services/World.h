// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Whether the world ticks, the freeze mods hold, and the engine's own world-to-screen. view/Project is the math.

namespace World
{
    // Creates the holder lock. Bootstrap calls it before any mod can load.
    void Init();

    // GBH_PAUSED_* bits. On the engine thread the SCREEN bit is the engine's own predicate, elsewhere a memory read.
    int Paused();

    // `owner` is a mod id, or nullptr for the framework. Idempotent both ways; GbhStatus.
    int Pause(const char* owner, bool on);

    // 1 on screen, 0 behind or outside, GBH_ERR_STATE when the camera cannot be read.
    int ToScreen(const float pos[3], float out[2]);

    // Main thread, from the pump: puts the freeze byte back while anyone holds it.
    void Reassert();
}
