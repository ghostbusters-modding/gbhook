// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Guarded reads of the engine's globals. Any thread; a pointer handed back is live game memory, game thread only.

#include <cstddef>

namespace Game
{
    void* Singleton();       // CGame*, or nullptr very early
    void* LocalPlayer();     // nullptr in menus and during loads

    // The current level stem into `out`, "" when there is none.
    void  LevelStem(char* out, size_t n);
}
