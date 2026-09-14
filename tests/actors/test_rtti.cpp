// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for actors/Rtti: the locator chain over a fake x64 image, demangling, base tests, the per-vtable cache.

#include "check.h"
#include "actors/Rtti.h"
#include "gbhook/gbhook.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
    // A byte buffer standing in for the process: reads outside it fail, like an unmapped page.
    struct BufferMemory : Memory
    {
        std::vector<unsigned char> buf;
        explicit BufferMemory(size_t n) : buf(n, 0) {}

        uintptr_t Base() const { return (uintptr_t)buf.data(); }
        uintptr_t At(size_t off) const { return Base() + off; }
        bool Inside(uintptr_t at, size_t n) const { return at >= Base() && n <= buf.size() && at - Base() <= buf.size() - n; }

        bool  Read(uintptr_t at, void* out, size_t n) override { if (!Inside(at, n)) return false; memcpy(out, (const void*)at, n); return true; }
        bool  Write(uintptr_t at, const void* in, size_t n) override { if (!Inside(at, n)) return false; memcpy((void*)at, in, n); return true; }
        void* Alloc(size_t) override { return nullptr; }

        template <typename T> void Put(size_t off, const T& v) { memcpy(&buf[off], &v, sizeof v); }
    };

    // A fake image whose base is the buffer, so every RVA is a buffer offset. Records are laid out MSVC x64 style.
    struct Image
    {
        BufferMemory m;
        size_t       cursor = 0x100;

        explicit Image(size_t bytes = 0x10000) : m(bytes) {}

        size_t Alloc(size_t n) { const size_t off = (cursor + 15) & ~(size_t)15; cursor = off + n; return off; }

        uint32_t TypeDesc(const char* mangled)
        {
            const size_t off = Alloc(0x10 + strlen(mangled) + 1);
            memcpy(&m.buf[off + 0x10], mangled, strlen(mangled) + 1);
            return (uint32_t)off;
        }

        uint32_t BaseDesc(uint32_t rvaTd) { const size_t off = Alloc(0x1C); m.Put(off, rvaTd); return (uint32_t)off; }

        uint32_t BaseArray(const std::vector<uint32_t>& bcds)
        {
            const size_t off = Alloc(4 * bcds.size());
            for (size_t i = 0; i < bcds.size(); ++i) m.Put(off + 4 * i, bcds[i]);
            return (uint32_t)off;
        }

        uint32_t Hierarchy(uint32_t count, uint32_t rvaArray)
        {
            const size_t off = Alloc(0x10);
            m.Put(off + 0x08, count);
            m.Put(off + 0x0C, rvaArray);
            return (uint32_t)off;
        }

        uint32_t Locator(uint32_t sig, uint32_t rvaTd, uint32_t rvaChd)
        {
            const size_t off = Alloc(0x18);
            m.Put(off + 0x00, sig);
            m.Put(off + 0x0C, rvaTd);
            m.Put(off + 0x10, rvaChd);
            m.Put(off + 0x14, (uint32_t)off);
            return (uint32_t)off;
        }

        // Slot 0 of a table whose [-1] is `colAddr`; the slots hold fake function addresses.
        uintptr_t Table(uintptr_t colAddr)
        {
            const size_t off = Alloc(8 * 4);
            m.Put(off, colAddr);
            for (size_t i = 1; i < 4; ++i) m.Put(off + 8 * i, (uintptr_t)(0x1000 * i));
            return m.At(off + 8);
        }

        // The whole chain for one class, the leaf's descriptor first as MSVC emits it.
        uintptr_t Vtable(const std::vector<const char*>& mangled)
        {
            std::vector<uint32_t> tds, bcds;
            for (const char* s : mangled) { tds.push_back(TypeDesc(s)); bcds.push_back(BaseDesc(tds.back())); }
            const uint32_t chd = Hierarchy((uint32_t)bcds.size(), BaseArray(bcds));
            return Table(m.At(Locator(1, tds[0], chd)));
        }
    };

    std::string Dm(const char* s)
    {
        char out[48];
        Rtti::Demangle(s, out, sizeof out);
        return out;
    }
}

