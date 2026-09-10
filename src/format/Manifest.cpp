// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Manifest.h"

#include <cstdio>
#include <cstring>

namespace Manifest
{
    std::string Field(const char* p, size_t cap)
    {
        size_t n = 0;
        while (n < cap && p[n] != '\0') ++n;
        return std::string(p, n);
    }

    std::string Validate(const GbhManifest& m)
    {
        if (memcmp(m.magic, GBH_MANIFEST_MAGIC, 8) != 0)
            return "manifest magic mismatch (stale SDK?)";

        if (m.abi_version != GBHOOK_ABI_VERSION)
        {
            char b[160];
            snprintf(b, sizeof b, "built for ABI %u, this gbhook speaks ABI %u -- rebuild the mod",
                     m.abi_version, (unsigned)GBHOOK_ABI_VERSION);
            return b;
        }

        // Smaller is an older SDK that appended nothing; larger knows fields we cannot honour.
        if (m.struct_size > sizeof(GbhManifest))
            return "manifest is newer than this framework";

        if (Field(m.id, sizeof m.id).empty())
            return "empty id in manifest";

        const std::string md5 = Field(m.target_md5, sizeof m.target_md5);
        if (md5 != GBHOOK_TARGET_MD5)
            return "built for ghost.exe " + (md5.empty() ? std::string("(unset)") : md5) +
                   ", this framework targets " GBHOOK_TARGET_MD5
                   " -- an offset table applied to the wrong build crashes unreadably";

        return "";
    }

    std::vector<std::string> ExclusiveHooks(const GbhManifest& m)
    {
        std::vector<std::string> out;
        for (int i = 0; i < GBH_MAX_EXCLUSIVE; ++i)
        {
            std::string s = Field(m.exclusive_hooks[i], sizeof m.exclusive_hooks[i]);
            if (s.empty()) break;
            out.push_back(s);
        }
        return out;
    }
}
