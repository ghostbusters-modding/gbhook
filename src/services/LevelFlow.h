// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Level events and script faults. The prepare call's return is the moment a level's actors are addressable.
// Front-end level loads and checkpoints arrive with the harness slice.

namespace LevelFlow
{
    // The prepare detour and the script-fault logger. Needs gameBase and HookBroker::Init().
    void InstallFlowHooks();

    // The stem of the last prepared level, "" before any. Any thread.
    const char* CurrentLevel();
}
