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
    // The list says ON or OFF, or ON>OFF once switched. The detail page says which of these it is.
    enum class State { Loaded, NoCode, Pending, OffIni, Refused, Failed };

    // The detail page's switch: gbhook.ini's mods.disabled, read by the next boot.
    enum class Switch { None, TurnOff, TurnOn };

    struct Mod
    {
        std::string id, version, folder, stage;
        State       state = State::Loaded;
        std::string note;      // the refusal or failure text, "" otherwise
        std::string content;   // what the content build did, "" for a code-only mod
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

    // One mod in full, ending with a Back row.
    std::vector<Rows::Row> Detail(const Mod& m);

    bool        IsOn(State s);
    const char* StateWord(State s);   // "ON" or "OFF"
}
