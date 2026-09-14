// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Catalogue.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace
{
    using namespace Catalogue;

    // Objective engine state only: every row has a field to read, and a writable one an engine native to call.
    const Entry kEntries[] = {
        { kGod,     "god",     GBH_ATTR_BOOL,  true,  false,    0.0f,  0.0f, "",    "invulnerable (CCharacter::setInvulnerableFlag)" },
        { kGiant,   "giant",   GBH_ATTR_BOOL,  true,  false,    0.0f,  0.0f, "",    "giant boss mode (CGhostbuster::enableGiantBossMode)" },
        { kTorpedo, "torpedo", GBH_ATTR_BOOL,  true,  false,    0.0f,  0.0f, "",    "proton torpedo (CGhostbuster::enableProtonTorpedo)" },
        { kHunt,    "hunt",    GBH_ATTR_BOOL,  true,  false,    0.0f,  0.0f, "",    "the pack's hunt flag (CGhostbuster::toggleHuntMode)" },
        { kGravity, "gravity", GBH_ATTR_FLOAT, true,  true,  -100.0f, 50.0f, "",    "gravity y, -32 is the engine's normal (setGravity, reset: resetGravity)" },
        { kTime,    "time",    GBH_ATTR_FLOAT, true,  true,     0.05f,  4.0f, "x",   "time factor, ramped over 0.25 s (setTimeFactor, reset: 1.00x)" },
        { kFov,     "fov",     GBH_ATTR_FLOAT, false, false,    0.0f,  0.0f, "deg", "the main view's field of view" },
        { kCamDist, "camdist", GBH_ATTR_FLOAT, false, false,    0.0f,  0.0f, "",    "the main view's follow arm length" },
        { kCamMode, "cammode", GBH_ATTR_INT,   false, false,    0.0f,  0.0f, "",    "the script camera mode: 0 normal, 10 path, 11 fixed, 13 orbit" },
    };
    constexpr int kCount = (int)(sizeof kEntries / sizeof kEntries[0]);
    static_assert(kCount <= kMaxEntries, "the catalogue outgrew its cap");

    bool IEquals(const char* a, const char* b)
    {
        for (;; ++a, ++b)
        {
            const int x = toupper((unsigned char)*a), y = toupper((unsigned char)*b);
            if (x != y) return false;
            if (!x) return true;
        }
    }

    bool AnyOf(const char* v, const char* a, const char* b, const char* c) { return IEquals(v, a) || IEquals(v, b) || IEquals(v, c); }

    // The whole word must be the number: "12abc" and "" are refused, and so is anything non-finite.
    bool Number(const char* s, float* out)
    {
        if (!s || !*s) return false;
        char* end = nullptr;
        const double d = strtod(s, &end);
        if (end == s || *end) return false;
        if (!(d > -1.0e30 && d < 1.0e30)) return false;
        *out = (float)d;
        return true;
    }
}

namespace Catalogue
{
    int Count() { return kCount; }

    const Entry* At(int i) { return (i >= 0 && i < kCount) ? &kEntries[i] : nullptr; }

    const Entry* Find(const char* key)
    {
        if (!key || !*key) return nullptr;
        for (const Entry& e : kEntries) if (IEquals(e.key, key)) return &e;
        return nullptr;
    }

    bool Parse(const Entry& e, const char* value, float current, Parsed* out, const char** why)
    {
        const char* unused = nullptr;
        if (!why) why = &unused;
        *why = nullptr;
        out->reset = false;
        out->value = 0.0f;
        if (!value) { *why = "no value"; return false; }

        if (IEquals(value, "reset"))
        {
            if (!e.resettable) { *why = "no reset for this key"; return false; }
            out->reset = true;
            return true;
        }

        if (e.type == GBH_ATTR_BOOL)
        {
            if (IEquals(value, "toggle"))          { out->value = current != 0.0f ? 0.0f : 1.0f; return true; }
            if (AnyOf(value, "on",  "1", "true"))  { out->value = 1.0f; return true; }
            if (AnyOf(value, "off", "0", "false")) { out->value = 0.0f; return true; }
            *why = "expected on|off|toggle";
            return false;
        }

        float f = 0.0f;
        const bool integral = e.type == GBH_ATTR_INT;
        if (!Number(value, &f) || (integral && f != floorf(f)))
        {
            *why = integral ? "expected an integer" : "expected a number";
            return false;
        }
        const bool bounded = e.min != 0.0f || e.max != 0.0f;
        if (bounded && !(f >= e.min && f <= e.max)) { *why = "out of range"; return false; }
        out->value = f;
        return true;
    }

    // A one-letter unit reads as a suffix, "1.00x"; a word wants a space, "23.00 deg".
    const char* Gap(const char* unit) { return (unit && unit[0] && unit[1]) ? " " : ""; }

    int Format(const Entry& e, float value, char* out, int cap)
    {
        if (!out || cap <= 0) return 0;
        switch (e.type)
        {
        case GBH_ATTR_BOOL: return snprintf(out, (size_t)cap, "%s", value != 0.0f ? "ON" : "OFF");
        case GBH_ATTR_INT:  return snprintf(out, (size_t)cap, "%d%s%s", (int)value, Gap(e.unit), e.unit);
        default:            return snprintf(out, (size_t)cap, "%.2f%s%s", value, Gap(e.unit), e.unit);
        }
    }
}
