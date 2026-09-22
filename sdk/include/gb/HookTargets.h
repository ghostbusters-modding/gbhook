// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The engine addresses gbhook and its mods detour, ghost.exe 0b89556c07e5b737efe444351227e747, ghost-relative.
// The framework owns the contended ones and re-broadcasts them as events; a mod names one here to claim it.

#include <cstdint>

namespace HookTargets
{
    // Per-level tick, void __fastcall(CGame*), once per frame from the per-level loop. Defines the game thread.
    static constexpr uintptr_t gameTick = 0x1F1210;

    // Dante VM global registration, void(char* "CGhostbuster Egon", object, a2, a3). Game thread, during load.
    static constexpr uintptr_t exportGlobalVariable = 0x2CED00;

    // Level flow: chainToLevel writes the pending-level globals, the loader consumes them.
    static constexpr uintptr_t chainToLevelImpl = 0x1EC8B0;
    static constexpr uintptr_t levelLoader      = 0x1EF790;
    static constexpr uintptr_t perLevelLoop     = 0x1F0070;

    // Script fault reporter. Logging it turns a silent script failure into a named one.
    static constexpr uintptr_t GTFO = 0x2D11C0;

    // Front-end menu dispatch, how the game's own main-menu rows are claimed.
    static constexpr uintptr_t frontEndMenu = 0x245BE0;

    // Level prepare. Its return is the moment a level's object table is stamped and its actors are addressable.
    static constexpr uintptr_t levelPrepare = 0x1EED30;
    // Begin-level's profile sync, void __fastcall(CGame*): after the script registered its checkpoints
    static constexpr uintptr_t levelBeginSync = 0x27C8F0;

    // The per-frame network pump, CTRINetwork vtable+0x18: the main thread's only per-frame visit at the front end.
    static constexpr uintptr_t pump = 0x3CA3A0;

    // The menu loop's action poll, __int64 __fastcall(feMgr*), every front-end frame. 5 = load the pending level.
    static constexpr uintptr_t frontEndAction = 0x247860;

    // Cold-boot screens. Each returns "screen is done", so a detour returning 0 skips it. mods/FastBoot claims both.
    static constexpr uintptr_t epilepsyScreen = 0x248190;
    static constexpr uintptr_t legalScreen    = 0x2487E0;

    // The GENERAL movie player, cutscenes included. Only its "video\logo" argument is the boot reel.
    static constexpr uintptr_t playMovie = 0x1F45F0;
}
