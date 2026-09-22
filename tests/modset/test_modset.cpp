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
        return std::string("version=\"1.0\"\n[gbhook]\nid = ") + id + "\nabi = 1\n" + extra;
    }

    Candidate Cand(const char* folder, const std::string& ini, Binary binary = Binary::None)
    {
        Candidate c;
        c.root       = "mods";
        c.folder     = folder;
        c.hasModInfo = true;
        c.ini        = ModIni::Parse(ini);
        c.binary     = binary;
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
        CHECK_EQ(r.records[0].mod.version, "1.0");
        CHECK_EQ(r.records[0].refusal, "");
        CHECK_EQ(r.conflicts.size(), (size_t)0);
    }

    // The silent half: gbhook/ without a modinfo.ini.
    {
        Candidate c; c.root = "mods"; c.folder = "Half"; c.hasGbhookDir = true;
        ModSet::Result r = Resolve({ c });
        CHECK(!r.records[0].accepted);
        CHECK_EQ(r.records[0].order, -1);
        CHECK_EQ(r.records[0].refusal, "gbhook/ exists but there is no previews/modinfo.ini");
    }

    // A leftover gbhook/mod.ini is named once, whatever else the folder does.
    {
        Candidate c = Cand("Old", Ini("gb.old")); c.hasModIni = true;
        ModSet::Result r = Resolve({ c });
        CHECK(r.records[0].accepted);
        CHECK_EQ(r.records[0].warnings.size(), (size_t)1);
        CHECK_EQ(r.records[0].warnings[0], "gbhook/mod.ini is no longer read: its keys go under [gbhook] in previews/modinfo.ini");
        Candidate h; h.root = "mods"; h.folder = "Half"; h.hasModIni = true; h.hasGbhookDir = true;
        r = Resolve({ h });
        CHECK_EQ(r.records[0].refusal, "gbhook/ exists but there is no previews/modinfo.ini");
        CHECK_EQ(r.records[0].warnings.size(), (size_t)1);
    }

    // A modinfo.ini refusal and its warnings carry through.
    {
        ModSet::Result r = Resolve({ Cand("B", "[gbhook]\ncolour = red\nabi = 1\n") });
        CHECK_EQ(r.records[0].refusal, "modinfo.ini has no id under [gbhook]");
        CHECK_EQ(r.records[0].warnings.size(), (size_t)1);
    }

    // A plain Mod Manager folder, no [gbhook] section and no gbhook/: a content mod under the folder's own name.
    {
        Candidate m = Cand("My Level", "version=\"1.0\"\ndescription=\"a level\"\n"); m.hasAssets = true;
        ModSet::Result r = Resolve({ m });
        const Record& x = r.records[0];
        CHECK(x.accepted);
        CHECK(x.implicit);
        CHECK_EQ(x.refusal, "");
        CHECK_EQ(x.mod.id, "my_level");
        CHECK_EQ(x.mod.version, "1.0");
        CHECK_EQ(x.mod.description, "a level");
        CHECK_EQ(x.mod.plugin, "");
        CHECK_EQ((int)x.mod.stage, (int)GBH_STAGE_BOOT);
        CHECK_EQ(x.mod.priority, 100);
        CHECK_EQ(x.order, 0);

        // The same with no modinfo.ini at all: the manager would refuse it, gbhook takes the assets.
        Candidate a; a.root = "mods"; a.folder = "Bare"; a.hasAssets = true;
        r = Resolve({ a });
        CHECK(r.records[0].accepted);
        CHECK(r.records[0].implicit);
        CHECK_EQ(r.records[0].mod.id, "bare");
        CHECK_EQ(r.records[0].mod.version, "");

        // A modinfo.ini without the section and without assets is still a mod: the manager lists it, so do we.
        Candidate e = Cand("Empty", "version=\"2\"\n");
        r = Resolve({ e });
        CHECK(r.records[0].accepted);

        // Nothing at all: not a mod.
        Candidate n; n.root = "mods"; n.folder = "Junk";
        r = Resolve({ n });
        CHECK(!r.records[0].accepted);
        CHECK_EQ(r.records[0].refusal, "not a mod: no previews/modinfo.ini and no asset folder");

        // A gbhook/ folder means a DLL was intended, and that still needs the section.
        Candidate d = Cand("Dll", "version=\"1.0\"\n"); d.hasGbhookDir = true; d.hasAssets = true;
        r = Resolve({ d });
        CHECK(!r.records[0].accepted);
        CHECK_EQ(r.records[0].refusal, "previews/modinfo.ini has no [gbhook] section");

        // An explicit id and a folder-derived one collide like any two ids: the first folder keeps it.
        Candidate f = Cand("Zed", Ini("harbor")); f.hasAssets = true;
        Candidate g = Cand("Harbor", "version=\"1.0\"\n"); g.hasAssets = true;
        r = Resolve({ g, f });
        CHECK(Find(r, "Harbor")->accepted);
        CHECK(!Find(r, "Zed")->accepted);
        CHECK(Find(r, "Zed")->refusal.find("duplicate id 'harbor'") != std::string::npos);
    }

    // The plugin named by modinfo.ini must exist, be readable, and pass the manifest checks.
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
                 "modinfo.ini says id 'gb.c' but C.dll says 'gb.other' -- one was edited after the build");
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
                                     Cand("B", "[gbhook]\nid = gb.b\n") });
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
        Candidate half; half.root = "mods"; half.folder = "Half"; half.hasGbhookDir = true;
        ModSet::Result r = Resolve({ Cand("Z", Ini("gb.z", "stage = boot\npriority = 50\n")),
                                     Cand("Y", Ini("gb.y", "stage = preboot\npriority = 500\n")),
                                     Cand("X", Ini("gb.x", "stage = boot\npriority = 50\n")),
                                     Cand("Bad", "[gbhook]\nabi = 9\n"),
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
        CHECK_EQ(r.records[5].refusal, "gbhook/ exists but there is no previews/modinfo.ini");
    }

    // disabled = 1: listed as off, never accepted, no refusal text, and judged no further (a missing DLL is fine).
    {
        ModSet::Result r = Resolve({ Cand("A", Ini("gb.a", "plugin = A.dll\ndisabled = 1\n"), Binary::Missing),
                                     Cand("B", Ini("gb.b", "requires = gb.a\n")) });
        const Record* a = Find(r, "A");
        CHECK(a && !a->accepted && a->disabled);
        CHECK_EQ(a->refusal, "");
        CHECK_EQ(a->order, -1);
        const Record* b = Find(r, "B");
        CHECK(b && !b->accepted && !b->disabled);
        CHECK_EQ(b->refusal, "requires 'gb.a', which is disabled in its modinfo.ini");
    }

    return check::Done("modset");
}
