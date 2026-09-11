// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The live actor directory, fed by the VM registration detour. A generation is one level prepare; older pointers
// are dropped there, never handed out stale. Readers copy under a short lock and never touch a pointer themselves.

#include <cstdint>
#include <string>
#include <vector>

namespace Registry
{
    struct Entry
    {
        std::string cls, name;
        void*       ptr;
        uint32_t    generation;
    };

    // Game thread, once per registration. `buffer` is "CGhostbuster Egon"; a class not starting with C is skipped.
    void Register(const char* buffer, void* ptr);

    // Level prepare begins: everything from the previous level is a stale pointer now.
    void NewGeneration();

    void     Snapshot(std::vector<Entry>& out);
    bool     Find(const char* name, Entry& out);   // exact then substring, case-insensitive
    uint32_t Generation();
    int      Count();

    // "N actors in M classes", once per level, at prepare end.
    void LogSummary(const char* level);
}
