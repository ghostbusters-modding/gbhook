// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "BindTable.h"

#include <cctype>
#include <cstring>

namespace
{
    void Copy(char* dst, int cap, const char* src)
    {
        if (!src) src = "";
        strncpy(dst, src, (size_t)cap - 1);
        dst[cap - 1] = 0;
    }

    bool SameOwner(const char* a, const char* b) { return strcmp(a ? a : "", b ? b : "") == 0; }

    bool InRange(int vk) { return vk > 0 && vk < BindTable::kVkCount; }
}

namespace BindTable
{
    bool ValidName(const char* name)
    {
        if (!name || !*name) return false;
        size_t n = 0;
        for (const char* p = name; *p; ++p, ++n)
            if (isspace((unsigned char)*p) || n >= (size_t)kNameCap - 1) return false;
        return true;
    }

    Table::Table() : claims_(), count_(0), down_(), eaten_(), captured_(false), captor_() {}

    AddResult Table::Add(const char* owner, const char* name, const char* help, Chord::Key key, void* fn, void* user)
    {
        AddResult r;
        if (!ValidName(name)) { r.status = Add::BadName; return r; }
        if (Find(owner, name) >= 0) { r.status = Add::Taken; return r; }
        if (count_ >= kMaxClaims) { r.status = Add::Full; return r; }

        if (key.vk)
            for (int i = 0; i < count_; ++i)
                if (claims_[i].key == key) { r.clashWith = i; key = Chord::Key(); break; }

        Claim& c = claims_[count_];
        Copy(c.owner, kOwnerCap, owner);
        Copy(c.name, kNameCap, name);
        Copy(c.help, kHelpCap, help);
        c.key     = key;
        c.fn      = fn;
        c.user    = user;
        c.enabled = true;
        c.faulted = false;
        r.index   = count_++;
        return r;
    }

    int Table::Find(const char* owner, const char* name) const
    {
        if (!name) return -1;
        for (int i = 0; i < count_; ++i)
            if (SameOwner(claims_[i].owner, owner) && strcmp(claims_[i].name, name) == 0) return i;
        return -1;
    }

    unsigned Table::HeldMods() const
    {
        return (down_[Chord::kVkCtrl] ? Chord::kCtrl : 0u) | (down_[Chord::kVkShift] ? Chord::kShift : 0u)
             | (down_[Chord::kVkAlt] ? Chord::kAlt : 0u);
    }

    bool Table::Live(int i) const
    {
        if (i < 0 || i >= count_) return false;
        const Claim& c = claims_[i];
        return c.enabled && !c.faulted && (!captured_ || SameOwner(c.owner, captor_));
    }

    Edge Table::Down(int vk)
    {
        Edge e;
        if (!InRange(vk)) return e;
        if (down_[vk]) { e.consume = eaten_[vk]; return e; }
        down_[vk] = true;

        const unsigned held = HeldMods();
        int best = -1, bestMods = -1;
        bool shutOut = false;
        for (int i = 0; i < count_; ++i)
        {
            const Claim& c = claims_[i];
            if (c.key.vk != vk || (c.key.mods & ~held)) continue;
            if (!Live(i))
            {
                // Under a capture another mod's live chord is eaten so it cannot reach the engine either.
                if (captured_ && c.enabled && !c.faulted) shutOut = true;
                continue;
            }
            const int n = (int)(c.key.mods & 1u) + (int)((c.key.mods >> 1) & 1u) + (int)((c.key.mods >> 2) & 1u);
            if (n > bestMods) { best = i; bestMods = n; }
        }
        e.fired   = best;
        e.consume = best >= 0 || shutOut;
        eaten_[vk] = e.consume;
        return e;
    }

    bool Table::Up(int vk)
    {
        if (!InRange(vk)) return false;
        const bool eat = down_[vk] && eaten_[vk];
        down_[vk]  = false;
        eaten_[vk] = false;
        return eat;
    }

    void Table::ReleaseAll()
    {
        memset(down_, 0, sizeof down_);
        memset(eaten_, 0, sizeof eaten_);
    }

    bool Table::Held(int i) const
    {
        if (!Live(i)) return false;
        const Chord::Key& k = claims_[i].key;
        return k.vk && down_[k.vk] && (k.mods & ~HeldMods()) == 0;
    }

    bool Table::Enable(int i, bool on)
    {
        if (i < 0 || i >= count_) return false;
        claims_[i].enabled = on;
        return true;
    }

    void Table::Kill(int i)
    {
        if (i >= 0 && i < count_) claims_[i].faulted = true;
    }

    bool Table::Capture(const char* owner, bool on)
    {
        if (captured_ && !SameOwner(captor_, owner)) return false;
        if (on) { captured_ = true; Copy(captor_, kOwnerCap, owner); }
        else    { captured_ = false; captor_[0] = 0; }
        return true;
    }

    bool Table::CapturedBy(const char* owner) const { return captured_ && SameOwner(captor_, owner); }

    const Claim* Table::At(int i) const { return (i >= 0 && i < count_) ? &claims_[i] : nullptr; }
}
