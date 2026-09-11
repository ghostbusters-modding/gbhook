// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Bus.h"

#include <cstring>

namespace
{
    void CopyOwner(char* dst, const char* owner)
    {
        const char* src = (owner && *owner) ? owner : "gbhook";
        size_t n = strlen(src);
        if (n > 63) n = 63;
        memcpy(dst, src, n);
        dst[n] = '\0';
    }
}

namespace Bus
{
    Table::Table(int kinds, const char* const* names)
        : m_kinds(kinds < 0 ? 0 : (kinds > kMaxKinds ? kMaxKinds : kinds)), m_names(names)
    {
        memset(m_kind, 0, sizeof m_kind);
    }

    Handle Table::Pack(int kind, int slot)
    {
        return reinterpret_cast<Handle>((static_cast<uintptr_t>(kind + 1) << 16) | static_cast<uintptr_t>(slot + 1));
    }

    bool Table::Unpack(Handle h, int* kind, int* slot)
    {
        const uint64_t v = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(h));
        if (v & kForeignBit) return false;
        const int k = static_cast<int>((v >> 16) & 0xFFFF) - 1;
        const int s = static_cast<int>(v & 0xFFFF) - 1;
        if (k < 0 || k >= kMaxKinds || s < 0 || s >= kMaxSubs) return false;
        if (v >> 32) return false;
        *kind = k;
        *slot = s;
        return true;
    }

    bool Table::IsOurs(Handle h)
    {
        return (static_cast<uint64_t>(reinterpret_cast<uintptr_t>(h)) & kForeignBit) == 0;
    }

    Table::Sub* Table::Find(Handle h)
    {
        int k, s;
        if (!Unpack(h, &k, &s) || k >= m_kinds || s >= m_kind[k].count) return nullptr;
        return &m_kind[k].sub[s];
    }

    Handle Table::Add(int kind, const char* owner, void* fn, void* user, std::string* why)
    {
        if (kind < 0 || kind >= m_kinds) { if (why) *why = "no such event kind"; return nullptr; }
        if (!fn)                          { if (why) *why = "null callback"; return nullptr; }

        Kind& t = m_kind[kind];
        int slot = -1;
        for (int i = 0; i < t.count; ++i)
            if (!t.sub[i].live) { slot = i; break; }
        if (slot < 0 && t.count < kMaxSubs) slot = t.count++;
        if (slot < 0)
        {
            if (why) *why = std::string(Name(kind)) + " bus full (" + std::to_string(kMaxSubs) + ")";
            return nullptr;
        }

        Sub& s = t.sub[slot];
        memset(&s, 0, sizeof s);
        s.fn   = fn;
        s.user = user;
        s.live = true;
        CopyOwner(s.owner, owner);
        return Pack(kind, slot);
    }

    bool Table::Remove(Handle h)
    {
        Sub* s = Find(h);
        if (!s) return false;
        s->live = false;
        return true;
    }

    int Table::DisableOwner(const char* owner)
    {
        if (!owner || !*owner) return 0;
        int n = 0;
        for (int k = 0; k < m_kinds; ++k)
            for (int i = 0; i < m_kind[k].count; ++i)
            {
                Sub& s = m_kind[k].sub[i];
                if (s.live && strcmp(s.owner, owner) == 0) { s.live = false; ++n; }
            }
        return n;
    }

    int Table::Snapshot(int kind, Snap* out, int cap) const
    {
        if (kind < 0 || kind >= m_kinds || !out || cap <= 0) return 0;
        const Kind& t = m_kind[kind];
        int n = 0;
        for (int i = 0; i < t.count && n < cap; ++i)
            if (t.sub[i].live)
            {
                out[n].fn   = t.sub[i].fn;
                out[n].user = t.sub[i].user;
                out[n].h    = Pack(kind, i);
                ++n;
            }
        return n;
    }

    void Table::Kill(Handle h)
    {
        Sub* s = Find(h);
        if (!s || !s->live) return;
        s->live    = false;
        s->faulted = true;
    }

    void Table::Charge(Handle h, long long ticks)
    {
        Sub* s = Find(h);
        if (!s || !s->live) return;
        s->ticks += ticks;
        s->calls += 1;
    }

    int Table::OverBudget(int kind, long long budgetTicks, Over* out, int cap)
    {
        if (kind < 0 || kind >= m_kinds) return 0;
        Kind& t = m_kind[kind];
        int n = 0;
        for (int i = 0; i < t.count; ++i)
        {
            Sub& s = t.sub[i];
            if (s.live && s.calls > 0)
            {
                const long long avg = s.ticks / s.calls;
                if (avg > budgetTicks && out && n < cap)
                {
                    memcpy(out[n].owner, s.owner, sizeof out[n].owner);
                    out[n].avgTicks = avg;
                    out[n].calls    = s.calls;
                    ++n;
                }
            }
            s.ticks = 0;
            s.calls = 0;
        }
        return n;
    }

    Count Table::CountOf(int kind) const
    {
        Count c = { 0, 0 };
        if (kind < 0 || kind >= m_kinds) return c;
        const Kind& t = m_kind[kind];
        for (int i = 0; i < t.count; ++i)
        {
            if (t.sub[i].live)         ++c.live;
            else if (t.sub[i].faulted) ++c.faulted;
        }
        return c;
    }

    const char* Table::OwnerOf(Handle h) const
    {
        int k, s;
        if (!Unpack(h, &k, &s) || k >= m_kinds || s >= m_kind[k].count) return "";
        return m_kind[k].sub[s].owner;
    }
}
