// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for registry/VtableRegistry: one clone per table, slot ownership, recorded vptr swaps, drift.

#include "check.h"
#include "registry/VtableRegistry.h"

#include <cstdlib>
#include <cstring>

namespace
{
    struct RealMemory : Memory
    {
        bool  Read(uintptr_t at, void* out, size_t n) override { memcpy(out, (const void*)at, n); return true; }
        bool  Write(uintptr_t at, const void* in, size_t n) override { memcpy((void*)at, in, n); return true; }
        void* Alloc(size_t n) override { return malloc(n); }
    };

    struct NoAllocMemory : RealMemory
    {
        void* Alloc(size_t) override { return nullptr; }
    };

    struct UnreadableMemory : RealMemory
    {
        bool Read(uintptr_t, void*, size_t) override { return false; }
    };

    // A shipped table: the RTTI locator at [-1], then `N` slots holding distinct fake function addresses.
    struct Table
    {
        static constexpr int N = 8;
        uintptr_t words[1 + N];
        Table()
        {
            words[0] = 0xA5A5A5A5u;                     // the locator
            for (int i = 0; i < N; ++i) words[1 + i] = 0x1000u + (uintptr_t)i * 0x10u;
        }
        uintptr_t Slot0() const { return (uintptr_t)&words[1]; }
    };

    struct Object { uintptr_t vptr; };

    uintptr_t At(const void* p) { return (uintptr_t)p; }
    uintptr_t Word(uintptr_t at) { uintptr_t v; memcpy(&v, (const void*)at, sizeof v); return v; }

    struct Drift { int count = 0; std::string what; uintptr_t at = 0; std::string owner; };
    void OnDrift(const char* what, uintptr_t at, const char* owner, void* ctx)
    {
        Drift* d = (Drift*)ctx;
        ++d->count; d->what = what; d->at = at; d->owner = owner;
    }
}

