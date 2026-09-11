// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Table.h"

#include <algorithm>
#include <cctype>

namespace
{
    std::string Lower(std::string s)
    {
        for (char& c : s) c = (char)tolower((unsigned char)c);
        return s;
    }
}

namespace CmdTable
{
    std::string Qualify(const char* owner, const char* name)
    {
        std::string n = Lower(name ? name : "");
        if (!owner || !*owner) return n;
        return Lower(owner) + "." + n;
    }

    bool Table::Add(const char* owner, const char* name, void* fn, void* user, const char* help, unsigned flags,
                    std::string* why)
    {
        if (!name || !*name)                 { if (why) *why = "empty command name"; return false; }
        if (!fn)                              { if (why) *why = "null handler"; return false; }
        for (const char* p = name; *p; ++p)
            if (*p == ' ' || *p == '\t' || *p == '"') { if (why) *why = "a command name cannot contain blanks or quotes"; return false; }

        Entry e;
        e.name  = Qualify(owner, name);
        e.owner = owner ? owner : "";
        e.help  = help ? help : "";
        e.fn    = fn;
        e.user  = user;
        e.flags = flags;

        if (const Entry* taken = Find(e.name))
        {
            if (why) *why = "'" + e.name + "' is already registered by " + (taken->owner.empty() ? "gbhook" : taken->owner);
            return false;
        }
        m_entries.push_back(e);
        return true;
    }

    const Entry* Table::Find(const std::string& name) const
    {
        const std::string want = Lower(name);
        for (const Entry& e : m_entries) if (e.name == want) return &e;
        return nullptr;
    }

    std::vector<Entry> Table::Sorted() const
    {
        std::vector<Entry> out = m_entries;
        std::sort(out.begin(), out.end(), [](const Entry& a, const Entry& b) { return a.name < b.name; });
        return out;
    }
}
