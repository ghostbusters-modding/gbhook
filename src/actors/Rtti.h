// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// MSVC RTTI read out of the image over Memory: vtable[-1] is the locator and its hierarchy names every base.
// Pure memory, so it classifies a disabled actor too. tests/actors/test_rtti.cpp is the spec.

#include "registry/Memory.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace Rtti
{
    constexpr int kMaxBases = 32;
    constexpr int kNameCap  = 48;

    struct Chain
    {
        char     leaf[kNameCap];               // "?" when nothing usable could be read
        int      count;                        // names in `bases`, the class itself first, in the hierarchy's order
        char     bases[kMaxBases][kNameCap];
        uint32_t flags;                        // GbhActorFlags for the bases the ABI names; never GBH_ACTOR_ENABLED

        bool Has(const char* cls) const;       // case-insensitive, the leaf included
    };

    // ".?AVCScuttler@@" to "CScuttler", ".?AUSFoo@@" to "SFoo". Empty for anything that is not an RTTI name.
    void Demangle(const char* mangled, char* out, size_t cap);

    // False when the locator, its descriptor or the leaf name could not be read. A base list cut short by a bad
    // read is still a success with fewer names.
    bool Decode(Memory& mem, uintptr_t vtable, Chain& out);

    // Decode once per vtable. Tables are static for the process, so nothing is evicted; only successes are kept.
    class Cache
    {
    public:
        static constexpr int kMaxEntries = 1024;

        explicit Cache(Memory& mem) : m_mem(mem) {}

        // Null when unreadable. Valid for the life of the cache, or until the next call once it is full.
        const Chain* Lookup(uintptr_t vtable);
        int          Size() const { return (int)m_chains.size(); }

    private:
        Memory&                               m_mem;
        std::unordered_map<uintptr_t, Chain>  m_chains;
        Chain                                 m_overflow = {};
    };
}
