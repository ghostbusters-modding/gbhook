// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The Mods root as data: gbhook's two rows, then one row per mod page in registration order. Pure.

#include "Rows.h"

#include <string>
#include <vector>

namespace ModRows
{
    constexpr int kLoadLevel = 1;
    constexpr int kViewMods  = 2;
    constexpr int kFirstMod  = 100;   // a mod row's action is kFirstMod + its index
    constexpr int kMaxMods   = 38;    // the page's 40 rows less gbhook's two

    enum class Added { Ok, Taken, Full, BadLabel };

    struct Entry
    {
        std::string owner;   // the mod id, as the caller resolved it
        std::string label;   // fitted to the row
    };

    class Table
    {
    public:
        Added Add(const std::string& owner, const std::string& label);
        int   Find(const std::string& owner) const;   // any case; -1 when absent
        const std::vector<Entry>& Entries() const { return entries_; }

    private:
        std::vector<Entry> entries_;
    };

    // `live[i]` says whether entry i's mod is still up; a mod that failed after adding its row loses it.
    std::vector<Rows::Row> Root(const Table& t, const std::vector<bool>& live);
}
