// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The level lists as data: the engine's career table, and every other .lvl the mounted archives hold.
// Pure; tests/menu/test_levelspage.cpp is the spec.

#include "Rows.h"

#include <string>
#include <vector>

namespace LevelsPage
{
    struct Page
    {
        std::vector<Rows::Row>   rows;
        std::vector<std::string> levels;   // a row's action indexes this: the stem to chain to
    };

    // The career table in its own order. Names may carry .lvl.
    Page Career(const std::vector<std::string>& career);

    // Everything `found` names that the career table does not, sorted, one row per name, any case.
    Page Custom(const std::vector<std::string>& career, const std::vector<std::string>& found);

    constexpr int kStart = 0;   // the checkpoint page's first row: the level from its beginning

    // The checkpoint page: Level Start, then one row per registered name, "checkpoint_Lobby" shown as "Lobby".
    // A row's action is 1 + its index in `registered`.
    std::vector<Rows::Row> Checkpoints(const std::vector<std::string>& registered);

    // "haunt1.lvl" -> "haunt1"; "haunt1" -> "haunt1".
    std::string Stem(const std::string& name);
}
