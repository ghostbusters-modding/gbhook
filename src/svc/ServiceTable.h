// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The service directory: a name, a table pointer and its size, first publisher wins. tests/svc/test_servicetable.cpp
// is the spec. Pure and unlocked; the Windows layer locks around it and writes the log lines.

#include <cstdint>
#include <string>

class ServiceTable
{
public:
    static constexpr int kMaxEntries = 64;
    static constexpr int kNameCap    = 64;   // a longer name is refused, never cut: Find is an exact match

    // `owner` is a mod id, or null for the framework. Refused with `why` for an empty or overlong name, a null
    // table, a size under 4, a full table, or a name already taken (Find answers non-null for that case only).
    bool Publish(const char* owner, const char* name, const void* table, uint32_t size, std::string* why);

    // Case-sensitive, exact. Null when unknown; `size` may be null and is zeroed on a miss.
    const void* Find(const char* name, uint32_t* size) const;

    int         Count() const { return m_count; }
    bool        Full() const  { return m_count >= kMaxEntries; }
    const char* NameAt(int i) const;                 // null out of range
    const char* OwnerOf(const char* name) const;     // null when unknown; "" for the framework

private:
    struct Entry
    {
        char        name[kNameCap];
        char        owner[64];
        const void* table;
        uint32_t    size;
    };

    Entry m_entries[kMaxEntries] = {};
    int   m_count = 0;

    const Entry* Lookup(const char* name) const;
};
