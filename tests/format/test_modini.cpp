// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for format/ModIni: modinfo.ini into a record, with the exact refusal for each way it can be wrong.

#include "check.h"
#include "format/ModIni.h"

using ModIni::Parse;

static const char* kFull =
    "version=\"0.1.0\"\n"
    "compatibility=\"PC\"\n"
    "description=\"A local test mod; with a # in it\"\n"
    "link=\"\"\n"
    "\n"
    "[gbhook]\n"
    "id          = gb.mymod              ; trailing note\n"
    "abi         = 1\n"
    "plugin      = MyMod.dll\n"
    "scripts     = scripts/\n"
    "content     = content/A.POD, content/B.POD\n"
    "stage       = early\n"
    "priority    = 50\n"
    "requires    = gb.sensors, gb.core\n"
    "\n"
    "[settings]\n"
    "greeting    = hello\n"
    "net.port    = 12345\n";

static const char* kMinimal = "[gbhook]\nid = gb.x\nabi = 1\n";

int main()
{
    // The full record: the Mod Manager's quoted keys on top, ours in [gbhook].
    {
        ModIni::Result r = Parse(kFull);
        CHECK(r.gbhook);
        CHECK_EQ(r.refusal, "");
        CHECK_EQ(r.warnings.size(), (size_t)0);
        CHECK_EQ(r.mod.id, "gb.mymod");
        CHECK_EQ(r.mod.version, "0.1.0");
        CHECK_EQ(r.mod.description, "A local test mod; with a # in it");
        CHECK_EQ(r.mod.abi, 1);
        CHECK_EQ(r.mod.plugin, "MyMod.dll");
        CHECK_EQ(r.mod.scripts, "scripts/");
        CHECK_EQ(r.mod.content.size(), (size_t)2);
        CHECK_EQ(r.mod.content[1], "content/B.POD");
        CHECK_EQ((int)r.mod.stage, (int)GBH_STAGE_EARLY);
        CHECK_EQ(r.mod.priority, 50);
        CHECK_EQ(r.mod.requires_.size(), (size_t)2);
        CHECK_EQ(r.mod.requires_[0], "gb.sensors");
        CHECK_EQ(r.mod.settings.size(), (size_t)2);
        CHECK_EQ(r.mod.settings[0].first, "greeting");
        CHECK_EQ(r.mod.settings[0].second, "hello");
        CHECK_EQ(r.mod.settings[1].first, "net.port");
    }

    // The two-line minimum, with the defaults.
    {
        ModIni::Result r = Parse(kMinimal);
        CHECK_EQ(r.refusal, "");
        CHECK_EQ((int)r.mod.stage, (int)GBH_STAGE_BOOT);
        CHECK_EQ(r.mod.priority, 100);
        CHECK_EQ(r.mod.plugin, "");
        CHECK_EQ(r.mod.version, "");
        CHECK_EQ(r.mod.content.size(), (size_t)0);
        CHECK_EQ(r.mod.requires_.size(), (size_t)0);
    }

    // No [gbhook] section: the Mod Manager's file alone. Not a refusal; its version and description still read.
    {
        ModIni::Result r = Parse("version=\"1.0\"\ncompatibility=\"PC\"\ndescription=\"x\"\nlink=\"\"\n");
        CHECK(!r.gbhook);
        CHECK_EQ(r.refusal, "");
        CHECK_EQ(r.mod.version, "1.0");
        CHECK_EQ(r.mod.description, "x");
        CHECK_EQ(r.mod.id, "");
        CHECK(!Parse("").gbhook);
        CHECK_EQ(Parse("").refusal, "");
        CHECK(!Parse("junk\n[settings]\na = 1\n").gbhook);        // the manager's file is not judged, malformed or not
        CHECK_EQ(Parse("junk\n[settings]\na = 1\n").refusal, "");
        CHECK_EQ(Parse("[General]\nversion=\"3.1\"\n").mod.version, "3.1");
        CHECK_EQ(Parse("version=\"\"\n[General]\nversion=\"3.2\"\n").mod.version, "3.2");   // empty reads as absent
        CHECK(Parse("[GBHook]\nid = gb.x\n").gbhook);
    }

    // The manager's keys read unquoted too, and from [General] where QSettings puts them.
    CHECK_EQ(Parse(std::string("version = 2.0\n") + kMinimal).mod.version, "2.0");
    CHECK_EQ(Parse(std::string("[General]\nversion=\"3.0\"\ndescription=\"d\"\n") + kMinimal).mod.version, "3.0");
    CHECK_EQ(Parse(std::string("[General]\nversion=\"3.0\"\ndescription=\"d\"\n") + kMinimal).mod.description, "d");
    CHECK_EQ(Parse(std::string("[General]\nversion=\"3.0\"\n") + kMinimal).warnings.size(), (size_t)0);

    // A malformed line refuses the whole file, by number.
    CHECK_EQ(Parse("[gbhook]\nid = gb.x\njunk\nabi = 1\n").refusal, "modinfo.ini line 3 is not key = value");

    // id
    CHECK_EQ(Parse("[gbhook]\nabi = 1\n").refusal, "modinfo.ini has no id under [gbhook]");
    CHECK_EQ(Parse("[gbhook]\nid =\nabi = 1\n").refusal, "modinfo.ini has no id under [gbhook]");
    CHECK_EQ(Parse("[gbhook]\nid = gb x\nabi = 1\n").refusal, "id 'gb x' contains whitespace");
    {
        std::string longId(64, 'a');
        CHECK_EQ(Parse("[gbhook]\nid = " + longId + "\nabi = 1\n").refusal, "id is longer than 63 characters");
        std::string okId(63, 'a');
        CHECK_EQ(Parse("[gbhook]\nid = " + okId + "\nabi = 1\n").refusal, "");
    }

    // abi
    CHECK_EQ(Parse("[gbhook]\nid = gb.x\n").refusal, "modinfo.ini has no abi under [gbhook]");
    CHECK_EQ(Parse("[gbhook]\nid = gb.x\nabi = 0.1.0\n").refusal, "abi '0.1.0' is not a number (the integer GBHOOK_ABI_VERSION)");
    CHECK_EQ(Parse("[gbhook]\nid = gb.x\nabi = 2\n").refusal, "built for ABI 2, this gbhook speaks ABI 1 -- rebuild the mod");

    // stage, case-insensitive, and every name
    {
        const char* names[4] = { "preboot", "early", "boot", "ready" };
        for (int i = 0; i < 4; ++i)
        {
            ModIni::Result r = Parse(std::string(kMinimal) + "stage = " + names[i] + "\n");
            CHECK_EQ(r.refusal, "");
            CHECK_EQ((int)r.mod.stage, i);
            CHECK_EQ(std::string(ModIni::StageName((GbhStage)i)), names[i]);
        }
        CHECK_EQ((int)Parse(std::string(kMinimal) + "stage = Boot\n").mod.stage, (int)GBH_STAGE_BOOT);
        CHECK_EQ(Parse(std::string(kMinimal) + "stage = 2\n").refusal, "stage '2' is not one of preboot, early, boot, ready");
        GbhStage s;
        CHECK(ModIni::StageFromName("READY", &s));
        CHECK_EQ((int)s, (int)GBH_STAGE_READY);
        CHECK(!ModIni::StageFromName("later", &s));
    }

    // The manual disable switch, default off, read as a bool.
    CHECK_EQ(Parse(kMinimal).mod.disabled, false);
    CHECK_EQ(Parse(std::string(kMinimal) + "disabled = 1\n").mod.disabled, true);
    CHECK_EQ(Parse(std::string(kMinimal) + "disabled = yes\n").mod.disabled, true);
    CHECK_EQ(Parse(std::string(kMinimal) + "disabled = 0\n").mod.disabled, false);

    // priority
    CHECK_EQ(Parse(std::string(kMinimal) + "priority = high\n").refusal, "priority 'high' is not a number");
    CHECK_EQ(Parse(std::string(kMinimal) + "priority = -5\n").mod.priority, -5);

    // Paths stay inside gbhook/.
    CHECK_EQ(Parse(std::string(kMinimal) + "plugin = ..\\Other.dll\n").refusal, "plugin '..\\Other.dll' leaves gbhook/");
    CHECK_EQ(Parse(std::string(kMinimal) + "plugin = C:\\x\\Other.dll\n").refusal, "plugin 'C:\\x\\Other.dll' leaves gbhook/");
    CHECK_EQ(Parse(std::string(kMinimal) + "plugin = /abs/Other.dll\n").refusal, "plugin '/abs/Other.dll' leaves gbhook/");
    CHECK_EQ(Parse(std::string(kMinimal) + "scripts = ../s\n").refusal, "scripts '../s' leaves gbhook/");
    CHECK_EQ(Parse(std::string(kMinimal) + "content = content/A.POD, ../B.POD\n").refusal, "content '../B.POD' leaves gbhook/");
    CHECK_EQ(Parse(std::string(kMinimal) + "plugin = bin/Inner.dll\n").refusal, "");

    // Unknown keys in a section warn and do not refuse. Top-level keys are the manager's and are never judged.
    {
        ModIni::Result r = Parse("author=\"me\"\n[mod]\nformat = 1\nid = gb.x\n[gbhook]\nid = gb.x\nabi = 1\ncolour = red\n[other]\nk = v\n");
        CHECK_EQ(r.refusal, "");
        CHECK_EQ(r.warnings.size(), (size_t)4);
        CHECK_EQ(r.warnings[0], "line 3: unknown key 'mod.format'");
        CHECK_EQ(r.warnings[1], "line 4: unknown key 'mod.id'");
        CHECK_EQ(r.warnings[2], "line 8: unknown key 'gbhook.colour'");
        CHECK_EQ(r.warnings[3], "line 10: unknown key 'other.k'");
    }

    // The first refusal wins; later problems are not reported over it.
    CHECK_EQ(Parse("[gbhook]\nabi = 9\nplugin = ../x.dll\n").refusal, "modinfo.ini has no id under [gbhook]");

    return check::Done("modini");
}
