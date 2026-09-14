// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Rtti.h"
#include "gbhook/gbhook.h"

#include <cctype>
#include <cstring>

namespace
{
    // x64 layouts: the locator carries its own image base, the type descriptor's name follows two pointers.
    constexpr uintptr_t kColSignature = 0x00;
    constexpr uintptr_t kColTypeDesc  = 0x0C;
    constexpr uintptr_t kColClassDesc = 0x10;
    constexpr uintptr_t kColSelf      = 0x14;
    constexpr size_t    kColBytes     = 0x18;
    constexpr uintptr_t kChdBaseCount = 0x08;
    constexpr uintptr_t kChdBaseArray = 0x0C;
    constexpr uintptr_t kTdName       = 0x10;
    constexpr size_t    kMangledCap   = 96;

    bool IEquals(const char* a, const char* b)
    {
        for (;; ++a, ++b)
        {
            const int x = tolower((unsigned char)*a), y = tolower((unsigned char)*b);
            if (x != y) return false;
            if (!x) return true;
        }
    }

    uint32_t U32(const unsigned char* p, uintptr_t off)
    {
        uint32_t v;
        memcpy(&v, p + off, sizeof v);
        return v;
    }

    // Byte by byte: a name may end a page, and one read past it would fail the whole string.
    bool ReadMangled(Memory& mem, uintptr_t at, char* out, size_t cap)
    {
        size_t n = 0;
        while (n + 1 < cap)
        {
            unsigned char c = 0;
            if (!mem.Read(at + n, &c, 1)) break;
            if (c == 0) { out[n] = 0; return n > 0; }
            if (c < 0x20 || c > 0x7E) break;
            out[n++] = (char)c;
        }
        out[0] = 0;
        return false;
    }

    // The RVA chain locator -> type descriptor -> name, demangled into `out`.
    bool ReadClassName(Memory& mem, uintptr_t image, uint32_t rvaTd, char* out, size_t cap)
    {
        char mangled[kMangledCap];
        if (!ReadMangled(mem, image + rvaTd + kTdName, mangled, sizeof mangled)) return false;
        Rtti::Demangle(mangled, out, cap);
        return out[0] != 0;
    }

    uint32_t FlagsOf(const Rtti::Chain& c)
    {
        uint32_t f = 0;
        if (c.Has("CCharacter"))         f |= GBH_ACTOR_CHARACTER;
        if (c.Has("CGhostbuster"))       f |= GBH_ACTOR_GHOSTBUSTER;
        if (c.Has("CNPC") || c.Has("CHuman")) f |= GBH_ACTOR_NPC;
        if (c.Has("CGhost"))             f |= GBH_ACTOR_GHOST;
        if (c.Has("CBreaker"))           f |= GBH_ACTOR_BREAKER;
        if (c.Has("CAniModel"))          f |= GBH_ACTOR_ANIMODEL;
        if (c.Has("CPhysicsObjectBase")) f |= GBH_ACTOR_PHYSOBJ;
        return f;
    }
}

namespace Rtti
{
    bool Chain::Has(const char* cls) const
    {
        if (!cls || !*cls) return false;
        if (IEquals(leaf, cls)) return true;
        for (int i = 0; i < count; ++i) if (IEquals(bases[i], cls)) return true;
        return false;
    }

    void Demangle(const char* mangled, char* out, size_t cap)
    {
        if (cap == 0) return;
        out[0] = 0;
        if (!mangled || cap < 2) return;
        // ".?AV" is a class, ".?AU" a struct; the kind letter is skipped and the name runs to the first '@'.
        if (mangled[0] != '.' || mangled[1] != '?' || mangled[2] != 'A' || !mangled[3]) return;
        const char* s = mangled + 4;
        size_t n = 0;
        while (s[n] && s[n] != '@' && n + 1 < cap)
        {
            const unsigned char u = (unsigned char)s[n];
            if (u < 0x20 || u > 0x7E) break;
            out[n] = s[n];
            ++n;
        }
        out[n] = 0;
    }

    bool Decode(Memory& mem, uintptr_t vtable, Chain& out)
    {
        memset(&out, 0, sizeof out);
        out.leaf[0] = '?';
        if (!vtable) return false;

        uintptr_t col = 0;
        if (!mem.Read(vtable - sizeof(uintptr_t), &col, sizeof col) || !col) return false;

        unsigned char hdr[kColBytes];
        if (!mem.Read(col, hdr, sizeof hdr)) return false;
        const uint32_t sig = U32(hdr, kColSignature), rvaTd = U32(hdr, kColTypeDesc);
        const uint32_t rvaCd = U32(hdr, kColClassDesc), rvaSelf = U32(hdr, kColSelf);
        if (sig != 1 || rvaSelf == 0 || col <= rvaSelf) return false;
        const uintptr_t image = col - rvaSelf;

        if (!ReadClassName(mem, image, rvaTd, out.leaf, sizeof out.leaf)) { out.leaf[0] = '?'; out.leaf[1] = 0; return false; }

        // A garbled count or a bad entry only shortens the list; the leaf alone is still an answer.
        uint32_t nBase = 0, rvaArr = 0;
        if (mem.Read(image + rvaCd + kChdBaseCount, &nBase, sizeof nBase) &&
            mem.Read(image + rvaCd + kChdBaseArray, &rvaArr, sizeof rvaArr))
        {
            if (nBase > (uint32_t)kMaxBases) nBase = (uint32_t)kMaxBases;
            for (uint32_t k = 0; k < nBase; ++k)
            {
                uint32_t rvaBcd = 0, rvaBtd = 0;
                if (!mem.Read(image + rvaArr + 4 * k, &rvaBcd, sizeof rvaBcd)) break;
                if (!mem.Read(image + rvaBcd, &rvaBtd, sizeof rvaBtd)) break;
                if (!ReadClassName(mem, image, rvaBtd, out.bases[out.count], kNameCap)) continue;
                ++out.count;
            }
        }
        out.flags = FlagsOf(out);
        return true;
    }

    const Chain* Cache::Lookup(uintptr_t vtable)
    {
        if (!vtable) return nullptr;
        auto it = m_chains.find(vtable);
        if (it != m_chains.end()) return &it->second;

        Chain& slot = m_chains.size() < (size_t)kMaxEntries ? m_chains[vtable] : m_overflow;
        if (Decode(m_mem, vtable, slot)) return &slot;
        if (&slot != &m_overflow) m_chains.erase(vtable);
        return nullptr;
    }
}
