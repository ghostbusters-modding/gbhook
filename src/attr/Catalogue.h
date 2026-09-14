// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The attribute catalogue: which keys exist, their types and ranges, and the word grammar for setting one. Pure, no
// allocation. services/Attr.cpp is the memory and the natives behind it, tests/attr/test_catalogue.cpp is the spec.

#include "gbhook/gbhook.h"

namespace Catalogue
{
    // Stable ids, so the service switches on one instead of matching keys twice.
    enum Id
    {
        kGod, kGiant, kTorpedo, kHunt, kGravity, kTime, kFov, kCamDist, kCamMode
    };

    struct Entry
    {
        Id          id;
        const char* key;
        GbhAttrType type;
        bool        writable;
        bool        resettable;   // "reset" is a valid value: the engine has a native that restores its default
        float       min, max;     // both zero when unbounded
        const char* unit;
        const char* help;
    };

    constexpr int kMaxEntries = 32;

    int          Count();
    const Entry* At(int i);               // nullptr out of range
    const Entry* Find(const char* key);   // case-insensitive; nullptr when unknown

    struct Parsed
    {
        bool  reset;   // the caller runs the engine's reset native instead of a set
        float value;   // 1 or 0 for a bool, else the number
    };

    // "on|off|1|0|true|false|toggle" for a bool, a number in range for a float or int, "reset" where allowed.
    // `current` is what "toggle" flips. False with `why` pointing at a static string when the value is refused.
    bool Parse(const Entry& e, const char* value, float current, Parsed* out, const char** why);

    // The display form: "ON"/"OFF", "%.2f" plus the unit ("1.00x"), an int in decimal. snprintf's return: the
    // full length, so a result of cap or more means `out` was truncated.
    int Format(const Entry& e, float value, char* out, int cap);
}
