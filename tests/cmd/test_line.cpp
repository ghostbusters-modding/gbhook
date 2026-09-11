// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for cmd/Line: the tokenizer behind every command channel.

#include "check.h"
#include "cmd/Line.h"

using Line::Tokenize;

int main()
{
    {
        auto t = Tokenize("level  haunt1\tcheckpoint_BeginLevel");
        CHECK_EQ(t.size(), (size_t)3);
        CHECK_EQ(t[0], "level");
        CHECK_EQ(t[1], "haunt1");
        CHECK_EQ(t[2], "checkpoint_BeginLevel");
    }
    // Quotes keep a space inside one word and vanish from it; an empty quoted word survives.
    {
        auto t = Tokenize("hud 3 \"HELLO WORLD\" \"\" tail");
        CHECK_EQ(t.size(), (size_t)5);
        CHECK_EQ(t[2], "HELLO WORLD");
        CHECK_EQ(t[3], "");
        CHECK_EQ(t[4], "tail");
    }
    // A quote glued to a word joins it; an unclosed quote runs to the end.
    CHECK_EQ(Tokenize("say\"hi there\"x")[0], "sayhi therex");
    CHECK_EQ(Tokenize("say \"to the end")[1], "to the end");
    CHECK_EQ(Tokenize("").size(), (size_t)0);
    CHECK_EQ(Tokenize("   \t ").size(), (size_t)0);

    CHECK(Line::IsSkippable(""));
    CHECK(Line::IsSkippable("   "));
    CHECK(Line::IsSkippable("# a comment"));
    CHECK(Line::IsSkippable("  # indented"));
    CHECK(!Line::IsSkippable("help"));
    CHECK(!Line::IsSkippable("level x # not a comment"));

    CHECK_EQ(Line::StripEol("ping\r\n"), "ping");
    CHECK_EQ(Line::StripEol("ping\n"), "ping");
    CHECK_EQ(Line::StripEol("ping  "), "ping  ");
    CHECK_EQ(Line::StripEol("\r\n"), "");

    return check::Done("line");
}
