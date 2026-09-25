// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for format/LogLine: the greppable line grammar every log writer shares.

#include "check.h"
#include "format/LogLine.h"

#include <cstring>
#include <string>

static std::string Line(const char* tag, const char* id, const char* text)
{
    char buf[256];
    memset(buf, 'X', sizeof buf);
    LogLine::Format(buf, sizeof buf, 1, 2, 3, tag, id, text);
    return buf;
}

int main()
{
    // Framework line: stamp, tag padded to four columns, one space, text.
    CHECK_EQ(Line("BOOT", nullptr, "attached"), "[01:02:03] BOOT attached");
    CHECK_EQ(Line("INI", nullptr, "3 settings"), "[01:02:03] INI  3 settings");

    // Mod line: the id in brackets between tag and text.
    CHECK_EQ(Line("NET", "gbcoop", "listening"), "[01:02:03] NET  [gbcoop] listening");

    // An empty id is the framework, same as null.
    CHECK_EQ(Line("NET", "", "x"), "[01:02:03] NET  x");

    // Null tag and null text are tolerated, not crashed on.
    CHECK_EQ(Line(nullptr, nullptr, "x"), "[01:02:03]      x");
    CHECK_EQ(Line("BOOT", nullptr, nullptr), "[01:02:03] BOOT ");

    // A long tag is not cut; four columns is a minimum.
    CHECK_EQ(Line("PLUGIN", nullptr, "x"), "[01:02:03] PLUGIN x");

    // Two-digit zero padding on every field.
    {
        char buf[64];
        LogLine::Format(buf, sizeof buf, 0, 0, 0, "T", nullptr, "");
        CHECK_EQ(std::string(buf).substr(0, 10), "[00:00:00]");
        LogLine::Format(buf, sizeof buf, 23, 59, 59, "T", nullptr, "");
        CHECK_EQ(std::string(buf).substr(0, 10), "[23:59:59]");
    }

    // The return value is the length written.
    {
        char buf[64];
        size_t n = LogLine::Format(buf, sizeof buf, 1, 2, 3, "BOOT", "id", "text");
        CHECK_EQ(n, strlen(buf));
        CHECK_EQ(n, (size_t)25);
    }

    // Truncation keeps the prefix, terminates, and reports what fit.
    {
        char buf[16];
        size_t n = LogLine::Format(buf, sizeof buf, 1, 2, 3, "BOOT", nullptr, "a long message here");
        CHECK_EQ(n, (size_t)15);
        CHECK_EQ(std::string(buf), "[01:02:03] BOOT");
    }

    // Degenerate capacities never write past the buffer.
    {
        char buf[4] = { 'a', 'b', 'c', 'd' };
        CHECK_EQ(LogLine::Format(buf, 0, 1, 2, 3, "T", nullptr, "x"), (size_t)0);
        CHECK_EQ(buf[0], 'a');
        CHECK_EQ(LogLine::Format(buf, 1, 1, 2, 3, "T", nullptr, "x"), (size_t)0);
        CHECK_EQ(buf[0], '\0');
        CHECK_EQ(buf[1], 'b');
    }

    return check::Done("logline");
}
