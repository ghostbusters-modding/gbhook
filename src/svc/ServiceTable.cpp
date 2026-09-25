// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ServiceTable.h"

#include <cctype>
#include <cstring>

namespace
{
    bool SameName(const char* a, const char* b)
    {
        for (; *a && *b; ++a, ++b)
            if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        return *a == *b;
    }

    void CopyOwner(char* dst, size_t cap, const char* owner)
    {
        size_t n = owner ? strlen(owner) : 0;
        if (n > cap - 1) n = cap - 1;
        memcpy(dst, owner ? owner : "", n);
        dst[n] = '\0';
    }
}

const ServiceTable::Entry* ServiceTable::Lookup(const char* name) const
{
    if (!name || !*name) return nullptr;
    for (int i = 0; i < m_count; ++i)
        if (SameName(m_entries[i].name, name)) return &m_entries[i];
    return nullptr;
}

std::string ServiceTable::Qualify(const char* owner, const char* name)
{
    std::string q = (owner && *owner) ? std::string(owner) + "." + (name ? name : "") : (name ? name : "");
    for (char& c : q) c = (char)tolower((unsigned char)c);
    return q;
}

bool ServiceTable::Publish(const char* owner, const char* name, const void* table, uint32_t size, std::string* why)
{
    if (!name || !*name)                 { if (why) *why = "empty service name"; return false; }
    if (strpbrk(name, " \t\""))          { if (why) *why = "a service name cannot contain blanks or quotes"; return false; }
    const std::string q = Qualify(owner, name);
    if (q.size() >= (size_t)kNameCap)    { if (why) *why = "service name is too long"; return false; }
    if (!table)                          { if (why) *why = "null table"; return false; }
    if (size < sizeof(uint32_t))         { if (why) *why = "size is smaller than the struct_size field"; return false; }

    if (const Entry* taken = Lookup(q.c_str()))
    {
        if (why) *why = "'" + q + "' is already published by " + (taken->owner[0] ? taken->owner : "gbhook");
        return false;
    }
    if (Full()) { if (why) *why = "the service table is full"; return false; }

    Entry& e = m_entries[m_count++];
    memcpy(e.name, q.c_str(), q.size() + 1);
    CopyOwner(e.owner, sizeof e.owner, owner);
    e.table = table;
    e.size  = size;
    return true;
}

const void* ServiceTable::Find(const char* name, uint32_t* size) const
{
    const Entry* e = Lookup(name);
    if (size) *size = e ? e->size : 0;
    return e ? e->table : nullptr;
}

const char* ServiceTable::NameAt(int i) const
{
    return (i >= 0 && i < m_count) ? m_entries[i].name : nullptr;
}

const char* ServiceTable::OwnerOf(const char* name) const
{
    const Entry* e = Lookup(name);
    return e ? e->owner : nullptr;
}
