// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Target.h"
#include "gbhook/gbhook.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
    struct Key { uint32_t stamp, sizeOfImage, entry; int build; };

    constexpr Key kKeys[] = {
        { 0x5FA8FC04, 0x31CB000, 0x51A8B0, GBH_BUILD_STEAM },
        { 0x5F997696, 0x3122000, 0x519C90, GBH_BUILD_EPIC  },
    };

    const Target::Site kSites[] = {
#include "SiteTable.inc"
    };

    uint32_t U32(const uint8_t* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
}

namespace Target
{
    int Identify(const uint8_t* image, size_t n)
    {
        if (!image || n < 0x40 || image[0] != 'M' || image[1] != 'Z') return kUnknown;
        const uint32_t nt = U32(image + 0x3C);
        if (nt > n || n - nt < 24 + 0x40 || memcmp(image + nt, "PE\0\0", 4) != 0) return kUnknown;
        const uint32_t stamp = U32(image + nt + 8);
        const uint8_t* opt   = image + nt + 24;
        if (opt[0] != 0x0B || opt[1] != 0x02) return kUnknown;   // PE32+ only
        const uint32_t entry = U32(opt + 16), size = U32(opt + 56);
        for (const Key& k : kKeys)
            if (k.stamp == stamp && k.sizeOfImage == size && k.entry == entry) return k.build;
        return kUnknown;
    }

    const char* Name(int b) { return b == GBH_BUILD_STEAM ? "Steam" : b == GBH_BUILD_EPIC ? "Epic" : "unknown"; }
    const char* Md5(int b)  { return b == GBH_BUILD_STEAM ? GBHOOK_TARGET_MD5 : b == GBH_BUILD_EPIC ? GBHOOK_TARGET_MD5_EPIC : ""; }

    size_t      SiteCount()       { return sizeof kSites / sizeof kSites[0]; }
    const Site& SiteAt(size_t i)  { return kSites[i]; }
    uint32_t    RvaOf(const Site& s, int b)    { return b == GBH_BUILD_EPIC ? s.epic : b == GBH_BUILD_STEAM ? s.steam : 0; }
    const uint8_t* ExpectOf(const Site& s, int b) { return b == GBH_BUILD_EPIC ? s.epicBytes : s.steamBytes; }

    const Site* SiteCovering(uint32_t rva, int b)
    {
        for (const Site& s : kSites)
        {
            const uint32_t at = RvaOf(s, b);
            if (at && rva >= at && rva < at + kGuardBytes) return &s;
        }
        return nullptr;
    }

    const Site* SiteBySteam(uint32_t steam)
    {
        for (const Site& s : kSites)
            if (s.steam == steam) return &s;
        return nullptr;
    }

    bool BytesMatch(const uint8_t* expect, const uint8_t* now, const uint8_t* mask, size_t n)
    {
        for (size_t i = 0; i < n; ++i)
        {
            const uint8_t m = mask ? mask[i] : 0xFF;
            if ((expect[i] & m) != (now[i] & m)) return false;
        }
        return true;
    }

    bool GuardHolds(const Site& s, int b, const uint8_t* now)
    {
        return RvaOf(s, b) != 0 && BytesMatch(ExpectOf(s, b), now, nullptr, kGuardBytes);
    }

    bool ParseClaim(const std::string& claim, uint32_t* steam, uint32_t* epic)
    {
        const char* p = claim.c_str();
        if (strncmp(p, "ghost+0x", 8) != 0) return false;
        char* end = nullptr;
        const unsigned long s = strtoul(p + 8, &end, 16);
        if (end == p + 8 || s == 0) return false;
        unsigned long e = 0;
        if (*end == '|')
        {
            const char* q = end + 1;
            if (strncmp(q, "0x", 2) != 0) return false;
            e = strtoul(q + 2, &end, 16);
            if (end == q + 2 || e == 0) return false;
        }
        if (*end != '\0') return false;
        if (steam) *steam = (uint32_t)s;
        if (epic)  *epic  = (uint32_t)e;
        return true;
    }

    std::string ClaimKey(const std::string& claim)
    {
        uint32_t s = 0;
        if (!ParseClaim(claim, &s, nullptr)) return claim;
        char b[32];
        snprintf(b, sizeof b, "ghost+0x%X", (unsigned)s);
        return b;
    }

    bool TranslateClaim(const std::string& claim, int b, uint32_t* rva)
    {
        uint32_t s = 0, e = 0;
        if (!ParseClaim(claim, &s, &e)) return false;
        uint32_t out = 0;
        if (b == GBH_BUILD_STEAM) out = s;
        else if (b == GBH_BUILD_EPIC)
        {
            // A claim that disagrees with gbhook's own table names some other site: refuse it.
            const Site* site = SiteBySteam(s);
            if (site && e && e != site->epic) return false;
            out = site ? site->epic : e;
        }
        if (!out) return false;
        if (rva) *rva = out;
        return true;
    }
}
