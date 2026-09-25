// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for cmd/Table: qualified names only, no aliases, duplicates refused by name.

#include "check.h"
#include "cmd/Table.h"

using CmdTable::Qualify;
using CmdTable::Table;

namespace
{
    void F1() {}
    void F2() {}
}

int main()
{
    CHECK_EQ(Qualify("mymod", "Hello"), "mymod.hello");
    CHECK_EQ(Qualify("GbCoop", "net.host"), "gbcoop.net.host");
    CHECK_EQ(Qualify(nullptr, "Help"), "help");
    CHECK_EQ(Qualify("", "level"), "level");

    Table t;
    std::string why;

    // Framework commands are bare; mod commands carry the full id and nothing else.
    CHECK(t.Add(nullptr, "help", (void*)&F1, nullptr, "list commands", 0, &why));
    CHECK(t.Add("mymod", "hello", (void*)&F2, (void*)7, "say hello", 1, &why));
    CHECK(t.Find("mymod.hello") != nullptr);
    CHECK(t.Find("MyMod.HELLO") != nullptr);
    CHECK(t.Find("hello") == nullptr);                 // no alias
    CHECK(t.Find("mymod") == nullptr);                 // nor the id alone
    CHECK_EQ(t.Find("mymod.hello")->owner, "mymod");
    CHECK(t.Find("mymod.hello")->user == (void*)7);
    CHECK_EQ(t.Find("mymod.hello")->flags, 1u);

    // Two mods may both have `hello`; one mod may not register it twice; a mod may not shadow a framework command.
    CHECK(t.Add("other", "hello", (void*)&F1, nullptr, "", 0, &why));
    CHECK(!t.Add("mymod", "HELLO", (void*)&F1, nullptr, "", 0, &why));
    CHECK_EQ(why, "'mymod.hello' is already registered by mymod");
    CHECK(!t.Add(nullptr, "help", (void*)&F1, nullptr, "", 0, &why));
    CHECK_EQ(why, "'help' is already registered by gbhook");

    // Refusals.
    CHECK(!t.Add("mymod", "", (void*)&F1, nullptr, "", 0, &why));
    CHECK_EQ(why, "empty command name");
    CHECK(!t.Add("mymod", "two words", (void*)&F1, nullptr, "", 0, &why));
    CHECK_EQ(why, "a command name cannot contain blanks or quotes");
    CHECK(!t.Add("mymod", "x", nullptr, nullptr, "", 0, &why));
    CHECK_EQ(why, "null handler");
    CHECK_EQ(t.Count(), 3);

    // Help order is alphabetical on the qualified name.
    const std::vector<CmdTable::Entry> s = t.Sorted();
    CHECK_EQ(s[0].name, "help");
    CHECK_EQ(s[1].name, "mymod.hello");
    CHECK_EQ(s[2].name, "other.hello");

    return check::Done("table");
}
