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

    enum class Outcome { None, Pending, Mounted, InChain, Failed };

    // What happened to one mod's content, in a phrase for the log. "" for a mod without content.
    std::string Summary(const char* id);
    Outcome     OutcomeOf(const char* id);

    // The same as data, for the Mods page. Origin is how the cache POD came to be; None when there is none.
    enum class Origin { None, Cached, Built };
    struct Info
    {
        int     assetFiles = 0;
        Origin  origin     = Origin::None;
        Outcome outcome    = Outcome::None;
    };
    Info InfoOf(const char* id);
}
