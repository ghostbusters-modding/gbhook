// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for registry/PatchRegistry: byte patches recorded, arbitrated, reverted and verified.

#include "check.h"
#include "registry/PatchRegistry.h"

#include <cstdlib>
#include <cstring>

namespace
{
    // The test process's own memory is the "game": addresses are real pointers.
    struct RealMemory : Memory
    {
        bool  Read(uintptr_t at, void* out, size_t n) override { memcpy(out, (const void*)at, n); return true; }
        bool  Write(uintptr_t at, const void* in, size_t n) override { memcpy((void*)at, in, n); return true; }
        void* Alloc(size_t n) override { return malloc(n); }
    };

    // Refuses reads or writes that touch [lo, hi).
    struct FencedMemory : RealMemory
    {
        uintptr_t lo = 0, hi = 0;
        bool denyRead = false, denyWrite = false;
        bool Hits(uintptr_t at, size_t n) const { return at < hi && at + n > lo; }
        bool Read(uintptr_t at, void* out, size_t n) override
        { return (denyRead && Hits(at, n)) ? false : RealMemory::Read(at, out, n); }
        bool Write(uintptr_t at, const void* in, size_t n) override
        { return (denyWrite && Hits(at, n)) ? false : RealMemory::Write(at, in, n); }
    };

    // Says yes to every write and changes nothing.
    struct InertMemory : RealMemory
    {
        bool Write(uintptr_t, const void*, size_t) override { return true; }
    };

    uintptr_t A(const void* p) { return (uintptr_t)p; }
    int       B(uint8_t b)     { return (int)b; }

    struct Drift { int count = 0; uintptr_t at = 0; uint8_t now0 = 0; std::string owner; };
    void OnDrift(const PatchRegistry::Patch& p, const uint8_t* now, void* ctx)
    {
        Drift* d = (Drift*)ctx;
        ++d->count; d->at = p.at; d->now0 = now[0]; d->owner = p.owner;
    }
}

