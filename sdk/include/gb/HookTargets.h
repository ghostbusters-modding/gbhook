// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The engine addresses gbhook and its mods detour, ghost-relative, { Steam, Epic }.
// The framework owns the contended ones and re-broadcasts them as events; a mod names one here to claim it.

#include <cstdint>
#include "gb/Build.h"

namespace HookTargets
{
    // Per-level tick, void __fastcall(CGame*), once per frame from the per-level loop. Defines the game thread.
    static constexpr GbAddr gameTick = { 0x1F1210, 0x1F11A0 };

    // Dante VM global registration, void(char* "CGhostbuster Egon", object, a2, a3). Game thread, during load.
    static constexpr GbAddr exportGlobalVariable = { 0x2CED00, 0x2CEB10 };

    // Level flow: chainToLevel writes the pending-level globals, the loader consumes them.
    static constexpr GbAddr chainToLevelImpl = { 0x1EC8B0, 0x1EC8B0 };
    static constexpr GbAddr levelLoader      = { 0x1EF790, 0x1EF750 };
    static constexpr GbAddr perLevelLoop     = { 0x1F0070, 0x1F0030 };

    // Script fault reporter. Logging it turns a silent script failure into a named one.
    static constexpr GbAddr GTFO = { 0x2D11C0, 0x2D0FD0 };

    // Front-end menu dispatch, how the game's own main-menu rows are claimed.
    static constexpr GbAddr frontEndMenu = { 0x245BE0, 0x245A00 };

    // Level prepare. Its return is the moment a level's object table is stamped and its actors are addressable.
    static constexpr GbAddr levelPrepare = { 0x1EED30, 0x1EECF0 };
    // Begin-level's profile sync, void __fastcall(CGame*): after the script registered its checkpoints
    static constexpr GbAddr levelBeginSync = { 0x27C8F0, 0x27C710 };

    // The per-frame network pump, CTRINetwork vtable+0x18: the main thread's only per-frame visit at the front end.
    static constexpr GbAddr pump = { 0x3CA3A0, 0x3CA1B0 };

    // The menu loop's action poll, __int64 __fastcall(feMgr*), every front-end frame. 5 = load the pending level.
    static constexpr GbAddr frontEndAction = { 0x247860, 0x247680 };

    // Cold-boot screens. Each returns "screen is done", so a detour returning 0 skips it. mods/FastBoot claims both.
    static constexpr GbAddr epilepsyScreen = { 0x248190, 0x247FB0 };
    static constexpr GbAddr legalScreen    = { 0x2487E0, 0x248600 };

    // The GENERAL movie player, cutscenes included. Only its "video\logo" argument is the boot reel.
    static constexpr GbAddr playMovie = { 0x1F45F0, 0x1F4580 };

    // CGame vtable+0x390, the only achievement unlock: Steam and the save flag both. gbhook patches it to ret.
    // Epic 0: the block is for Steam only.
    static constexpr GbAddr achievementUnlock = { 0x1F3DF0, 0x0 };

    // Clears one entry of the 600-entry key table, char __fastcall(input, int code). Bounds-checked: a bad code fast-fails.
    static constexpr GbAddr clearKey = { 0x403320, 0x403130 };

    // GPU scene profiler: 5 x 40 timestamp slots, never bounds-checked. Its frame end re-enables it every present.
    static constexpr GbAddr gpuProfiler       = { 0x265AFC0, 0x25B2D70 };
    static constexpr GbAddr gpuProfilerEnable = { 0x3E69F8, 0x3E6808 };   // mov byte [r14+8], 1; gbhook patches the 1 to 0
}
