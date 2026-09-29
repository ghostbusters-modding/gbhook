// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for menu/ModRows: the Mods root with one row per mod page.

#include "check.h"
#include "menu/ModRows.h"

using ModRows::Added;
using ModRows::Table;

int main()
{
    // gbhook's two rows alone when no mod adds a page.
    {
        const Table t;
        const auto rows = ModRows::Root(t, {});
        CHECK_EQ(rows.size(), (size_t)2);
        CHECK_EQ(rows[0].label, "Load Level");
        CHECK_EQ(rows[0].action, ModRows::kLoadLevel);
        CHECK_EQ(rows[1].label, "View Mods");
        CHECK_EQ(rows[1].action, ModRows::kViewMods);
    }

    // Registration order, one row per mod, a second add refused whatever the case.
    {
        Table t;
        CHECK(t.Add("gbskin", "Skins") == Added::Ok);
        CHECK(t.Add("superhot", "  SUPERHOT  ") == Added::Ok);
        CHECK(t.Add("GbSkin", "Skins again") == Added::Taken);
        CHECK_EQ(t.Find("GBSKIN"), 0);
        CHECK_EQ(t.Find("superhot"), 1);
        CHECK_EQ(t.Find("nobody"), -1);

        const auto rows = ModRows::Root(t, { true, true });
        CHECK_EQ(rows.size(), (size_t)4);
        CHECK_EQ(rows[2].label, "Skins");
        CHECK_EQ(rows[2].action, ModRows::kFirstMod);
        CHECK_EQ(rows[3].label, "SUPERHOT");
        CHECK_EQ(rows[3].action, ModRows::kFirstMod + 1);

        // A mod that is no longer up drops out; the others keep their actions.
        const auto down = ModRows::Root(t, { false, true });
        CHECK_EQ(down.size(), (size_t)3);
        CHECK_EQ(down[2].label, "SUPERHOT");
        CHECK_EQ(down[2].action, ModRows::kFirstMod + 1);
        CHECK_EQ(ModRows::Root(t, {}).size(), (size_t)2);
    }

    // Labels: blank or multi-line refused, long ones fitted to the row.
    {
        Table t;
        CHECK(t.Add("a", "") == Added::BadLabel);
        CHECK(t.Add("a", "   ") == Added::BadLabel);
        CHECK(t.Add("a", "one\ntwo") == Added::BadLabel);
        CHECK(t.Add("", "Label") == Added::BadLabel);
        CHECK(t.Add("a", std::string(60, 'x')) == Added::Ok);
        CHECK_EQ(t.Entries()[0].label.size(), Rows::kLabelMax);
        CHECK_EQ(t.Entries()[0].label, Rows::Fit(std::string(60, 'x')));
    }

    // The page holds 40 rows: gbhook's two and 38 mods.
    {
        Table t;
        for (int i = 0; i < ModRows::kMaxMods; ++i) CHECK(t.Add("m" + std::to_string(i), "Row") == Added::Ok);
        CHECK(t.Add("late", "Row") == Added::Full);
        CHECK_EQ(ModRows::Root(t, std::vector<bool>(ModRows::kMaxMods, true)).size(), (size_t)40);
    }

    return check::Done("modrows");
}
