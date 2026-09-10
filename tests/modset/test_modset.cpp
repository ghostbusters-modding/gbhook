// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for modset/ModSet: the judgement over a set of discovered folders, and the resolved code order.

#include "check.h"
#include "modset/ModSet.h"

#include <cstring>

using ModSet::Binary;
using ModSet::Candidate;
using ModSet::Record;
using ModSet::Resolve;

namespace
{
    GbhManifest Mf(const char* id, uint32_t abi = GBHOOK_ABI_VERSION)
    {
        GbhManifest m;
        memset(&m, 0, sizeof m);
        memcpy(m.magic, GBH_MANIFEST_MAGIC, 9);
        m.struct_size = sizeof m;
        m.abi_version = abi;
        strcpy(m.id, id);
        strcpy(m.target_md5, GBHOOK_TARGET_MD5);
        return m;
    }

    std::string Ini(const char* id, const char* extra = "")
    {
        return std::string("[mod]\nformat = 1\nid = ") + id + "\n[gbhook]\nabi = 1\n" + extra;
    }

    Candidate Cand(const char* folder, const std::string& ini, Binary binary = Binary::None)
    {
        Candidate c;
        c.root      = "mods";
        c.folder    = folder;
        c.hasModIni = true;
        c.ini       = ModIni::Parse(ini);
        c.binary    = binary;
        return c;
    }

    const Record* Find(const ModSet::Result& r, const char* folder)
    {
        for (const Record& x : r.records) if (x.folder == folder) return &x;
        return nullptr;
    }
}

