// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Engine globals gbhook reads, ghost.exe 0b89556c07e5b737efe444351227e747, ghost-relative. Detour sites: HookTargets.h.

#include <cstdint>

namespace Globals
{
    // CGame* gGame. Null very early in boot.
    static constexpr uintptr_t gGame = 0xDCF680;

    // CGhostbuster* the local player. Null in menus and while a level loads; callers treat that as normal.
    static constexpr uintptr_t localPlayer = 0x2322AD8;

    // CGame + 0x08: the current level stem, "firehouse", NUL-terminated in place. "" at the front end.
    static constexpr uintptr_t cgameLevelStem = 0x08;

    // The 600-byte scan-code table the WM_KEYDOWN handler writes and the game reads, and its index mask word.
    static constexpr uintptr_t scanTable     = 0x2523F00;
    static constexpr uintptr_t scanTableMask = 0xDD6774;
}
