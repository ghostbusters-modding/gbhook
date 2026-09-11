// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Level events, script faults, and loading a level by name: from a live level through the engine's pending
// globals, from the front end through the boot flow's own load sequence run on the pump.

namespace LevelFlow
{
    // The prepare detour and the script-fault logger. Needs gameBase and HookBroker::Init().
    void InstallFlowHooks();

    // `level` and `checkpoint`.
    void RegisterCommands();

    // Chain to `level`, a stem or a .lvl name. Never with a checkpoint: the engine loads and then drops the level.
    bool ChainToLevel(const char* level);
    bool LoadCheckpoint(const char* checkpoint);

    // A checkpoint armed with a front-end load makes the engine drop the level a second later: parked instead.
    void DeferCheckpoint(const char* checkpoint);
    bool FrontEndLoadPending();

    // Pump only. The first runs the whole level inside the call while the tick is parked; the second arms a
    // deferred checkpoint on the first live tick.
    void RunFrontEndLoadIfPending();
    void ArmDeferredCheckpoint();

    // The stem of the last prepared level, "" before any. Any thread.
    const char* CurrentLevel();
}
