// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for target/Target and gb/Build.h: build detection, GbAddr, guard bytes and claims across builds.

#include "check.h"
#include "target/Target.h"
#include "gb/Build.h"
#include "gb/HookTargets.h"

#include <cstring>
#include <vector>

extern "C" { int GbhPluginBuild = GBH_BUILD_STEAM; }

namespace
{
    void Put32(std::vector<uint8_t>& b, size_t at, uint32_t v)
    {
        for (int i = 0; i < 4; ++i) b[at + i] = (uint8_t)(v >> (8 * i));
    }

    // A PE32+ header with the three fields Identify reads.
    std::vector<uint8_t> Header(uint32_t stamp, uint32_t size, uint32_t entry)
    {
        std::vector<uint8_t> b(0x1000, 0);
        b[0] = 'M'; b[1] = 'Z';
        Put32(b, 0x3C, 0x100);
        memcpy(&b[0x100], "PE\0\0", 4);
        Put32(b, 0x108, stamp);
        b[0x118] = 0x0B; b[0x119] = 0x02;
        Put32(b, 0x118 + 16, entry);
        Put32(b, 0x118 + 56, size);
        return b;
    }

    const Target::Site& Named(const char* name)
    {
        for (size_t i = 0; i < Target::SiteCount(); ++i)
            if (strcmp(Target::SiteAt(i).name, name) == 0) return Target::SiteAt(i);
        static Target::Site none{};
        return none;
    }
}

