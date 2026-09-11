// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Read and extend the engine's mounted POD list. docs/CONTENT.md and engine/RE_POD_LOADING.md have the layout.
// Nothing in CPod locks and async loaders read the slots, so Mount is front-end-only, off the game thread.

namespace Pods
{
    // True once CPod::init has run. Quiet, so a per-tick poll leaves nothing in the log.
    bool Ready();

    // Dump the mounted slot table to the log. False only if the pod object itself could not be read.
    bool List();

    // Mount <gamedir>\<name> and its whole header chain through the engine's own CPod::mountPod, then re-sort.
    // `name` is relative to the game directory. On failure *err points at a static or engine reason.
    bool Mount(const char* name, const char** err);
}
