// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for format/Ini: the one grammar behind gbhook.ini and mod.ini.

#include "check.h"
#include "format/Ini.h"

using Ini::Parse;
using Ini::Find;

static std::string Value(const Ini::Document& d, const char* key)
{
    const std::string* v = Find(d, key);
    return v ? *v : std::string("<missing>");
}

int main()
{
    // Empty and blank input.
    {
        Ini::Document d = Parse("");
        CHECK_EQ(d.entries.size(), (size_t)0);
        CHECK_EQ(d.malformed.size(), (size_t)0);
        d = Parse("\n\n   \n\t\n");
        CHECK_EQ(d.entries.size(), (size_t)0);
        CHECK_EQ(d.malformed.size(), (size_t)0);
    }

    // The basic pair, trimmed on both sides, numbered from line 1.
    {
        Ini::Document d = Parse("  a   =   hello world  \n");
        CHECK_EQ(d.entries.size(), (size_t)1);
        CHECK_EQ(d.entries[0].key, "a");
        CHECK_EQ(d.entries[0].value, "hello world");
        CHECK_EQ(d.entries[0].line, 1);
    }

    // Tabs separate as well as spaces.
    CHECK_EQ(Value(Parse("a\t=\t1"), "a"), "1");

    // Keys fold to lower case; values keep theirs.
    {
        Ini::Document d = Parse("MODS.Root = Mods");
        CHECK_EQ(d.entries[0].key, "mods.root");
        CHECK_EQ(d.entries[0].value, "Mods");
    }

    // Comments: whole-line and mid-line, '#' and ';' alike.
    {
        Ini::Document d = Parse("# c1\n; c2\na = 1 ; note\nb = 2 # note\n   # indented\n");
        CHECK_EQ(d.entries.size(), (size_t)2);
        CHECK_EQ(Value(d, "a"), "1");
        CHECK_EQ(Value(d, "b"), "2");
        CHECK_EQ(d.malformed.size(), (size_t)0);
    }

    // A section folds into the key as a prefix, trimmed and lowercased.
    {
        Ini::Document d = Parse("[Mod]\nid = gb.x\n[ GBHOOK ]\nplugin = X.dll\n");
        CHECK_EQ(d.entries[0].key, "mod.id");
        CHECK_EQ(d.entries[1].key, "gbhook.plugin");
        CHECK_EQ(Value(d, "gbhook.plugin"), "X.dll");
    }

    // An empty section header returns to the global namespace.
    {
        Ini::Document d = Parse("[a]\nk = 1\n[]\nk = 2\n");
        CHECK_EQ(d.entries[0].key, "a.k");
        CHECK_EQ(d.entries[1].key, "k");
    }

    // The first '=' splits; later ones belong to the value.
    CHECK_EQ(Value(Parse("a = b = c"), "a"), "b = c");

    // An empty value is an entry, not an omission.
    {
        Ini::Document d = Parse("a =\n");
        CHECK_EQ(d.entries.size(), (size_t)1);
        CHECK(Find(d, "a") != nullptr);
        CHECK_EQ(Value(d, "a"), "");
    }

    // Malformed lines are reported by number and produce no entry.
    {
        Ini::Document d = Parse("ok = 1\njusttext\n= novalue\n[unterminated\n");
        CHECK_EQ(d.entries.size(), (size_t)1);
        CHECK_EQ(d.malformed.size(), (size_t)3);
        CHECK_EQ(d.malformed[0], 2);
        CHECK_EQ(d.malformed[1], 3);
        CHECK_EQ(d.malformed[2], 4);
    }

    // Line numbers count blank and comment lines.
    CHECK_EQ(Parse("\n# c\na = 1").entries[0].line, 3);

    // CRLF endings and a final line without a newline.
    {
        Ini::Document d = Parse("a = 1\r\nb = 2");
        CHECK_EQ(d.entries.size(), (size_t)2);
        CHECK_EQ(d.entries[0].value, "1");
        CHECK_EQ(d.entries[1].value, "2");
    }

    // A UTF-8 BOM, which Notepad writes, is not part of the first key.
    CHECK_EQ(Parse("\xEF\xBB\xBF" "a = 1\n").entries[0].key, "a");

    // Duplicates are kept in order; Find takes the last, case-insensitively.
    {
        Ini::Document d = Parse("a = 1\nA = 2\n");
        CHECK_EQ(d.entries.size(), (size_t)2);
        CHECK_EQ(Value(d, "a"), "2");
        CHECK_EQ(Value(d, "A"), "2");
        CHECK(Find(d, "missing") == nullptr);
    }

    // Lists: comma-separated, trimmed, empties dropped.
    {
        std::vector<std::string> l = Ini::List("a, b ,c");
        CHECK_EQ(l.size(), (size_t)3);
        CHECK_EQ(l[0], "a");
        CHECK_EQ(l[1], "b");
        CHECK_EQ(l[2], "c");
        CHECK_EQ(Ini::List("").size(), (size_t)0);
        CHECK_EQ(Ini::List("a,,b,").size(), (size_t)2);
        CHECK_EQ(Ini::List(" x ")[0], "x");
        Ini::Document d = Parse("content = content/A.POD, content/B.POD");
        CHECK_EQ(Ini::List(Value(d, "content")).size(), (size_t)2);
        CHECK_EQ(Ini::List(Value(d, "content"))[1], "content/B.POD");
    }

    // Typed readers. Empty or unparsable means the default.
    {
        CHECK_EQ(Ini::ToInt("12", -1), 12);
        CHECK_EQ(Ini::ToInt("-5", -1), -5);
        CHECK_EQ(Ini::ToInt("0x10", -1), 16);
        CHECK_EQ(Ini::ToInt("12abc", -1), 12);
        CHECK_EQ(Ini::ToInt("", 7), 7);
        CHECK_EQ(Ini::ToInt("abc", 7), 7);

        CHECK_EQ(Ini::ToFloat("1.5", -1.f), 1.5f);
        CHECK_EQ(Ini::ToFloat("", 2.f), 2.f);
        CHECK_EQ(Ini::ToFloat("x", 2.f), 2.f);

        for (const char* t : { "1", "true", "yes", "on", "On", "Y", "TRUE" })
            CHECK_EQ(Ini::ToBool(t, false), true);
        for (const char* f : { "0", "false", "no", "off", "OFF", "F", "No" })
            CHECK_EQ(Ini::ToBool(f, true), false);
        CHECK_EQ(Ini::ToBool("", true), true);
        CHECK_EQ(Ini::ToBool("", false), false);
    }

    return check::Done("ini");
}
