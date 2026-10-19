// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// One engine address per ghost.exe build, Steam then Epic, picked by GbhPluginBuild at each use.

#include <cstdint>
#include "../gbhook/gbhook.h"

struct GbAddr
{
    uintptr_t steam;
    uintptr_t epic;   // 0: the build has no such site, so check present() first

    constexpr GbAddr(uintptr_t s, uintptr_t e) : steam(s), epic(e) {}

    uintptr_t get() const { return GbhPluginBuild == GBH_BUILD_EPIC ? epic : steam; }
    bool present() const { return get() != 0; }
    operator uintptr_t() const { return get(); }
};
