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
        std::string id;                          // the command, settings and log namespace
        std::string version, description;        // the Mod Manager's own keys, quoted
        int         abi = 0;
        std::string plugin;                      // a DLL under gbhook/, or empty
        std::string scripts;                     // a folder under gbhook/, or empty
        std::vector<std::string> content;        // archives under gbhook/
        GbhStage    stage    = GBH_STAGE_BOOT;
        int         priority = 100;
        std::vector<std::string> requires_;
        std::vector<std::pair<std::string, std::string>> settings;   // every key in the file, as setting defaults
    };

    struct Result
    {
        Mod                      mod;
        bool                     gbhook = false;   // one of gbhook's keys is present; without one only version and description are read
        std::string              refusal;          // the first thing wrong, or empty
        std::vector<std::string> warnings;
    };

    Result Parse(const std::string& text);

    // Why `id` cannot be a mod id, or "" when it can: a-z, 0-9 and _, at most 63 characters.
    std::string IdProblem(const std::string& id);

    const char* StageName(GbhStage s);
    bool        StageFromName(const std::string& name, GbhStage* out);
}
