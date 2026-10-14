// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Which ghost.exe is running, the guarded sites per build, and hook claims across builds. tests/target is the spec.

#include <cstddef>
#include <cstdint>
#include <string>

namespace Target
{
    constexpr int kUnknown = -1;   // otherwise GBH_BUILD_STEAM or GBH_BUILD_EPIC

    // Keyed on the PE header, not a file hash: three fields read in place cost nothing at attach.
    int Identify(const uint8_t* image, size_t n);
    const char* Name(int build);
    const char* Md5(int build);

    constexpr size_t kGuardBytes = 16;

    struct Site
    {
        const char* name;
        uint32_t    steam;
        uint32_t    epic;   // 0: no such site on Epic
        uint8_t     steamBytes[kGuardBytes];
        uint8_t     epicBytes[kGuardBytes];
    };

    size_t      SiteCount();
    const Site& SiteAt(size_t i);
    uint32_t    RvaOf(const Site& s, int build);
    const uint8_t* ExpectOf(const Site& s, int build);

    // The site whose guard window holds `rva` in this build, or nullptr.
    const Site* SiteCovering(uint32_t rva, int build);
    const Site* SiteBySteam(uint32_t steam);

    // `now` is kGuardBytes read at the site. A null mask compares every byte.
    bool BytesMatch(const uint8_t* expect, const uint8_t* now, const uint8_t* mask, size_t n);
    bool GuardHolds(const Site& s, int build, const uint8_t* now);

    // "ghost+0xSTEAM" or "ghost+0xSTEAM|0xEPIC". False when it is not that shape.
    bool ParseClaim(const std::string& claim, uint32_t* steam, uint32_t* epic);
    // The Steam form, the key two claims are compared by. Unparsed text comes back as it is.
    std::string ClaimKey(const std::string& claim);
    // The claim's RVA in `build`: from the site table, else from the claim's own Epic part. False: no Epic site.
    bool TranslateClaim(const std::string& claim, int build, uint32_t* rva);
}
