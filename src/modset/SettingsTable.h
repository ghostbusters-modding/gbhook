// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The two settings layers as one lookup: gbhook.ini "<id>.<key>" over a mod's own modinfo.ini key.
// Pure. tests/modset/test_settings.cpp is the spec; docs/MOD_FORMAT.md states the rule.

#include "format/Ini.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SettingsTable
{
    // One accepted mod's modinfo.ini keys, as the mod reads them.
    struct Defaults
    {
        std::string id;
        std::vector<std::pair<std::string, std::string>> settings;
    };

    struct Table
    {
        std::unordered_map<std::string, std::string> ini;        // gbhook.ini, keys lowered, last write wins
        std::unordered_map<std::string, std::string> defaults;   // "<id>.<key>" lowered, from modinfo.ini
        std::vector<std::string>                     warnings;
    };

    // "<id>.<key>" lowered. A null or empty id names a framework key, returned as-is but lowered.
    std::string Qualify(const char* id, const char* key);

    Table Build(const Ini::Document& ini, const std::vector<Defaults>& mods);

    // The value for a mod's key, or nullptr when neither layer has it. Pointers stay valid while `t` lives.
    const char* Get(const Table& t, const char* id, const char* key);
}
