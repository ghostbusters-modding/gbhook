// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Boot-time content: build each mod's loose tree into a cached POD, decide what to mount, stand down for GBMM.
// The Windows glue over the pure pod/ and modset/ units. Mount() does the mounts; the caller times it off the pump.

#include <string>
#include <vector>

namespace ContentBuild
{
    struct Planned
    {
        std::string id;
        std::string cachePod;   // <gamedir>-relative path to the cache POD, ready for Pods::Mount
    };

    // Walk every accepted mod's loose assets, scan the PATCH.POD chain once, build or reuse each cache POD,
    // and log the verdict. Returns the archives that should be mounted, in code order. No engine call here.
    const std::vector<Planned>& Build();

    // The result of the last Build(), for whoever performs the mounts.
    const std::vector<Planned>& Plans();

    // Mount every planned archive through Pods::Mount. Main thread at the front end, and once only.
    // False means the engine's pod object is not up yet: call again next tick.
    bool Mount();

    // What happened to one mod's content, in a phrase for the Mods page. "" for a mod without content.
    const char* Summary(const char* id);
}
