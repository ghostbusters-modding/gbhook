// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "PatchRegistry.h"

#include <cstdio>
#include <cstring>

namespace
{
    void CopyOwner(char* dst, size_t cap, const char* owner)
    {
        const char* who = (owner && *owner) ? owner : "gbhook";
        size_t n = strlen(who);
        if (n >= cap) n = cap - 1;
        memcpy(dst, who, n);
        dst[n] = '\0';
    }

    std::string Hex(uintptr_t v)
    {
        char b[32];
        snprintf(b, sizeof b, "%llX", (unsigned long long)v);
        return b;
    }
}

PatchRegistry::PatchRegistry(Memory& mem) : m_mem(mem) {}

int PatchRegistry::Write(const char* owner, uintptr_t at, const void* bytes, size_t n, std::string* why)
{
    std::string sink;
    if (!why) why = &sink;

    if (!at || !bytes || n == 0 || n > GBH_MAX_PATCH_BYTES) { *why = "bad argument"; return 0; }
    if (const Patch* p = Overlapping(at, n))
    {
        *why = "overlaps the patch at 0x" + Hex(p->at) + " held by '" + p->owner + "'";
        return 0;
    }

    Patch p;
    memset(&p, 0, sizeof p);
    p.at = at;
    p.n  = n;
    CopyOwner(p.owner, sizeof p.owner, owner);

    if (!m_mem.Read(at, p.original, n)) { *why = "site is not readable"; return 0; }
    memcpy(p.written, bytes, n);
    if (!m_mem.Write(at, bytes, n))     { *why = "site is not writable"; return 0; }

    // Re-read after writing, the same instinct as re-reading hook bytes after install.
    uint8_t now[GBH_MAX_PATCH_BYTES];
    if (!m_mem.Read(at, now, n) || memcmp(now, bytes, n) != 0)
    {
        m_mem.Write(at, p.original, n);
        *why = "write did not take";
        return 0;
    }

    p.live = true;
    m_patches.push_back(p);
    why->clear();
    return (int)m_patches.size();
}

bool PatchRegistry::Revert(int handle, std::string* why)
{
    std::string sink;
    if (!why) why = &sink;

    if (handle <= 0 || handle > (int)m_patches.size()) { *why = "no such patch"; return false; }
    Patch& p = m_patches[(size_t)handle - 1];
    if (!p.live) { *why = "already reverted"; return false; }

    uint8_t now[GBH_MAX_PATCH_BYTES];
    if (!m_mem.Read(p.at, now, p.n)) { *why = "site is not readable"; return false; }
    if (memcmp(now, p.written, p.n) != 0)
    {
        *why = "site changed since the patch; reverting would clobber it";
        return false;
    }
    if (!m_mem.Write(p.at, p.original, p.n)) { *why = "site is not writable"; return false; }

    p.live = false;
    why->clear();
    return true;
}

const PatchRegistry::Patch* PatchRegistry::Get(int handle) const
{
    if (handle <= 0 || handle > (int)m_patches.size()) return nullptr;
    return &m_patches[(size_t)handle - 1];
}

const PatchRegistry::Patch* PatchRegistry::Overlapping(uintptr_t at, size_t n) const
{
    for (const Patch& p : m_patches)
        if (p.live && at < p.at + p.n && p.at < at + n) return &p;
    return nullptr;
}

int PatchRegistry::Verify(void (*onDrift)(const Patch& p, const uint8_t* now, void* ctx), void* ctx)
{
    int drift = 0;
    for (const Patch& p : m_patches)
    {
        if (!p.live) continue;
        uint8_t now[GBH_MAX_PATCH_BYTES];
        memset(now, 0, sizeof now);
        if (!m_mem.Read(p.at, now, p.n) || memcmp(now, p.written, p.n) != 0)
        {
            ++drift;
            if (onDrift) onDrift(p, now, ctx);
        }
    }
    return drift;
}

int PatchRegistry::Count() const
{
    int n = 0;
    for (const Patch& p : m_patches) if (p.live) ++n;
    return n;
}