int main()
{
    const uint8_t nop[2] = { 0x90, 0x90 };

    // A write lands, records the original bytes, and reads back as written.
    {
        uint8_t code[16];
        for (int i = 0; i < 16; ++i) code[i] = (uint8_t)i;
        RealMemory mem;
        PatchRegistry reg(mem);
        std::string why;
        int h = reg.Write("gb.a", A(code + 4), nop, 2, &why);
        CHECK(h != 0);
        CHECK_EQ(why, "");
        CHECK_EQ(B(code[3]), 3);
        CHECK_EQ(B(code[4]), 0x90);
        CHECK_EQ(B(code[5]), 0x90);
        CHECK_EQ(B(code[6]), 6);
        const PatchRegistry::Patch* p = reg.Get(h);
        CHECK(p != nullptr);
        CHECK_EQ(p->at, A(code + 4));
        CHECK_EQ(p->n, (size_t)2);
        CHECK_EQ(B(p->original[0]), 4);
        CHECK_EQ(B(p->original[1]), 5);
        CHECK_EQ(B(p->written[1]), 0x90);
        CHECK_EQ(std::string(p->owner), "gb.a");
        CHECK(p->live);
        CHECK_EQ(reg.Count(), 1);
    }

    // Overlap with a live patch is refused and names its owner; adjacent is fine.
    {
        uint8_t code[16] = { 0 };
        RealMemory mem;
        PatchRegistry reg(mem);
        std::string why;
        CHECK(reg.Write("gb.a", A(code + 4), nop, 2, &why) != 0);
        CHECK_EQ(reg.Write("gb.b", A(code + 5), nop, 2, &why), 0);
        CHECK(why.find("gb.a") != std::string::npos);
        CHECK_EQ(reg.Write("gb.b", A(code + 3), nop, 2, &why), 0);
        CHECK_EQ(reg.Write("gb.a", A(code + 4), nop, 2, &why), 0);   // the same owner twice is a bug too
        CHECK(reg.Write("gb.b", A(code + 6), nop, 2, &why) != 0);
        CHECK(reg.Write("gb.b", A(code + 2), nop, 2, &why) != 0);
        CHECK_EQ(reg.Count(), 3);
        CHECK(reg.Overlapping(A(code + 5), 1) != nullptr);
        CHECK_EQ(std::string(reg.Overlapping(A(code + 5), 1)->owner), "gb.a");
        CHECK(reg.Overlapping(A(code + 8), 4) == nullptr);
        CHECK(reg.Overlapping(A(code + 0), 3) != nullptr);   // touches [2,4)
    }

    // Revert restores the bytes, frees the range, and keeps the record for inspection.
    {
        uint8_t code[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        RealMemory mem;
        PatchRegistry reg(mem);
        std::string why;
        int h = reg.Write("gb.a", A(code + 2), nop, 2, &why);
        CHECK(reg.Revert(h, &why));
        CHECK_EQ(B(code[2]), 3);
        CHECK_EQ(B(code[3]), 4);
        CHECK_EQ(reg.Count(), 0);
        CHECK(reg.Get(h) != nullptr);
        CHECK(!reg.Get(h)->live);
        CHECK(reg.Overlapping(A(code + 2), 2) == nullptr);
        CHECK(reg.Write("gb.b", A(code + 2), nop, 2, &why) != 0);
        CHECK(!reg.Revert(h, &why));   // already reverted
    }

    // Arguments are checked before memory is touched.
    {
        uint8_t code[8] = { 0 };
        RealMemory mem;
        PatchRegistry reg(mem);
        std::string why;
        CHECK_EQ(reg.Write("gb.a", A(code), nop, 0, &why), 0);
        CHECK(!why.empty());
        uint8_t big[GBH_MAX_PATCH_BYTES + 1] = { 0 };
        CHECK_EQ(reg.Write("gb.a", A(code), big, sizeof big, &why), 0);
        CHECK_EQ(reg.Write("gb.a", A(code), nullptr, 2, &why), 0);
        CHECK_EQ(reg.Write("gb.a", 0, nop, 2, &why), 0);
        CHECK_EQ(reg.Count(), 0);
        CHECK(reg.Get(0) == nullptr);
        CHECK(reg.Get(99) == nullptr);
        CHECK(!reg.Revert(0, &why));
        CHECK(!reg.Revert(99, &why));
    }

    // An unreadable or unwritable site is refused and leaves no record.
    {
        uint8_t code[8] = { 0 };
        FencedMemory mem;
        mem.lo = A(code); mem.hi = A(code + 8);
        PatchRegistry reg(mem);
        std::string why;
        mem.denyRead = true;
        CHECK_EQ(reg.Write("gb.a", A(code + 2), nop, 2, &why), 0);
        CHECK(why.find("read") != std::string::npos);
        mem.denyRead = false; mem.denyWrite = true;
        CHECK_EQ(reg.Write("gb.a", A(code + 2), nop, 2, &why), 0);
        CHECK(why.find("writ") != std::string::npos);
        CHECK_EQ(reg.Count(), 0);
        CHECK_EQ(B(code[2]), 0);
    }

    // A write that reads back unchanged did not take.
    {
        uint8_t code[8] = { 0 };
        InertMemory mem;
        PatchRegistry reg(mem);
        std::string why;
        CHECK_EQ(reg.Write("gb.a", A(code + 2), nop, 2, &why), 0);
        CHECK(why.find("did not take") != std::string::npos);
        CHECK_EQ(reg.Count(), 0);
    }

    // Verify names drift; a drifted patch refuses to revert rather than clobber.
    {
        uint8_t code[8] = { 0 };
        RealMemory mem;
        PatchRegistry reg(mem);
        std::string why;
        int h = reg.Write("gb.a", A(code + 2), nop, 2, &why);
        Drift d;
        CHECK_EQ(reg.Verify(OnDrift, &d), 0);
        CHECK_EQ(d.count, 0);
        code[3] = 0xCC;
        CHECK_EQ(reg.Verify(OnDrift, &d), 1);
        CHECK_EQ(d.count, 1);
        CHECK_EQ(d.at, A(code + 2));
        CHECK_EQ(B(d.now0), 0x90);
        CHECK_EQ(d.owner, "gb.a");
        CHECK(!reg.Revert(h, &why));
        CHECK(why.find("changed") != std::string::npos);
        CHECK_EQ(B(code[3]), 0xCC);
        CHECK(reg.Get(h)->live);
        code[3] = 0x90;
        CHECK(reg.Revert(h, &why));
        CHECK_EQ(reg.Verify(OnDrift, &d), 0);
    }

    return check::Done("patch");
}
