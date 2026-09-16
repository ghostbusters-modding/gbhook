// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// previews/modinfo.ini as a record: docs/MOD_FORMAT.md is the format, tests/format/test_modini.cpp is the spec.

#include "gbhook/gbhook.h"

#include <string>
#include <utility>
#include <vector>

namespace ModIni
{
    struct Mod
    {
        std::string id;                          // [gbhook] id: the command, settings and log namespace
        std::string version, description;        // the Mod Manager's own keys, top level and quoted
        int         abi = 0;
        std::string plugin;                      // a DLL under gbhook/, or empty
        std::string scripts;                     // a folder under gbhook/, or empty
        std::vector<std::string> content;        // archives under gbhook/
        bool        disabled = false;   // the manual off switch; gbhook reads it, never writes it
        GbhStage    stage    = GBH_STAGE_BOOT;
        int         priority = 100;
        std::vector<std::string> requires_;
        std::vector<std::pair<std::string, std::string>> settings;   // key without the section, value
    };

    struct Result
    {
        Mod                      mod;
        bool                     gbhook = false;   // a [gbhook] section exists; without one the folder is the Mod Manager's alone
        std::string              refusal;          // the first thing wrong, or empty
        std::vector<std::string> warnings;
    };

    Result Parse(const std::string& text);

    const char* StageName(GbhStage s);
    bool        StageFromName(const std::string& name, GbhStage* out);
}