int main()
{
    // Demangling: class and struct prefixes, the first '@' ends the name, anything else is not a name.
    {
        CHECK_EQ(Dm(".?AVCScuttler@@"), "CScuttler");
        CHECK_EQ(Dm(".?AUSFoo@@"), "SFoo");
        CHECK_EQ(Dm(".?AVCFoo@NS@@"), "CFoo");
        CHECK_EQ(Dm(".?AVIDeformableModelPoser@@"), "IDeformableModelPoser");
        CHECK_EQ(Dm("CScuttler"), "");
        CHECK_EQ(Dm(".?A"), "");
        CHECK_EQ(Dm(""), "");
        CHECK_EQ(Dm(nullptr), "");
        char tiny[4];
        Rtti::Demangle(".?AVCScuttler@@", tiny, sizeof tiny);
        CHECK_EQ(std::string(tiny), "CSc");
        Rtti::Demangle(".?AVCScuttler@@", tiny, 0);
        CHECK_EQ(std::string(tiny), "CSc");
    }

    // The chain the reference verified in-game: CScuttler down to IDeformableModelPoser.
    {
        Image img;
        const uintptr_t vt = img.Vtable({ ".?AVCScuttler@@", ".?AVCCharacter@@", ".?AVCActor@@", ".?AVCActorBase@@",
                                          ".?AVCEntity@@", ".?AVIDeformableModelPoser@@" });
        Rtti::Chain c;
        CHECK(Rtti::Decode(img.m, vt, c));
        CHECK_EQ(std::string(c.leaf), "CScuttler");
        CHECK_EQ(c.count, 6);
        CHECK_EQ(std::string(c.bases[0]), "CScuttler");
        CHECK_EQ(std::string(c.bases[1]), "CCharacter");
        CHECK_EQ(std::string(c.bases[3]), "CActorBase");
        CHECK_EQ(std::string(c.bases[5]), "IDeformableModelPoser");
        CHECK(c.Has("CScuttler"));
        CHECK(c.Has("CCharacter"));
        CHECK(c.Has("cactorbase"));
        CHECK(c.Has("IDEFORMABLEMODELPOSER"));
        CHECK(!c.Has("CGhost"));
        CHECK(!c.Has("CActo"));
        CHECK(!c.Has(""));
        CHECK(!c.Has(nullptr));
        CHECK_EQ(c.flags, (uint32_t)GBH_ACTOR_CHARACTER);
    }

    // Flags for each base the ABI names, and none for the ENABLED bit.
    {
        Image img;
        Rtti::Chain c;
        CHECK(Rtti::Decode(img.m, img.Vtable({ ".?AVCGhostbuster@@", ".?AVCCharacter@@", ".?AVCActor@@" }), c));
        CHECK_EQ(c.flags, (uint32_t)(GBH_ACTOR_CHARACTER | GBH_ACTOR_GHOSTBUSTER));
        CHECK(!c.Has("CGhost"));          // whole names only: CGhostbuster is not CGhost
        CHECK(Rtti::Decode(img.m, img.Vtable({ ".?AVCSlimer@@", ".?AVCGhost@@", ".?AVCCharacter@@", ".?AVCActor@@" }), c));
        CHECK_EQ(c.flags, (uint32_t)(GBH_ACTOR_CHARACTER | GBH_ACTOR_GHOST));
        CHECK(Rtti::Decode(img.m, img.Vtable({ ".?AVCCivilian@@", ".?AVCHuman@@", ".?AVCCharacter@@" }), c));
        CHECK_EQ(c.flags, (uint32_t)(GBH_ACTOR_CHARACTER | GBH_ACTOR_NPC));
        CHECK(Rtti::Decode(img.m, img.Vtable({ ".?AVCNPC@@", ".?AVCCharacter@@" }), c));
        CHECK_EQ(c.flags, (uint32_t)(GBH_ACTOR_CHARACTER | GBH_ACTOR_NPC));
        CHECK(Rtti::Decode(img.m, img.Vtable({ ".?AVCBreaker@@", ".?AVCPhysicsObjectBase@@", ".?AVCActor@@" }), c));
        CHECK_EQ(c.flags, (uint32_t)(GBH_ACTOR_BREAKER | GBH_ACTOR_PHYSOBJ));
        CHECK(Rtti::Decode(img.m, img.Vtable({ ".?AVCAniModel@@", ".?AVCActor@@" }), c));
        CHECK_EQ(c.flags, (uint32_t)GBH_ACTOR_ANIMODEL);
        CHECK(Rtti::Decode(img.m, img.Vtable({ ".?AVCTrigger@@", ".?AVCActor@@" }), c));
        CHECK_EQ(c.flags, 0u);
        CHECK_EQ(c.flags & GBH_ACTOR_ENABLED, 0u);
    }

    // Failures: a null or unreadable table, a null locator, a non-x64 signature, a name that is not mangled.
    {
        Image img;
        Rtti::Chain c;
        CHECK(!Rtti::Decode(img.m, 0, c));
        CHECK_EQ(std::string(c.leaf), "?");
        CHECK(!Rtti::Decode(img.m, (uintptr_t)0xDEAD0000, c));
        CHECK_EQ(std::string(c.leaf), "?");
        CHECK(!Rtti::Decode(img.m, img.Table(0), c));
        CHECK(!Rtti::Decode(img.m, img.Table((uintptr_t)0xDEAD0000), c));

        const uint32_t td = img.TypeDesc(".?AVCFoo@@");
        const uint32_t chd = img.Hierarchy(1, img.BaseArray({ img.BaseDesc(td) }));
        CHECK(!Rtti::Decode(img.m, img.Table(img.m.At(img.Locator(0, td, chd))), c));
        CHECK_EQ(std::string(c.leaf), "?");
        CHECK(Rtti::Decode(img.m, img.Table(img.m.At(img.Locator(1, td, chd))), c));
        CHECK_EQ(std::string(c.leaf), "CFoo");

        const uint32_t plain = img.TypeDesc("CFoo");
        CHECK(!Rtti::Decode(img.m, img.Table(img.m.At(img.Locator(1, plain, chd))), c));
        CHECK_EQ(std::string(c.leaf), "?");
        CHECK_EQ(c.count, 0);
        CHECK_EQ(c.flags, 0u);
    }

    // A readable leaf with an unreadable hierarchy is still an answer: the leaf alone, no bases.
    {
        Image img;
        Rtti::Chain c;
        const uint32_t td = img.TypeDesc(".?AVCGhostbuster@@");
        CHECK(Rtti::Decode(img.m, img.Table(img.m.At(img.Locator(1, td, 0xFFFFF000u))), c));
        CHECK_EQ(std::string(c.leaf), "CGhostbuster");
        CHECK_EQ(c.count, 0);
        CHECK(c.Has("CGhostbuster"));
        CHECK(!c.Has("CCharacter"));
        CHECK_EQ(c.flags, (uint32_t)GBH_ACTOR_GHOSTBUSTER);
    }

    // A garbled base count is clamped and the list stops at the first unreadable entry; a bad name is skipped.
    {
        Image img;
        Rtti::Chain c;
        std::vector<uint32_t> bcds;
        for (const char* s : { ".?AVCScuttler@@", ".?AVCCharacter@@", ".?AVCActor@@" }) bcds.push_back(img.BaseDesc(img.TypeDesc(s)));
        bcds.push_back(img.BaseDesc(0xFFFFF000u));                       // a descriptor whose name is unreadable
        bcds.push_back(img.BaseDesc(img.TypeDesc(".?AVCActorBase@@")));
        for (int i = 0; i < 4; ++i) bcds.push_back(0xFFFFFFF0u);         // entries outside the image
        const uint32_t chd = img.Hierarchy(1000, img.BaseArray(bcds));
        CHECK(Rtti::Decode(img.m, img.Table(img.m.At(img.Locator(1, img.TypeDesc(".?AVCScuttler@@"), chd))), c));
        CHECK_EQ(c.count, 4);
        CHECK_EQ(std::string(c.bases[3]), "CActorBase");
        CHECK_EQ(c.flags, (uint32_t)GBH_ACTOR_CHARACTER);

        // Exactly kMaxBases entries fit; one more is dropped.
        std::vector<uint32_t> many;
        for (int i = 0; i < Rtti::kMaxBases + 1; ++i)
        {
            char name[48];
            snprintf(name, sizeof name, ".?AVCBase%d@@", i);
            many.push_back(img.BaseDesc(img.TypeDesc(name)));
        }
        const uint32_t deep = img.Hierarchy((uint32_t)many.size(), img.BaseArray(many));
        CHECK(Rtti::Decode(img.m, img.Table(img.m.At(img.Locator(1, img.TypeDesc(".?AVCBase0@@"), deep))), c));
        CHECK_EQ(c.count, Rtti::kMaxBases);
        CHECK(c.Has("CBase31"));
        CHECK(!c.Has("CBase32"));
    }

    // The cache: one decode per table, the same pointer back, failures not kept, the cap honoured.
    {
        Image img(0x80000);
        Rtti::Cache cache(img.m);
        const uintptr_t a = img.Vtable({ ".?AVCScuttler@@", ".?AVCCharacter@@" });
        const uintptr_t b = img.Vtable({ ".?AVCBreaker@@", ".?AVCPhysicsObjectBase@@" });

        const Rtti::Chain* ca = cache.Lookup(a);
        CHECK(ca != nullptr);
        CHECK_EQ(std::string(ca->leaf), "CScuttler");
        CHECK_EQ(cache.Size(), 1);
        CHECK(cache.Lookup(a) == ca);
        CHECK_EQ(cache.Size(), 1);

        const Rtti::Chain* cb = cache.Lookup(b);
        CHECK(cb != nullptr);
        CHECK(cb != ca);
        CHECK_EQ(std::string(cb->leaf), "CBreaker");
        CHECK_EQ(cb->flags, (uint32_t)(GBH_ACTOR_BREAKER | GBH_ACTOR_PHYSOBJ));
        CHECK_EQ(cache.Size(), 2);
        CHECK_EQ(std::string(cache.Lookup(a)->leaf), "CScuttler");

        CHECK(cache.Lookup(0) == nullptr);
        CHECK(cache.Lookup((uintptr_t)0xDEAD0000) == nullptr);
        CHECK(cache.Lookup(img.Table(0)) == nullptr);
        CHECK_EQ(cache.Size(), 2);

        // Past the cap a lookup still answers, out of a slot that is reused by the next miss.
        for (int i = 0; i < Rtti::Cache::kMaxEntries - 2; ++i)
        {
            char name[48];
            snprintf(name, sizeof name, ".?AVCFill%d@@", i);
            CHECK(cache.Lookup(img.Vtable({ name })) != nullptr);
        }
        CHECK_EQ(cache.Size(), Rtti::Cache::kMaxEntries);
        const uintptr_t x = img.Vtable({ ".?AVCOverflowX@@" });
        const uintptr_t y = img.Vtable({ ".?AVCOverflowY@@" });
        const Rtti::Chain* cx = cache.Lookup(x);
        CHECK(cx != nullptr);
        CHECK_EQ(std::string(cx->leaf), "COverflowX");
        CHECK_EQ(cache.Size(), Rtti::Cache::kMaxEntries);
        const Rtti::Chain* cy = cache.Lookup(y);
        CHECK(cy == cx);
        CHECK_EQ(std::string(cy->leaf), "COverflowY");
        CHECK_EQ(cache.Size(), Rtti::Cache::kMaxEntries);
        CHECK_EQ(std::string(cache.Lookup(a)->leaf), "CScuttler");
    }

    return check::Done("rtti");
}
