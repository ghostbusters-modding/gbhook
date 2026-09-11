// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for menu/LevelsPage: the career table as one page, the archives' other levels as another.

#include "check.h"
#include "menu/LevelsPage.h"

int main()
{
    CHECK_EQ(LevelsPage::Stem("haunt1.lvl"), "haunt1");
    CHECK_EQ(LevelsPage::Stem("HAUNT1.LVL"), "HAUNT1");
    CHECK_EQ(LevelsPage::Stem("haunt1"), "haunt1");
    CHECK_EQ(LevelsPage::Stem(".lvl"), ".lvl");

    const std::vector<std::string> career = { "firehouse.lvl", "hotel1a.lvl", "abyss.lvl" };
    const std::vector<std::string> found  = { "Firehouse.lvl", "zzz_test.lvl", "duel_arena.lvl", "HOTEL1A.LVL", "Duel_Arena.lvl", "abyss.lvl" };

    // Career: table order, one row per entry, each action indexing its stem.
    LevelsPage::Page c = LevelsPage::Career(career);
    CHECK_EQ(c.rows.size(), (size_t)3);
    CHECK_EQ(c.rows[0].label, "firehouse");
    CHECK_EQ(c.rows[0].action, 0);
    CHECK_EQ(c.rows[2].label, "abyss");
    CHECK_EQ(c.levels[c.rows[2].action], "abyss");

    // Custom: what the archives hold beyond the table, sorted, deduplicated across case.
    LevelsPage::Page u = LevelsPage::Custom(career, found);
    CHECK_EQ(u.rows.size(), (size_t)2);
    CHECK_EQ(u.rows[0].label, "duel_arena");
    CHECK_EQ(u.rows[0].action, 0);
    CHECK_EQ(u.rows[1].label, "zzz_test");
    CHECK_EQ(u.levels.size(), (size_t)2);
    CHECK_EQ(u.levels[u.rows[1].action], "zzz_test");

    // Nothing custom: the page says where one would appear, and nothing is selectable.
    LevelsPage::Page n = LevelsPage::Custom(career, { "abyss.lvl" });
    CHECK_EQ(n.rows.size(), (size_t)1);
    CHECK_EQ(n.rows[0].label, "(none: a mod's world\\*.lvl lands here)");
    CHECK_EQ(n.rows[0].action, Rows::kInert);
    CHECK_EQ(n.levels.size(), (size_t)0);

    // An unreadable career table says so; the archives' levels are still a full custom list.
    LevelsPage::Page e = LevelsPage::Career({});
    CHECK_EQ(e.rows.size(), (size_t)1);
    CHECK_EQ(e.rows[0].action, Rows::kInert);
    CHECK_EQ(LevelsPage::Custom({}, { "b.lvl", "a.lvl" }).rows[0].label, "a");

    // Checkpoints: the start first, then the names without their prefix, actions 1-based into the input.
    std::vector<Rows::Row> cp = LevelsPage::Checkpoints({ "checkpoint_Start", "checkpoint_Lobby2a", "oddName" });
    CHECK_EQ(cp.size(), (size_t)4);
    CHECK_EQ(cp[0].label, "Level Start");
    CHECK_EQ(cp[0].action, LevelsPage::kStart);
    CHECK_EQ(cp[1].label, "Start");
    CHECK_EQ(cp[1].action, 1);
    CHECK_EQ(cp[2].label, "Lobby2a");
    CHECK_EQ(cp[3].label, "oddName");
    CHECK_EQ(cp[3].action, 3);
    CHECK_EQ(LevelsPage::Checkpoints({}).size(), (size_t)1);

    return check::Done("levelspage");
}
