// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for bus/Bus: handles that validate, registration order, slot reuse, owner disable, fault kill, the budget.

#include "check.h"
#include "bus/Bus.h"

#include <cstring>
#include <string>

using Bus::Handle;
using Bus::Table;

namespace
{
    const char* kNames[] = { "frame", "pump", "level" };
    void F1(void*) {}
    void F2(void*) {}
    void F3(void*) {}
    Handle Raw(uint64_t v) { return reinterpret_cast<Handle>(static_cast<uintptr_t>(v)); }
}

int main()
{
    // Handles: zero is never valid, garbage is rejected, and the reserved bit marks a foreign handle.
    {
        int k = -1, s = -1;
        CHECK(!Table::Unpack(nullptr, &k, &s));
        CHECK(Table::Unpack(Table::Pack(2, 63), &k, &s) && k == 2 && s == 63);
        CHECK(!Table::Unpack(Table::Pack(8, 0), &k, &s));
        CHECK(!Table::Unpack(Table::Pack(0, 64), &k, &s));
        CHECK(!Table::Unpack(Raw(0x1000000000000ull), &k, &s));
        CHECK(!Table::Unpack(Raw(Bus::kForeignBit | 0x10001ull), &k, &s));
        CHECK(Table::IsOurs(Table::Pack(0, 0)));
        CHECK(!Table::IsOurs(Raw(Bus::kForeignBit)));
    }

    Table t(3, kNames);
    std::string why;

    // Refusals: a bad kind, a null callback.
    CHECK(t.Add(3, "gb.a", (void*)&F1, nullptr, &why) == nullptr);
    CHECK_EQ(why, "no such event kind");
    CHECK(t.Add(0, "gb.a", nullptr, nullptr, &why) == nullptr);
    CHECK_EQ(why, "null callback");

    // Registration order is dispatch order, and a stale handle stays rejected after its slot is reused.
    Handle a = t.Add(0, "gb.a", (void*)&F1, (void*)1, &why);
    Handle b = t.Add(0, "gb.b", (void*)&F2, (void*)2, &why);
    Handle c = t.Add(0, nullptr, (void*)&F3, (void*)3, &why);
    CHECK(a && b && c && a != b && b != c);
    CHECK_EQ(std::string(t.OwnerOf(c)), "gbhook");
    {
        Bus::Snap snap[4];
        CHECK_EQ(t.Snapshot(0, snap, 4), 3);
        CHECK(snap[0].fn == (void*)&F1 && snap[1].fn == (void*)&F2 && snap[2].fn == (void*)&F3);
        CHECK(snap[0].user == (void*)1 && snap[0].h == a);
        CHECK_EQ(t.Snapshot(1, snap, 4), 0);
    }
    CHECK(t.Remove(b));
    CHECK(t.Remove(b));                       // idempotent
    CHECK(!t.Remove(Table::Pack(0, 10)));     // never handed out
    CHECK(!t.Remove(Raw(Bus::kForeignBit)));
    {
        Bus::Snap snap[4];
        CHECK_EQ(t.Snapshot(0, snap, 4), 2);
        CHECK(snap[0].fn == (void*)&F1 && snap[1].fn == (void*)&F3);
    }
    // The freed slot is reused, so the newcomer dispatches where the dead one did.
    Handle d = t.Add(0, "gb.d", (void*)&F2, nullptr, &why);
    CHECK(d == b);
    {
        Bus::Snap snap[4];
        CHECK_EQ(t.Snapshot(0, snap, 4), 3);
        CHECK(snap[1].fn == (void*)&F2 && snap[1].h == d);
    }

    // A fault kills for good and is counted; a plain removal is not. Charges after death are ignored.
    t.Kill(d);
    CHECK_EQ(t.CountOf(0).live, 2);
    CHECK_EQ(t.CountOf(0).faulted, 1);
    CHECK(t.Remove(a));
    CHECK_EQ(t.CountOf(0).live, 1);
    CHECK_EQ(t.CountOf(0).faulted, 1);
    a = t.Add(0, "gb.a", (void*)&F1, (void*)1, &why);
    CHECK(a != nullptr);
    t.Charge(d, 100);
    {
        Bus::Over over[4];
        CHECK_EQ(t.OverBudget(0, 0, over, 4), 0);
    }

    // Disable by owner spans every kind and touches nobody else.
    Handle p = t.Add(1, "gb.a", (void*)&F1, nullptr, &why);
    Handle q = t.Add(1, "gb.z", (void*)&F2, nullptr, &why);
    CHECK(p && q);
    CHECK_EQ(t.DisableOwner("gb.a"), 2);
    CHECK_EQ(t.DisableOwner("gb.a"), 0);
    CHECK_EQ(t.DisableOwner(nullptr), 0);
    CHECK_EQ(t.CountOf(0).live, 1);
    CHECK_EQ(t.CountOf(1).live, 1);
    {
        Bus::Snap snap[4];
        CHECK_EQ(t.Snapshot(1, snap, 4), 1);
        CHECK(snap[0].h == q);
    }

    // Budget: the average per call decides, the report names the owner, and every counter resets.
    t.Charge(q, 900);
    t.Charge(q, 100);         // average 500
    Handle r = t.Add(1, "gb.r", (void*)&F3, nullptr, &why);
    t.Charge(r, 10);
    {
        Bus::Over over[4];
        CHECK_EQ(t.OverBudget(1, 250, over, 4), 1);
        CHECK_EQ(std::string(over[0].owner), "gb.z");
        CHECK_EQ(over[0].avgTicks, 500LL);
        CHECK_EQ(over[0].calls, 2);
        CHECK_EQ(t.OverBudget(1, 250, over, 4), 0);
        CHECK_EQ(t.OverBudget(1, 0, over, 4), 0);   // nothing charged since the reset
    }

    // A full table refuses by name.
    {
        Table full(1, kNames);
        for (int i = 0; i < Bus::kMaxSubs; ++i) CHECK(full.Add(0, "gb.x", (void*)&F1, nullptr, &why) != nullptr);
        CHECK(full.Add(0, "gb.x", (void*)&F1, nullptr, &why) == nullptr);
        CHECK_EQ(why, "frame bus full (64)");
        CHECK(full.Remove(Table::Pack(0, 5)));
        CHECK(full.Add(0, "gb.y", (void*)&F1, nullptr, &why) == Table::Pack(0, 5));
    }

    // More kinds than the table holds are clamped, not overrun.
    {
        const char* many[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i" };
        Table big(9, many);
        CHECK_EQ(big.Kinds(), Bus::kMaxKinds);
        CHECK(big.Add(8, "gb.a", (void*)&F1, nullptr, &why) == nullptr);
    }

    return check::Done("bus");
}
