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
    CHECK_EQ(Qualify("qol", "bootskip"), "qol.bootskip");
    CHECK_EQ(Qualify("QoL", "BootSkip"), "qol.bootskip");
    CHECK_EQ(Qualify(nullptr, "Mods_Root"), "mods_root");
    CHECK_EQ(Qualify("", "mods_root"), "mods_root");

    const Ini::Document ini = Ini::Parse("mods_root = mods\nqol.bootskip = 0\nQOL.Late = Yes\nmymod.greeting = from ini\n");
    const std::vector<Defaults> mods = {
        { "qol",   { { "bootskip", "1" }, { "bootskip.screens", "1" } } },
        { "mymod", { { "greeting", "hello" }, { "bootskip", "unrelated" } } },
    };
    const Table t = Build(ini, mods);

    // gbhook.ini wins over the mod's default.
    CHECK_EQ(S(Get(t, "qol", "bootskip")), "0");
    // The default answers when gbhook.ini is silent.
    CHECK_EQ(S(Get(t, "qol", "bootskip.screens")), "1");
    // Neither layer: nullptr, so the caller's default applies.
    CHECK_EQ(S(Get(t, "qol", "missing")), "<null>");
    // Case never matters for keys or ids; values keep their case.
    CHECK_EQ(S(Get(t, "QOL", "LATE")), "Yes");
    // gbhook.ini wins over the mod's own modinfo.ini key.
    CHECK_EQ(S(Get(t, "mymod", "greeting")), "from ini");
    // Two mods with one key name do not collide.
    CHECK_EQ(S(Get(t, "mymod", "bootskip")), "unrelated");
    // Framework keys read with no id and never fall through to a mod default.
    CHECK_EQ(S(Get(t, nullptr, "mods_root")), "mods");
    CHECK_EQ(S(Get(t, nullptr, "bootskip")), "<null>");
    CHECK_EQ(S(Get(t, nullptr, "qol.bootskip")), "0");
    // Empty key: nothing.
    CHECK_EQ(S(Get(t, "qol", "")), "<null>");
    CHECK_EQ(t.warnings.size(), (size_t)0);

    // Last duplicate in gbhook.ini wins, matching Ini::Find.
    {
        const Table d = Build(Ini::Parse("a.k = 1\na.k = 2\n"), {});
        CHECK_EQ(S(Get(d, "a", "k")), "2");
    }

    // A modinfo.ini key that repeats the mod id is read doubled, and the mod is warned.
    {
        const Table w = Build(Ini::Parse(""), { { "a", { { "a.k", "v" } } } });
        CHECK_EQ(w.warnings.size(), (size_t)1);
        CHECK_EQ(w.warnings[0], "a: modinfo.ini key 'a.k' repeats the mod id; it is read as 'a.a.k'");
        CHECK_EQ(S(Get(w, "a", "a.k")), "v");
        CHECK_EQ(S(Get(w, "a", "k")), "<null>");
    }

    return check::Done("settings");
}