int main()
{
    // One script-only mod: accepted, first in the order.
    {
        ModSet::Result r = Resolve({ Cand("A", Ini("gb.a")) });
        CHECK_EQ(r.records.size(), (size_t)1);
        CHECK(r.records[0].accepted);
        CHECK_EQ(r.records[0].order, 0);
        CHECK_EQ(r.records[0].mod.id, "gb.a");
        CHECK_EQ(r.records[0].refusal, "");
        CHECK_EQ(r.conflicts.size(), (size_t)0);
    }

    // The silent half: gbhook/ without a mod.ini.
    {
        Candidate c; c.root = "mods"; c.folder = "Half";
        ModSet::Result r = Resolve({ c });
        CHECK(!r.records[0].accepted);
        CHECK_EQ(r.records[0].order, -1);
        CHECK_EQ(r.records[0].refusal, "gbhook/ has no mod.ini");
    }

    // A mod.ini refusal and its warnings carry through.
    {
        ModSet::Result r = Resolve({ Cand("B", "[mod]\nformat = 1\ncolour = red\n[gbhook]\nabi = 1\n") });
        CHECK_EQ(r.records[0].refusal, "mod.ini has no id under [mod]");
        CHECK_EQ(r.records[0].warnings.size(), (size_t)1);
    }

    // The plugin named by mod.ini must exist, be readable, and pass the manifest checks.
    {
        ModSet::Result r = Resolve({ Cand("C", Ini("gb.c", "plugin = C.dll\n"), Binary::Missing) });
        CHECK_EQ(r.records[0].refusal, "plugin 'C.dll' is not in gbhook/");
    }
    {
        Candidate c = Cand("C", Ini("gb.c", "plugin = C.dll\n"), Binary::Unreadable);
        c.binaryWhy = "not a PE file (bad DOS header)";
        CHECK_EQ(Resolve({ c }).records[0].refusal, "plugin 'C.dll': not a PE file (bad DOS header)");
    }
    {
        Candidate c = Cand("C", Ini("gb.c", "plugin = C.dll\n"), Binary::Ok);
        c.manifest = Mf("gb.c"); c.manifest.magic[0] = 'X';
        CHECK_EQ(Resolve({ c }).records[0].refusal, "plugin 'C.dll': manifest magic mismatch (stale SDK?)");
    }

    // The two facts stated twice must agree.
    {
        Candidate c = Cand("C", Ini("gb.c", "plugin = C.dll\n"), Binary::Ok);
        c.manifest = Mf("gb.other");
        CHECK_EQ(Resolve({ c }).records[0].refusal,
                 "mod.ini says id 'gb.c' but C.dll says 'gb.other' -- one was edited after the build");
    }
    {
        Candidate c = Cand("C", Ini("gb.c", "plugin = C.dll\n"), Binary::Ok);
        c.manifest = Mf("gb.c", 2);
        CHECK_EQ(Resolve({ c }).records[0].refusal, "plugin 'C.dll': built for ABI 2, this gbhook speaks ABI 1 -- rebuild the mod");
    }

    // A good binary: accepted, exclusive hooks carried over.
    {
        Candidate c = Cand("C", Ini("gb.c", "plugin = C.dll\n"), Binary::Ok);
        c.manifest = Mf("gb.c");
        strcpy(c.manifest.exclusive_hooks[0], "ghost+0x46A110");
        ModSet::Result r = Resolve({ c });
        CHECK(r.records[0].accepted);
        CHECK_EQ(r.records[0].exclusiveHooks.size(), (size_t)1);
        CHECK_EQ(r.records[0].exclusiveHooks[0], "ghost+0x46A110");
    }

    // Duplicate ids: the first folder keeps it.
    {
        ModSet::Result r = Resolve({ Cand("A", Ini("gb.a")), Cand("B", Ini("gb.a")) });
        CHECK(Find(r, "A")->accepted);
        CHECK(!Find(r, "B")->accepted);
        CHECK_EQ(Find(r, "B")->refusal, "duplicate id 'gb.a', already claimed by folder 'A'");
    }

    // requires: absent, refused, and a cascade.
    {
        ModSet::Result r = Resolve({ Cand("A", Ini("gb.a", "requires = gb.z\n")) });
        CHECK_EQ(r.records[0].refusal, "requires 'gb.z', which is not present");
    }
    {
        ModSet::Result r = Resolve({ Cand("A", Ini("gb.a", "requires = gb.b\n")),
                                     Cand("B", "[mod]\nformat = 1\nid = gb.b\n") });
        CHECK_EQ(Find(r, "A")->refusal, "requires 'gb.b', which was refused");
    }
    {
        ModSet::Result r = Resolve({ Cand("A", Ini("gb.a", "requires = gb.b\n")),
                                     Cand("B", Ini("gb.b", "requires = gb.c\n")),
                                     Cand("C", Ini("gb.c", "requires = gb.z\n")) });
        CHECK_EQ(Find(r, "C")->refusal, "requires 'gb.z', which is not present");
        CHECK_EQ(Find(r, "B")->refusal, "requires 'gb.c', which was refused");
        CHECK_EQ(Find(r, "A")->refusal, "requires 'gb.b', which was refused");
    }

    // requires does not order; a dependency that initialises later is called out.
    {
        ModSet::Result r = Resolve({ Cand("A", Ini("gb.a", "requires = gb.b\n")),
                                     Cand("B", Ini("gb.b", "priority = 200\n")) });
        CHECK(Find(r, "A")->accepted);
        CHECK_EQ(Find(r, "A")->warnings.size(), (size_t)1);
        CHECK_EQ(Find(r, "A")->warnings[0],
                 "requires 'gb.b', which initialises after it -- give this mod a higher priority or a later stage");
        ModSet::Result ok = Resolve({ Cand("A", Ini("gb.a", "requires = gb.b\npriority = 300\n")),
                                      Cand("B", Ini("gb.b", "priority = 200\n")) });
        CHECK_EQ(Find(ok, "A")->warnings.size(), (size_t)0);
    }

    // Set-wide conflicts: both stay accepted, the conflict is named.
    {
        Candidate a = Cand("A", Ini("gb.a", "plugin = A.dll\n"), Binary::Ok); a.manifest = Mf("gb.a");
        Candidate b = Cand("B", Ini("gb.b", "plugin = B.dll\n"), Binary::Ok); b.manifest = Mf("gb.b");
        strcpy(a.manifest.exclusive_hooks[0], "ghost+0x1F1210");
        strcpy(b.manifest.exclusive_hooks[0], "ghost+0x1F1210");
        ModSet::Result r = Resolve({ a, b });
        CHECK(Find(r, "A")->accepted);
        CHECK(Find(r, "B")->accepted);
        CHECK_EQ(r.conflicts.size(), (size_t)1);
        CHECK_EQ(r.conflicts[0], "hook ghost+0x1F1210 claimed by both 'gb.a' and 'gb.b' -- whichever loads first wins and the other is refused at install time");
    }
    {
        ModSet::Result r = Resolve({ Cand("A", Ini("gb.a", "content = content/HAUNT.POD\n")),
                                     Cand("B", Ini("gb.b", "content = pods/haunt.pod, pods/OTHER.POD\n")) });
        CHECK_EQ(r.conflicts.size(), (size_t)1);
        CHECK_EQ(r.conflicts[0], "content HAUNT.POD shipped by both 'gb.a' and 'gb.b' -- mount order decides, silently");
    }

    // The code order: stage, then priority, then id; refused folders follow in discovery order.
    {
        Candidate half; half.root = "mods"; half.folder = "Half";
        ModSet::Result r = Resolve({ Cand("Z", Ini("gb.z", "stage = boot\npriority = 50\n")),
                                     Cand("Y", Ini("gb.y", "stage = preboot\npriority = 500\n")),
                                     Cand("X", Ini("gb.x", "stage = boot\npriority = 50\n")),
                                     Cand("Bad", "[mod]\nformat = 9\n"),
                                     Cand("W", Ini("gb.w", "stage = ready\n")),
                                     half });
        CHECK_EQ(r.records.size(), (size_t)6);
        CHECK_EQ(r.records[0].mod.id, "gb.y");
        CHECK_EQ(r.records[1].mod.id, "gb.x");
        CHECK_EQ(r.records[2].mod.id, "gb.z");
        CHECK_EQ(r.records[3].mod.id, "gb.w");
        CHECK_EQ(r.records[3].order, 3);
        CHECK_EQ(r.records[4].folder, "Bad");
        CHECK_EQ(r.records[5].folder, "Half");
        CHECK_EQ(r.records[5].refusal, "gbhook/ has no mod.ini");
    }

    // A version stated in mod.ini beside a modinfo.ini is ignored, and says so.
    {
        Candidate c = Cand("A", Ini("gb.a", "") );
        c.ini = ModIni::Parse("[mod]\nformat = 1\nid = gb.a\nversion = 1.0\n[gbhook]\nabi = 1\n");
        c.hasModInfo = true;
        ModSet::Result r = Resolve({ c });
        CHECK(r.records[0].accepted);
        CHECK_EQ(r.records[0].warnings.size(), (size_t)1);
        CHECK_EQ(r.records[0].warnings[0], "version in mod.ini is ignored: previews/modinfo.ini states it");
    }

    return check::Done("modset");
}
