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

    // What discovery gathered for one folder under a root. Every folder is judged; gbhook's keys are optional.
    struct Candidate
    {
        std::string    root;
        std::string    folder;        // the mod folder name, the Mod Manager's key
        bool           hasModInfo  = false;   // previews/modinfo.ini was read
        bool           hasGbhookDir = false;  // a gbhook/ folder exists, so a DLL is meant and needs its keys
        bool           hasAssets   = false;   // at least one of the engine's asset roots exists
        ModIni::Result ini;
        bool           hasModIni  = false;   // a gbhook/mod.ini left over from before the keys moved
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
        bool                     implicit = false;   // no id in modinfo.ini: a content mod under the folder's own name
        bool                     disabled = false;   // in gbhook.ini's mods.disabled: listed, never loaded, no content
        int                      order    = -1;      // position in the code order, accepted only
        std::string              refusal;
        std::vector<std::string> warnings;
    };

    struct Result
    {
        std::vector<Record>      records;     // accepted in code order, then refused in discovery order
        std::vector<std::string> conflicts;   // set-wide warnings: hooks and archives claimed twice
    };

    // Whether a mods.disabled entry means this mod: its folder, its id, or or a folder-named mod's legacy id.
    bool Names(const std::string& entry, const Record& r);

    // `off` is gbhook.ini's mods.disabled: ids or folder names, any case. It beats a refusal too.
    Result Resolve(const std::vector<Candidate>& found, const std::vector<std::string>& off = {});
}
