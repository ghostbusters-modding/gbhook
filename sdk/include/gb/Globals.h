// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Engine globals gbhook reads, ghost-relative, { Steam, Epic }. Detour sites: HookTargets.h.

#include <cstdint>
#include "gb/Build.h"

namespace Globals
{
    // CGame* gGame. Null very early in boot.
    static constexpr GbAddr gGame = { 0xDCF680, 0xD2B680 };

    // CGhostbuster* the local player. Null in menus and while a level loads; callers treat that as normal.
    static constexpr GbAddr localPlayer = { 0x2322AD8, 0x227A888 };

    // CGame + 0x08: the current level stem, "firehouse", NUL-terminated in place. "" at the front end.
    static constexpr uintptr_t cgameLevelStem = 0x08;

    // The 600-byte scan-code table the WM_KEYDOWN handler writes and the game reads, and its index mask word.
    static constexpr GbAddr scanTable     = { 0x2523F00, 0x247BCB0 };
    static constexpr GbAddr scanTableMask = { 0xDD6774, 0xD32764 };

    // The physics wrapper. Its first qword is the scene, whose gravity vector sits at +0x20/+0x24/+0x28.
    // resetGravity writes (0, -32, 0), so the engine's normal is -32.
    static constexpr GbAddr physicsWrapper = { 0xDD4FE0, 0xD30FD0 };
    static constexpr uintptr_t sceneGravity   = 0x20;

    // The time block. setTimeFactor writes the ramp target to +0x318 and sets the u8 at +0x32C.
    // A clear byte means the engine's own curve, so the target is stale and the factor reads as 1.
    static constexpr GbAddr timeBlock    = { 0xDD67A0, 0xD32790 };
    static constexpr uintptr_t timeTarget   = 0x318;
    static constexpr uintptr_t timeOverride = 0x32C;

    // CGame + 0x4A9C1: the u8 the level loop tests to skip the world update and the game clock. 
    // Epic is 0x10 lower, like every CGame field from +0x4A598.
    static constexpr GbAddr cgameFreeze = { 0x4A9C1, 0x4A9B1 };

    // The front-end manager, and the engine's "a screen is up" predicate on it. Read only: forcing it blacks the frame.
    static constexpr GbAddr feManager      = { 0xDD14D0, 0xD2D438 };
    static constexpr GbAddr feScreenActive = { 0x246E70, 0x246C90 };
    static constexpr uintptr_t feGate         = 0x1D9;

    // The active front-end controller, and its u8 active bit. The pause screen sets it.
    static constexpr GbAddr feController       = { 0x20B11B8, 0x2008F78 };
    static constexpr uintptr_t feControllerActive = 0x09;

    // u8, non-zero while the game window has lost focus.
    static constexpr GbAddr focusLost = { 0x2523EA4, 0x247BC54 };

    // The renderer's camera: the matrix-stack top pointer, projection scales, near plane and backbuffer size.
    static constexpr GbAddr matrixTop   = { 0x236EE10, 0x22C6BC0 };
    static constexpr GbAddr projScaleX  = { 0x236BB54, 0x22C3904 };
    static constexpr GbAddr projScaleY  = { 0x236BB58, 0x22C3908 };
    static constexpr GbAddr nearPlane   = { 0x236EDA0, 0x22C6B50 };
    static constexpr GbAddr backbufferW = { 0xDD674C, 0xD3273C };
    static constexpr GbAddr backbufferH = { 0xDD6750, 0xD32740 };
}
