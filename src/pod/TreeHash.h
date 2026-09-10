// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// A stable hash of a mod's loose tree, so an unchanged mod never rebuilds its POD. tests/pod/test_content.cpp covers it.

#include <cstdint>
#include <string>
#include <vector>

namespace TreeHash
{
    // One loose file, as the Windows walk found it. `relpath` uses the engine's backslash separator.
    struct File
    {
        std::string relpath;
        uint64_t    size = 0;
        uint64_t    mtime = 0;   // whatever the OS reports; only equality across runs matters
    };

    // A hex digest of the set: sorted by relpath, then (relpath, size, mtime) folded in. Order-independent.
    std::string Of(std::vector<File> files);
}
