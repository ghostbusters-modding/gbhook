// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Level events, script faults, and loading a level by name: from a live level through the engine's pending
// globals, from the front end by answering the menu loop's action poll with the engine's own "load pending".

namespace LevelFlow
{
    // The prepare detour and the script-fault logger. Needs gameBase and HookBroker::Init().
    void InstallFlowHooks();

    // `level` and `checkpoint`.
    void RegisterCommands();

    // Chain to `level`, a stem or a .lvl name. A checkpoint goes through DeferCheckpoint or LoadCheckpoint.
    bool ChainToLevel(const char* level);

    // Reload the live level at `checkpoint`, a script name or a display name it registers.
    bool LoadCheckpoint(const char* checkpoint);

    // Queue a checkpoint for the next level's begin
    void DeferCheckpoint(const char* checkpoint);
    bool FrontEndLoadPending();

    // The stem of the last prepared level, "" before any. Any thread.
    const char* CurrentLevel();
}
