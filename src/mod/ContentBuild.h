// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Boot-time content: build each mod's loose tree into a cached POD, decide what to mount, stand down for GBMM.
// The Windows glue over the pure pod/ and modset/ units. Runs on its own thread and mounts through the pump.

#include <string>

namespace ContentBuild
{
    // Walk every accepted mod's assets, scan the PATCH.POD chain once, build or reuse each cache POD.
    void Start();

    // True once the build has finished and every archive it produced has been mounted or refused.
    bool Done();

    // What happened to one mod's content, in a phrase for the Mods page. "" for a mod without content.
    std::string Summary(const char* id);
}
