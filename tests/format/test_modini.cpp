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
    "id          = gb.mymod              ; trailing note\n"
    "abi         = 1\n"
    "plugin      = MyMod.dll\n"
    "scripts     = scripts/\n"
    "content     = content/A.POD, content/B.POD\n"
    "stage       = early\n"
    "priority    = 50\n"
    "requires    = gb.sensors, gb.core\n"
    "\n"
    "greeting    = hello\n"
    "net.port    = 12345\n";

static const char* kMinimal = "id = gb.x\nabi = 1\n";

// The same record in the sectioned form older mods shipped with.
static const char* kSectioned =
    "version=\"0.1.0\"\n"
    "[gbhook]\n"
    "id       = gb.mymod\n"
    "abi      = 1\n"
    "plugin   = MyMod.dll\n"
    "[settings]\n"
    "greeting = hello\n"
    "net.port = 12345\n";

static std::string Setting(const ModIni::Result& r, const std::string& key)
{
    std::string found = "<none>";
    for (const auto& kv : r.mod.settings) if (kv.first == key) found = kv.second;
    return found;
}

int main()
{
    // The full record: the Mod Manager's quoted keys, ours, and settings, all bare.
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
        CHECK_EQ(r.mod.settings.size(), (size_t)14);
        CHECK_EQ(Setting(r, "greeting"), "hello");
        CHECK_EQ(Setting(r, "net.port"), "12345");
        CHECK_EQ(Setting(r, "version"), "0.1.0");      // shared with the Mod Manager
        CHECK_EQ(Setting(r, "id"), "gb.mymod");
    }

    // Section headers are skipped, so the old form reads as the flat one.
    {
        ModIni::Result r = Parse(kSectioned);
        CHECK(r.gbhook);
        CHECK_EQ(r.refusal, "");
        CHECK_EQ(r.warnings.size(), (size_t)0);
        CHECK_EQ(r.mod.id, "gb.mymod");
        CHECK_EQ(r.mod.version, "0.1.0");
        CHECK_EQ(r.mod.plugin, "MyMod.dll");
        CHECK_EQ(Setting(r, "greeting"), "hello");
        CHECK_EQ(Setting(r, "net.port"), "12345");
        CHECK_EQ(Setting(r, "settings.greeting"), "<none>");
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

    // None of our keys: the Mod Manager's file alone. Not a refusal; its version and description still read.
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
    CHECK_EQ(Parse("id = gb.x\njunk\nabi = 1\n").refusal, "modinfo.ini line 2 is not key = value");

    // id
    CHECK_EQ(Parse("abi = 1\n").refusal, "modinfo.ini has no id");
    CHECK_EQ(Parse("id =\nabi = 1\n").refusal, "modinfo.ini has no id");
    CHECK_EQ(Parse("id = gb x\nabi = 1\n").refusal, "id 'gb x' contains whitespace");
    {
        std::string longId(64, 'a');
        CHECK_EQ(Parse("id = " + longId + "\nabi = 1\n").refusal, "id is longer than 63 characters");
        std::string okId(63, 'a');
        CHECK_EQ(Parse("id = " + okId + "\nabi = 1\n").refusal, "");
    }

    // abi
    CHECK_EQ(Parse("id = gb.x\n").refusal, "modinfo.ini has no abi");
    CHECK_EQ(Parse("id = gb.x\nabi = 0.1.0\n").refusal, "abi '0.1.0' is not a number (the integer GBHOOK_ABI_VERSION)");
    CHECK_EQ(Parse("id = gb.x\nabi = 2\n").refusal, "built for ABI 2, this gbhook speaks ABI 1 -- rebuild the mod");

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

    // disabled moved to gbhook.ini: the old key is named, and the mod still loads.
    {
        ModIni::Result r = Parse(std::string(kMinimal) + "disabled = 1\n");
        CHECK_EQ(r.refusal, "");
        CHECK_EQ(r.warnings.size(), (size_t)1);
        CHECK(r.warnings[0].find("mods.disabled in gbhook.ini") != std::string::npos);
    }

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

    // Any other key is a setting, never a warning, whatever section it sat under.
    {
        ModIni::Result r = Parse("author=\"me\"\n[gbhook]\nid = gb.x\nabi = 1\ncolour = red\n[other]\nk = v\n");
        CHECK_EQ(r.refusal, "");
        CHECK_EQ(r.warnings.size(), (size_t)0);
        CHECK_EQ(Setting(r, "author"), "me");
        CHECK_EQ(Setting(r, "colour"), "red");
        CHECK_EQ(Setting(r, "k"), "v");
    }

    // The first refusal wins; later problems are not reported over it.
    CHECK_EQ(Parse("abi = 9\nplugin = ../x.dll\n").refusal, "modinfo.ini has no id");

    return check::Done("modini");
}
