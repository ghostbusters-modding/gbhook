// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for format/Ini: the one grammar behind gbhook.ini and modinfo.ini.

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

    // A double-quoted value: quotes dropped, '#' and ';' inside kept, a comment after the closing quote cut.
    {
        Ini::Document d = Parse("version=\"1.0.1\"\nd=\"a; b # c\"\ne = \"x\"  ; note\nf=\"\"\ng = \" padded \"\n");
        CHECK_EQ(d.malformed.size(), (size_t)0);
        CHECK_EQ(Value(d, "version"), "1.0.1");
        CHECK_EQ(Value(d, "d"), "a; b # c");
        CHECK_EQ(Value(d, "e"), "x");
        CHECK_EQ(Value(d, "f"), "");
        CHECK_EQ(Value(d, "g"), " padded ");
    }

    // An unclosed quote is an ordinary value, comment rule and all; a quote later in a value is not special.
    CHECK_EQ(Value(Parse("a = \"x ; y"), "a"), "\"x");
    CHECK_EQ(Value(Parse("a = x \"y\" z"), "a"), "x \"y\" z");
    CHECK_EQ(Parse("# \"quoted\" = comment\n").entries.size(), (size_t)0);

    // A header is skipped, never a prefix, and a broken one is still malformed.
    {
        Ini::Document d = Parse("[gbhook]\nid = gb.x\n[ settings ]\nnet.port = 1\n[]\nk = 2\n[bad\n");
        CHECK_EQ(d.entries.size(), (size_t)3);
        CHECK_EQ(d.entries[0].key, "id");
        CHECK_EQ(d.entries[1].key, "net.port");
        CHECK_EQ(d.entries[2].key, "k");
        CHECK_EQ(d.malformed.size(), (size_t)1);
        CHECK_EQ(d.malformed[0], 7);
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

    // Set: one line changed in place, everything else byte for byte.
    {
        CHECK_EQ(Ini::Set("", "mods.disabled", "gb.a"), "mods.disabled = gb.a\n");
        CHECK_EQ(Ini::Set("mods.root = mods\n", "mods.disabled", "gb.a"), "mods.root = mods\nmods.disabled = gb.a\n");
        CHECK_EQ(Ini::Set("a = 1\r\n\r\n", "b", "2"), "a = 1\r\nb = 2\r\n\r\n");
        CHECK_EQ(Ini::Set("# top\nmods.disabled = gb.a   # off for now\nx = 1\n", "mods.disabled", "gb.a, gb.b"),
                 "# top\nmods.disabled = gb.a, gb.b   # off for now\nx = 1\n");
        CHECK_EQ(Ini::Set("MODS.Disabled=gb.a", "mods.disabled", ""), "MODS.Disabled = ");
        CHECK_EQ(Ini::Set("k = \"a;b\" ; note\n", "k", "c"), "k = c   ; note\n");
        // Last one wins when read, so the last one is the one changed.
        CHECK_EQ(Ini::Set("k = 1\nk = 2\n", "k", "3"), "k = 1\nk = 3\n");
        // A header changes nothing: a new key goes at the end, past it.
        CHECK_EQ(Ini::Set("a = 1\n[gb.x]\nk = 2\n", "mods.disabled", "gb.b"), "a = 1\n[gb.x]\nk = 2\nmods.disabled = gb.b\n");
    }

    return check::Done("ini");
}
