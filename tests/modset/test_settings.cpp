// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for modset/SettingsTable: gbhook.ini over a mod's own defaults, keyed by the full mod id.

#include "check.h"
#include "modset/SettingsTable.h"

using SettingsTable::Build;
using SettingsTable::Defaults;
using SettingsTable::Get;
using SettingsTable::Qualify;
using SettingsTable::Table;

namespace
{
    std::string S(const char* p) { return p ? std::string(p) : std::string("<null>"); }
}

int main()
{
    // Qualify: the full id is the namespace, lowered; a framework key has none.
    CHECK_EQ(Qualify("gb.qol", "bootskip"), "gb.qol.bootskip");
    CHECK_EQ(Qualify("GB.QoL", "BootSkip"), "gb.qol.bootskip");
    CHECK_EQ(Qualify(nullptr, "Mods.Root"), "mods.root");
    CHECK_EQ(Qualify("", "mods.root"), "mods.root");

    const Ini::Document ini = Ini::Parse("mods.root = mods\ngb.qol.bootskip = 0\nGB.QOL.Late = Yes\ngb.mymod.greeting = from ini\n");
    const std::vector<Defaults> mods = {
        { "gb.qol",   { { "bootskip", "1" }, { "bootskip.screens", "1" } } },
        { "gb.mymod", { { "greeting", "hello" }, { "bootskip", "unrelated" } } },
    };
    const Table t = Build(ini, mods);

    // gbhook.ini wins over the mod's default.
    CHECK_EQ(S(Get(t, "gb.qol", "bootskip")), "0");
    // The default answers when gbhook.ini is silent.
    CHECK_EQ(S(Get(t, "gb.qol", "bootskip.screens")), "1");
    // Neither layer: nullptr, so the caller's default applies.
    CHECK_EQ(S(Get(t, "gb.qol", "missing")), "<null>");
    // Case never matters for keys or ids; values keep their case.
    CHECK_EQ(S(Get(t, "GB.QOL", "LATE")), "Yes");
    // gbhook.ini wins over the mod's own modinfo.ini key.
    CHECK_EQ(S(Get(t, "gb.mymod", "greeting")), "from ini");
    // Two mods with one key name do not collide.
    CHECK_EQ(S(Get(t, "gb.mymod", "bootskip")), "unrelated");
    // Framework keys read with no id and never fall through to a mod default.
    CHECK_EQ(S(Get(t, nullptr, "mods.root")), "mods");
    CHECK_EQ(S(Get(t, nullptr, "bootskip")), "<null>");
    CHECK_EQ(S(Get(t, nullptr, "gb.qol.bootskip")), "0");
    // Empty key: nothing.
    CHECK_EQ(S(Get(t, "gb.qol", "")), "<null>");
    CHECK_EQ(t.warnings.size(), (size_t)0);

    // Last duplicate in gbhook.ini wins, matching Ini::Find.
    {
        const Table d = Build(Ini::Parse("gb.a.k = 1\ngb.a.k = 2\n"), {});
        CHECK_EQ(S(Get(d, "gb.a", "k")), "2");
    }

    // A modinfo.ini key that repeats the mod id is read doubled, and the mod is warned.
    {
        const Table w = Build(Ini::Parse(""), { { "gb.a", { { "gb.a.k", "v" } } } });
        CHECK_EQ(w.warnings.size(), (size_t)1);
        CHECK_EQ(w.warnings[0], "gb.a: modinfo.ini key 'gb.a.k' repeats the mod id; it is read as 'gb.a.gb.a.k'");
        CHECK_EQ(S(Get(w, "gb.a", "gb.a.k")), "v");
        CHECK_EQ(S(Get(w, "gb.a", "k")), "<null>");
    }

    return check::Done("settings");
}