int main()
{
    // ---- build detection --------------------------------------------------
    {
        auto s = Header(0x5FA8FC04, 0x31CB000, 0x51A8B0);
        auto e = Header(0x5F997696, 0x3122000, 0x519C90);
        CHECK_EQ(Target::Identify(s.data(), s.size()), GBH_BUILD_STEAM);
        CHECK_EQ(Target::Identify(e.data(), e.size()), GBH_BUILD_EPIC);

        // One field off is another build: refused, never guessed.
        auto x = Header(0x5FA8FC04, 0x31CB000, 0x51A8B4);
        CHECK_EQ(Target::Identify(x.data(), x.size()), Target::kUnknown);
        auto y = Header(0x5F997696, 0x31CB000, 0x519C90);
        CHECK_EQ(Target::Identify(y.data(), y.size()), Target::kUnknown);

        auto pe32 = s; pe32[0x118] = 0x0B; pe32[0x119] = 0x01;
        CHECK_EQ(Target::Identify(pe32.data(), pe32.size()), Target::kUnknown);
        auto nomz = s; nomz[0] = 'X';
        CHECK_EQ(Target::Identify(nomz.data(), nomz.size()), Target::kUnknown);
        auto far = s; Put32(far, 0x3C, 0xFF0);
        CHECK_EQ(Target::Identify(far.data(), far.size()), Target::kUnknown);
        CHECK_EQ(Target::Identify(s.data(), 0x120), Target::kUnknown);
        CHECK_EQ(Target::Identify(nullptr, 0x1000), Target::kUnknown);

        CHECK_EQ(std::string(Target::Name(GBH_BUILD_EPIC)), "Epic");
        CHECK_EQ(std::string(Target::Md5(GBH_BUILD_STEAM)), GBHOOK_TARGET_MD5);
        CHECK_EQ(std::string(Target::Md5(GBH_BUILD_EPIC)), GBHOOK_TARGET_MD5_EPIC);
        CHECK_EQ(std::string(Target::Md5(Target::kUnknown)), "");
    }

    // ---- GbAddr picks by the build index ----------------------------------
    {
        constexpr GbAddr a = { 0x1F1210, 0x1F11A0 };
        constexpr GbAddr steamOnly = { 0x1F3DF0, 0 };
        static_assert(a.steam == 0x1F1210 && a.epic == 0x1F11A0, "constexpr");

        GbhPluginBuild = GBH_BUILD_STEAM;
        CHECK_EQ((uintptr_t)a, (uintptr_t)0x1F1210);
        CHECK(steamOnly.present());
        char base[16] = { 0 };
        CHECK_EQ((uintptr_t)((base + a) - base), (uintptr_t)0x1F1210);
        CHECK_EQ((uintptr_t)HookTargets::gameTick, (uintptr_t)0x1F1210);

        GbhPluginBuild = GBH_BUILD_EPIC;
        CHECK_EQ((uintptr_t)a, (uintptr_t)0x1F11A0);
        CHECK_EQ(a.get(), (uintptr_t)0x1F11A0);
        CHECK(!steamOnly.present());
        CHECK_EQ((uintptr_t)HookTargets::gameTick, (uintptr_t)0x1F11A0);
        CHECK(!HookTargets::achievementUnlock.present());
        CHECK_EQ((uintptr_t)(0x1000 + a), (uintptr_t)0x1F21A0);

        GbhPluginBuild = GBH_BUILD_STEAM;
    }

    // ---- the site table ---------------------------------------------------
    {
        CHECK_EQ(Target::SiteCount(), (size_t)20);
        for (size_t i = 0; i < Target::SiteCount(); ++i)
        {
            const Target::Site& s = Target::SiteAt(i);
            uint8_t zero[Target::kGuardBytes] = { 0 };
            CHECK(s.steam != 0);
            CHECK(memcmp(s.steamBytes, zero, sizeof zero) != 0);
            if (s.epic) CHECK(memcmp(s.epicBytes, zero, sizeof zero) != 0);
        }
        // The 0.2.2 Epic fault: the menu refill entry moved, and 0x20E900 is mid-function there.
        CHECK_EQ(Named("menuRefill").epic, (uint32_t)0x20E730);
        CHECK_EQ(Named("achievementUnlock").epic, (uint32_t)0);
        CHECK(Target::SiteCovering(0x20E900, GBH_BUILD_EPIC) == nullptr);
        CHECK(Target::SiteCovering(0x20E730, GBH_BUILD_EPIC) == &Named("menuRefill"));
        CHECK(Target::SiteCovering(0x20E73F, GBH_BUILD_EPIC) == &Named("menuRefill"));
        CHECK(Target::SiteCovering(0x20E740, GBH_BUILD_EPIC) == nullptr);
        CHECK(Target::SiteCovering(0x1F3DF0, GBH_BUILD_STEAM) == &Named("achievementUnlock"));
        CHECK(Target::SiteCovering(0x1F3DF0, GBH_BUILD_EPIC) == nullptr);
        CHECK(Target::SiteBySteam(0x1F1210) == &Named("gameTick"));
        CHECK(Target::SiteBySteam(0x1F1211) == nullptr);
        CHECK_EQ(Target::RvaOf(Named("gameTick"), Target::kUnknown), (uint32_t)0);
        // The table and the SDK header agree.
        CHECK_EQ((uintptr_t)Named("gameTick").epic, HookTargets::gameTick.epic);
        CHECK_EQ((uintptr_t)Named("pump").epic, HookTargets::pump.epic);
    }

    // ---- guard bytes: the same code, a rel32 field that differs -----------
    {
        // gameTick: push rbx; sub rsp,140h; mov rax,[rip+disp32]. Only the disp32 at +12 differs.
        const Target::Site& s = Named("gameTick");
        CHECK(memcmp(s.steamBytes, s.epicBytes, 12) == 0);
        CHECK(memcmp(s.steamBytes + 12, s.epicBytes + 12, 4) != 0);

        CHECK(Target::GuardHolds(s, GBH_BUILD_STEAM, s.steamBytes));
        CHECK(Target::GuardHolds(s, GBH_BUILD_EPIC, s.epicBytes));
        CHECK(!Target::GuardHolds(s, GBH_BUILD_EPIC, s.steamBytes));
        CHECK(!Target::GuardHolds(s, GBH_BUILD_STEAM, s.epicBytes));

        // A hook already in place (jmp rel32) is not the shipped code.
        uint8_t hooked[Target::kGuardBytes];
        memcpy(hooked, s.epicBytes, sizeof hooked);
        hooked[0] = 0xE9;
        CHECK(!Target::GuardHolds(s, GBH_BUILD_EPIC, hooked));

        // A mask that skips the disp32 accepts either build's bytes.
        uint8_t mask[Target::kGuardBytes];
        memset(mask, 0xFF, sizeof mask);
        memset(mask + 12, 0, 4);
        CHECK(Target::BytesMatch(s.steamBytes, s.epicBytes, mask, sizeof mask));
        CHECK(!Target::BytesMatch(s.steamBytes, s.epicBytes, nullptr, sizeof mask));
        CHECK(!Target::BytesMatch(s.steamBytes, hooked, mask, sizeof mask));

        // No Epic site: nothing holds.
        const Target::Site& a = Named("achievementUnlock");
        CHECK(Target::GuardHolds(a, GBH_BUILD_STEAM, a.steamBytes));
        CHECK(!Target::GuardHolds(a, GBH_BUILD_EPIC, a.epicBytes));
    }

    // ---- claims -----------------------------------------------------------
    {
        uint32_t s = 0, e = 0;
        CHECK(Target::ParseClaim("ghost+0x1F1210", &s, &e));
        CHECK_EQ(s, (uint32_t)0x1F1210);
        CHECK_EQ(e, (uint32_t)0);
        CHECK(Target::ParseClaim("ghost+0x46a110|0x469D80", &s, &e));
        CHECK_EQ(s, (uint32_t)0x46A110);
        CHECK_EQ(e, (uint32_t)0x469D80);
        CHECK(!Target::ParseClaim("ghost+0x", &s, &e));
        CHECK(!Target::ParseClaim("ghost+0x12|", &s, &e));
        CHECK(!Target::ParseClaim("ghost+0x12|0x", &s, &e));
        CHECK(!Target::ParseClaim("ghost+0x12 ", &s, &e));
        CHECK(!Target::ParseClaim("ghost+0x0", &s, &e));
        CHECK(!Target::ParseClaim("0x1F1210", &s, &e));

        CHECK_EQ(Target::ClaimKey("ghost+0x46a110|0x469D80"), "ghost+0x46A110");
        CHECK_EQ(Target::ClaimKey("ghost+0x46A110"), "ghost+0x46A110");
        CHECK_EQ(Target::ClaimKey("something else"), "something else");

        uint32_t r = 0;
        CHECK(Target::TranslateClaim("ghost+0x46A110", GBH_BUILD_STEAM, &r));
        CHECK_EQ(r, (uint32_t)0x46A110);
        // A site in gb/HookTargets.h translates with no Epic part.
        CHECK(Target::TranslateClaim("ghost+0x248190", GBH_BUILD_EPIC, &r));
        CHECK_EQ(r, (uint32_t)0x247FB0);
        CHECK(Target::TranslateClaim("ghost+0x248190|0x247FB0", GBH_BUILD_EPIC, &r));
        CHECK(!Target::TranslateClaim("ghost+0x248190|0x247FB4", GBH_BUILD_EPIC, &r));
        // A mod's own site needs its Epic part.
        CHECK(!Target::TranslateClaim("ghost+0x46A110", GBH_BUILD_EPIC, &r));
        CHECK(Target::TranslateClaim("ghost+0x46A110|0x469D80", GBH_BUILD_EPIC, &r));
        CHECK_EQ(r, (uint32_t)0x469D80);
        // A Steam-only site never translates.
        CHECK(!Target::TranslateClaim("ghost+0x1F3DF0", GBH_BUILD_EPIC, &r));
        CHECK(!Target::TranslateClaim("ghost+0x1F3DF0|0x1F3D80", GBH_BUILD_EPIC, &r));
        CHECK(!Target::TranslateClaim("ghost+0x1F1210", Target::kUnknown, &r));
    }

    return check::Done("target");
}
