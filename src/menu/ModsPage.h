// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The Mods page as data: one row per mod with its state, and a detail page with the reason a player never
// has to read a log for. Pure; tests/menu/test_modspage.cpp is the spec.

#include "Rows.h"

#include <string>
#include <vector>

namespace ModsPage
{
    // Both pages say ON or OFF; a refusal or failure adds its reason on the detail page.
    enum class State { Loaded, NoCode, Pending, OffIni, Refused, Failed };

    // The detail page's switch: gbhook.ini's mods.disabled, read by the next boot.
    enum class Switch { None, TurnOff, TurnOn };

    // How the cache POD came to be, and what became of it.
    enum class Origin { None, Cached, Built };
    enum class Mount  { None, NotYet, Mounted, MountFailed, BuildFailed, InChain };

    struct Mod
    {
        std::string id, version, folder;
        State       state = State::Loaded;
        std::string note;      // the refusal or failure text, "" otherwise
        int         assetFiles = 0;
        int         codeFiles  = 0;   // 1 when modinfo.ini names a plugin
        Origin      origin     = Origin::None;
        Mount       mount      = Mount::None;
        Switch      toggle  = Switch::None;
        bool        changed = false;   // gbhook.ini no longer matches this boot
        bool        saveFailed = false;
    };

    struct Header
    {
        std::string version;   // gbhook's
        std::string targetMd5; // the ghost.exe build every offset was verified against
        std::vector<std::string> roots;          // configured
        std::vector<std::string> missingRoots;   // configured and absent
    };

    constexpr int kBack   = 1000;
    constexpr int kToggle = 1001;

    // The list: a header row, one row per mod whose action is its index, and one row per missing root.
    std::vector<Rows::Row> List(const Header& h, const std::vector<Mod>& mods);

    // One mod in full, ending with a Back row. Once the disk differs from this boot the switch is an inert
    // "Restart to Apply Changes", so the row count never moves and a live page only relabels.
    std::vector<Rows::Row> Detail(const Mod& m);

    // "7 asset files, 1 code file", then a second row for how the cache came to be and what became of it.
    std::vector<std::string> ContentLines(const Mod& m);

    bool        IsOn(State s);
    const char* StateWord(State s);   // "ON" or "OFF"
}
