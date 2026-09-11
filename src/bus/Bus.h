// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The fan-out core behind every typed event: fixed tables, registration order, handles that validate, budget counts.
// Pure and unlocked. The Windows layer locks around it, dispatches from a snapshot, and owns the guard and the clock.

#include <cstdint>
#include <string>

namespace Bus
{
    constexpr int kMaxKinds = 8;
    constexpr int kMaxSubs  = 64;    // per kind; a hard bound keeps dispatch free of allocation

    // (kind + 1) << 16 | (slot + 1). Zero is never valid. Bit 63 is reserved for a second bus, never set here.
    typedef void* Handle;
    constexpr uint64_t kForeignBit = 1ull << 63;

    struct Snap  { void* fn; void* user; Handle h; };
    struct Over  { char owner[64]; long long avgTicks; int calls; };
    struct Count { int live, faulted; };   // a subscription dropped on purpose counts as neither

    class Table
    {
    public:
        // `names` has one entry per kind and must outlive the table; it labels log lines.
        Table(int kinds, const char* const* names);

        // A dead slot is reused first, so order is deterministic for a given call sequence, not strictly by age.
        Handle Add(int kind, const char* owner, void* fn, void* user, std::string* why);
        bool   Remove(Handle h);                    // idempotent; false for a handle that was never valid
        int    DisableOwner(const char* owner);     // every kind; how many were live

        // Live subscribers of one kind, in slot order. Returns how many were written.
        int    Snapshot(int kind, Snap* out, int cap) const;

        // After a fault: dead for good, and counted as such.
        void   Kill(Handle h);
        // Time accounting per subscriber, in whatever units the caller measures.
        void   Charge(Handle h, long long ticks);
        // Every live subscriber of `kind` averaging over `budgetTicks` per call, then all counters reset.
        int    OverBudget(int kind, long long budgetTicks, Over* out, int cap);

        Count  CountOf(int kind) const;
        int    Kinds() const               { return m_kinds; }
        const char* Name(int kind) const   { return kind >= 0 && kind < m_kinds ? m_names[kind] : "?"; }
        const char* OwnerOf(Handle h) const;   // "" for an invalid handle

        static Handle Pack(int kind, int slot);
        static bool   Unpack(Handle h, int* kind, int* slot);
        static bool   IsOurs(Handle h);        // the reserved bit is clear

    private:
        struct Sub
        {
            void*     fn;
            void*     user;
            char      owner[64];
            bool      live;
            bool      faulted;
            long long ticks;
            int       calls;
        };
        struct Kind { Sub sub[kMaxSubs]; int count; };

        int                m_kinds;
        const char* const* m_names;
        Kind               m_kind[kMaxKinds];

        Sub* Find(Handle h);
    };
}
