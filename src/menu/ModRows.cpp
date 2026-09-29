// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModRows.h"

#include <cctype>

namespace
{
    std::string Trim(const std::string& s)
    {
        const size_t a = s.find_first_not_of(" \t");
        if (a == std::string::npos) return {};
        return s.substr(a, s.find_last_not_of(" \t") - a + 1);
    }

    bool SameId(const std::string& a, const std::string& b)
    {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
        return true;
    }
}

namespace ModRows
{
    Added Table::Add(const std::string& owner, const std::string& label)
    {
        const std::string l = Trim(label);
        if (owner.empty() || l.empty() || l.find_first_of("\r\n") != std::string::npos) return Added::BadLabel;
        if (Find(owner) >= 0) return Added::Taken;
        if ((int)entries_.size() >= kMaxMods) return Added::Full;
        entries_.push_back({ owner, Rows::Fit(l) });
        return Added::Ok;
    }

    int Table::Find(const std::string& owner) const
    {
        for (size_t i = 0; i < entries_.size(); ++i)
            if (SameId(entries_[i].owner, owner)) return (int)i;
        return -1;
    }

    std::vector<Rows::Row> Root(const Table& t, const std::vector<bool>& live)
    {
        std::vector<Rows::Row> rows = { { "Load Level", kLoadLevel }, { "View Mods", kViewMods } };
        const std::vector<Entry>& e = t.Entries();
        for (size_t i = 0; i < e.size(); ++i)
            if (i < live.size() && live[i]) rows.push_back({ e[i].label, kFirstMod + (int)i });
        return rows;
    }
}
