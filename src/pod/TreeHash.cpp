// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "TreeHash.h"

#include <algorithm>
#include <cstdio>

namespace
{
    // FNV-1a, 64-bit. Not cryptographic: this only has to notice a changed tree, not resist an attacker.
    void Fold(uint64_t& h, const void* p, size_t n)
    {
        const uint8_t* b = (const uint8_t*)p;
        for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 0x100000001b3ULL; }
    }
}

namespace TreeHash
{
    std::string Of(std::vector<File> files)
    {
        std::sort(files.begin(), files.end(), [](const File& a, const File& b) { return a.relpath < b.relpath; });

        uint64_t h = 0xcbf29ce484222325ULL;
        for (const File& f : files)
        {
            Fold(h, f.relpath.data(), f.relpath.size());
            Fold(h, &f.size, sizeof f.size);
            Fold(h, &f.mtime, sizeof f.mtime);
            uint8_t sep = 0;
            Fold(h, &sep, 1);
        }
        char b[17];
        snprintf(b, sizeof b, "%016llx", (unsigned long long)h);
        return b;
    }
}