int main()
{
    // Clone copies the locator and every slot, and hands back slot 0 of the copy.
    {
        Table t;
        RealMemory mem;
        VtableRegistry reg(mem);
        std::string why;
        uintptr_t c = reg.Clone("gb.a", t.Slot0(), Table::N, &why);
        CHECK(c != 0);
        CHECK_EQ(why, "");
        CHECK(c != t.Slot0());
        CHECK_EQ(Word(c - sizeof(uintptr_t)), (uintptr_t)0xA5A5A5A5u);
        for (int i = 0; i < Table::N; ++i)
            CHECK_EQ(Word(c + (uintptr_t)i * sizeof(uintptr_t)), t.words[1 + i]);
        CHECK_EQ(reg.CloneCount(), 1);

        // The same table again is the same copy, for anyone; asking for more slots than exist is refused.
        CHECK_EQ(reg.Clone("gb.b", t.Slot0(), Table::N, &why), c);
        CHECK_EQ(reg.Clone("gb.b", t.Slot0(), 4, &why), c);
        CHECK_EQ(reg.Clone("gb.b", t.Slot0(), Table::N + 1, &why), (uintptr_t)0);
        CHECK(why.find("gb.a") != std::string::npos);
        CHECK_EQ(reg.CloneCount(), 1);
    }

    // Arguments and memory failures.
    {
        Table t;
        RealMemory mem;
        VtableRegistry reg(mem);
        std::string why;
        CHECK_EQ(reg.Clone("gb.a", 0, Table::N, &why), (uintptr_t)0);
        CHECK_EQ(reg.Clone("gb.a", t.Slot0(), 0, &why), (uintptr_t)0);
        CHECK_EQ(reg.Clone("gb.a", t.Slot0(), VtableRegistry::kMaxSlots + 1, &why), (uintptr_t)0);
        CHECK_EQ(reg.CloneCount(), 0);

        NoAllocMemory noAlloc;
        VtableRegistry reg2(noAlloc);
        CHECK_EQ(reg2.Clone("gb.a", t.Slot0(), Table::N, &why), (uintptr_t)0);
        CHECK(why.find("alloc") != std::string::npos);

        UnreadableMemory unreadable;
        VtableRegistry reg3(unreadable);
        CHECK_EQ(reg3.Clone("gb.a", t.Slot0(), Table::N, &why), (uintptr_t)0);
        CHECK(why.find("read") != std::string::npos);
    }

    // SetSlot writes the copy, never the shipped table, and reports the original.
    {
        Table t;
        RealMemory mem;
        VtableRegistry reg(mem);
        std::string why;
        uintptr_t c = reg.Clone("gb.a", t.Slot0(), Table::N, &why);
        uintptr_t orig = 0;
        CHECK(reg.SetSlot("gb.a", c, 3, 0xBEEF, &orig, &why));
        CHECK_EQ(orig, (uintptr_t)0x1030);
        CHECK_EQ(Word(c + 3 * sizeof(uintptr_t)), (uintptr_t)0xBEEF);
        CHECK_EQ(t.words[1 + 3], (uintptr_t)0x1030);
        CHECK_EQ(reg.Original(c, 3), (uintptr_t)0x1030);
        CHECK_EQ(reg.Original(c, 4), (uintptr_t)0x1040);   // unpatched slots answer too
        CHECK_EQ(reg.Original(c, 99), (uintptr_t)0);
        CHECK_EQ(reg.Original(0x1234, 0), (uintptr_t)0);

        // The same owner may re-point its slot; the original stays the shipped entry.
        CHECK(reg.SetSlot("gb.a", c, 3, 0xF00D, &orig, &why));
        CHECK_EQ(orig, (uintptr_t)0x1030);
        CHECK_EQ(Word(c + 3 * sizeof(uintptr_t)), (uintptr_t)0xF00D);

        // Another owner on the same slot is refused by name; a free slot is fine.
        CHECK(!reg.SetSlot("gb.b", c, 3, 0xDEAD, &orig, &why));
        CHECK(why.find("gb.a") != std::string::npos);
        CHECK_EQ(Word(c + 3 * sizeof(uintptr_t)), (uintptr_t)0xF00D);
        CHECK(reg.SetSlot("gb.b", c, 5, 0xDEAD, nullptr, &why));

        // Range and identity checks.
        CHECK(!reg.SetSlot("gb.a", c, Table::N, 0x1, &orig, &why));
        CHECK(!reg.SetSlot("gb.a", c, -1, 0x1, &orig, &why));
        CHECK(!reg.SetSlot("gb.a", 0x1234, 0, 0x1, &orig, &why));
        CHECK(!reg.SetSlot("gb.a", c, 0, 0, &orig, &why));
    }

    // Apply swaps the vptr, records what it replaced, and refuses a foreign object.
    {
        Table t;
        Object obj  { t.Slot0() };
        Object other{ 0x7777 };
        RealMemory mem;
        VtableRegistry reg(mem);
        std::string why;
        uintptr_t c = reg.Clone("gb.a", t.Slot0(), Table::N, &why);
        CHECK(reg.Apply("gb.a", At(&obj), c, &why));
        CHECK_EQ(obj.vptr, c);
        CHECK_EQ(reg.AppliedCount(), 1);
        CHECK(reg.Apply("gb.a", At(&obj), c, &why));     // idempotent
        CHECK_EQ(reg.AppliedCount(), 1);
        CHECK(!reg.Apply("gb.a", At(&other), c, &why));
        CHECK(why.find("7777") != std::string::npos);
        CHECK_EQ(other.vptr, (uintptr_t)0x7777);
        CHECK(!reg.Apply("gb.a", At(&obj), 0x1234, &why));
        CHECK(!reg.Apply("gb.a", 0, c, &why));
    }

    // Verify names a slot overwritten in the copy and an object whose vptr went back.
    {
        Table t;
        Object obj{ t.Slot0() };
        RealMemory mem;
        VtableRegistry reg(mem);
        std::string why;
        uintptr_t c = reg.Clone("gb.a", t.Slot0(), Table::N, &why);
        CHECK(reg.SetSlot("gb.a", c, 2, 0xBEEF, nullptr, &why));
        CHECK(reg.Apply("gb.b", At(&obj), c, &why));
        Drift d;
        CHECK_EQ(reg.Verify(OnDrift, &d), 0);

        uintptr_t bad = 0xBAD;
        memcpy((void*)(c + 2 * sizeof(uintptr_t)), &bad, sizeof bad);
        CHECK_EQ(reg.Verify(OnDrift, &d), 1);
        CHECK_EQ(d.what, "slot");
        CHECK_EQ(d.at, c + 2 * sizeof(uintptr_t));
        CHECK_EQ(d.owner, "gb.a");
        bad = 0xBEEF;
        memcpy((void*)(c + 2 * sizeof(uintptr_t)), &bad, sizeof bad);

        obj.vptr = t.Slot0();
        CHECK_EQ(reg.Verify(OnDrift, &d), 1);
        CHECK_EQ(d.what, "vptr");
        CHECK_EQ(d.at, At(&obj));
        CHECK_EQ(d.owner, "gb.b");
    }

    return check::Done("vtable");
}
