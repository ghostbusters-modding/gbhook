// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// DirectInput key names and the engine's own scan-table index for one. Pure; tests/input/test_dik.cpp is the spec.
// The engine indexes its table by make code with the extended flag in bit 8, masked by a live word: 0x7F or 0x1FF.

namespace Dik
{
    // The DIK_* code for a name such as "W", "ENTER", "LSHIFT", "UP", "F5", any case. -1 for an unknown name.
    int FromName(const char* name);

    // The canonical name for a code, or nullptr.
    const char* Name(int dik);

    // Where the engine's WM_KEYDOWN writer puts a key: DIK bit 7 becomes bit 8, then the engine's mask applies.
    // An extended key under the 0x7F mask lands on its base code, which is what the engine itself sees.
    int TableIndex(int dik, unsigned mask);

    constexpr int      kTableEntries = 600;
    constexpr unsigned kMaskDefault  = 0x7F;
    constexpr unsigned kMaskExtended = 0x1FF;
}
