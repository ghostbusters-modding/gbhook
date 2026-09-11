// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for input/Dik: names to codes, aliases, and the engine's scan-table index rule.

#include "check.h"
#include "input/Dik.h"

#include <string>

int main()
{
    // Names, any case, with the aliases a harness line would use.
    CHECK_EQ(Dik::FromName("W"), 0x11);
    CHECK_EQ(Dik::FromName("w"), 0x11);
    CHECK_EQ(Dik::FromName("ENTER"), 0x1C);
    CHECK_EQ(Dik::FromName("Return"), 0x1C);
    CHECK_EQ(Dik::FromName("esc"), 0x01);
    CHECK_EQ(Dik::FromName("SPACE"), 0x39);
    CHECK_EQ(Dik::FromName("LSHIFT"), 0x2A);
    CHECK_EQ(Dik::FromName("F12"), 0x58);
    CHECK_EQ(Dik::FromName("UP"), 0xC8);
    CHECK_EQ(Dik::FromName("PGDN"), 0xD1);
    CHECK_EQ(Dik::FromName("0"), 0x0B);
    CHECK_EQ(Dik::FromName(""), -1);
    CHECK_EQ(Dik::FromName(nullptr), -1);
    CHECK_EQ(Dik::FromName("BANANA"), -1);

    // The canonical name is the first spelling.
    CHECK_EQ(std::string(Dik::Name(0x1C)), "ENTER");
    CHECK_EQ(std::string(Dik::Name(0x01)), "ESCAPE");
    CHECK(Dik::Name(0xFE) == nullptr);

    // Every name resolves back to itself.
    for (int code = 0; code < 256; ++code)
        if (const char* n = Dik::Name(code)) CHECK_EQ(Dik::FromName(n), code);

    // The engine's index: plain keys pass through, extended keys move their flag from bit 7 to bit 8.
    CHECK_EQ(Dik::TableIndex(0x11, Dik::kMaskDefault), 0x11);
    CHECK_EQ(Dik::TableIndex(0xC8, Dik::kMaskExtended), 0x148);
    CHECK_EQ(Dik::TableIndex(0xC8, Dik::kMaskDefault), 0x48);      // the default mask drops the flag: Up lands on Numpad 8
    CHECK_EQ(Dik::TableIndex(0x9C, Dik::kMaskExtended), 0x11C);
    CHECK_EQ(Dik::TableIndex(-1, Dik::kMaskDefault), -1);
    CHECK_EQ(Dik::TableIndex(0x100, Dik::kMaskDefault), -1);
    CHECK(Dik::TableIndex(0xFF, Dik::kMaskExtended) < Dik::kTableEntries);

    return check::Done("dik");
}
