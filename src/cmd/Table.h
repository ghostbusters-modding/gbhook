// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The command registry: one qualified name per command, "<id>.<name>" for a mod, bare for the framework.
// Pure; tests/cmd/test_table.cpp is the spec. The Windows layer adds the lock, the routing and the log lines.

#include <string>
#include <vector>

namespace CmdTable
{
    struct Entry
    {
        std::string name;    // qualified, lowered
        std::string owner;   // the mod id, "" for the framework
        std::string help;
        void*       fn    = nullptr;
        void*       user  = nullptr;
        unsigned    flags = 0;
    };

    // "<owner>.<name>" lowered, or the bare lowered name for the framework.
    std::string Qualify(const char* owner, const char* name);

    class Table
    {
    public:
        // Refused with `why` for an empty name, a name with blanks, or a qualified name already taken.
        bool Add(const char* owner, const char* name, void* fn, void* user, const char* help, unsigned flags,
                 std::string* why);
        const Entry* Find(const std::string& name) const;   // any case
        std::vector<Entry> Sorted() const;
        int Count() const { return (int)m_entries.size(); }

    private:
        std::vector<Entry> m_entries;
    };
}
