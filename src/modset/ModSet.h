// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Judges the set of mod folders discovery found: one record each, cross-checks, the code order.
// No mod code runs here, so every conflict is described while every party is inert. tests/modset is the spec.

#include "format/ModIni.h"
#include "gbhook/gbhook.h"

#include <string>
#include <vector>

namespace ModSet
{
    enum class Binary { None, Missing, Unreadable, Ok };

    // What discovery gathered for one folder carrying gbhook/, before any judgement.
    struct Candidate
    {
        std::string    root;
        std::string    folder;        // the mod folder name, the Mod Manager's key
        bool           hasModIni  = false;
        ModIni::Result ini;
        bool           hasModInfo = false;   // previews/modinfo.ini exists
        Binary         binary     = Binary::None;
        std::string    binaryWhy;            // Unreadable: the reason the export walk gave
        GbhManifest    manifest{};           // Ok: the bytes read out of the DLL
    };

    struct Record
    {
        std::string              root, folder;
        ModIni::Mod              mod;
        std::vector<std::string> exclusiveHooks;   // from the manifest
        bool                     accepted = false;
        bool                     disabled = false;   // mod.ini disabled = 1: listed, never loaded, no content
        int                      order    = -1;      // position in the code order, accepted only
        std::string              refusal;
        std::vector<std::string> warnings;
    };

    struct Result
    {
        std::vector<Record>      records;     // accepted in code order, then refused in discovery order
        std::vector<std::string> conflicts;   // set-wide warnings: hooks and archives claimed twice
    };

    Result Resolve(const std::vector<Candidate>& found);
}
