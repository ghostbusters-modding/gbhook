// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Objective engine state by key: read out of game memory, written through the engine's own natives. attr/ holds
// the catalogue, parsing and formatting; this is the memory and the natives. A mod's remembered toggle never lands here.

#include "gbhook/gbhook.h"

#include <cstdint>

namespace Attr
{
    int Count();
    int At(int i, GbhAttrInfo* out, uint32_t stride);

    // Display form into `out`: "ON", "-32.00", "1.00x". GBH_ERR_STATE when the value cannot be read right now.
    int Get(const char* key, char* out, int cap);
    int GetFloat(const char* key, float* out);

    // Game thread. "on|off|toggle" for a bool, a number in range for a float, "reset" where the engine has one.
    int Set(const char* key, const char* value);

    // `attr`: the catalogue with live values; `attr <key>`; `attr <key> <value>`.
    void RegisterCommands();
}
