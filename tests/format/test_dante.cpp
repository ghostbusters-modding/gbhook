// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for format/Dante: the registered checkpoints of a script, from its STRINGS section only.

#include "check.h"
#include "format/Dante.h"

#include <cstring>

int main()
{
    // The shape of a real container, cut down: a helper export that STRINGS never names, CRLF line ends,
    // a duplicate, and a string that merely mentions a checkpoint.
    const char* text =
        "// DANTE compiled program\r\n"
        "\r\n"
        "BEGIN ASSUMPTIONS\r\n"
        "S \"sizeof(SSpawnInfo)\" 12\r\n"
        "END ASSUMPTIONS\r\n"
        "\r\n"
        "BEGIN STRINGS\r\n"
        "\r\n"
        "00000000 \"CScuttler\"\r\n"
        "00000004 \"scuttler\\\\Mini.cit\"\r\n"
        "00000008 \"void checkpoint_Start()\"\r\n"
        "0000000C \"The duel range\"\r\n"
        "00000010 \"void checkpoint_Lobby2a()\"\r\n"
        "00000014 \"void checkpoint_Start()\"\r\n"
        "00000018 \"see checkpoint_Kitchen later\"\r\n"
        "0000001C \"void checkpoint_()\"\r\n"
        "\r\n"
        "END STRINGS\r\n"
        "\r\n"
        "BEGIN CODE\r\n"
        "END CODE\r\n"
        "\r\n"
        "BEGIN EXPORTS\r\n"
        "C 00000139 \"void setupLevel()\"\r\n"
        "C 0000017C \"void checkpoint_Start()\"\r\n"
        "C 00000180 \"void checkpoint_Lobby2a()\"\r\n"
        "C 00000190 \"void checkpoint_Stage3()\"\r\n"
        "END EXPORTS\r\n"
        "END PROGRAM\r\n";

    std::vector<std::string> cps = Dante::Checkpoints(text, strlen(text));
    CHECK_EQ(cps.size(), (size_t)2);
    CHECK_EQ(cps[0], "checkpoint_Start");
    CHECK_EQ(cps[1], "checkpoint_Lobby2a");

    // LF only, the same answer.
    std::string lf(text);
    for (size_t p; (p = lf.find("\r")) != std::string::npos;) lf.erase(p, 1);
    CHECK_EQ(Dante::Checkpoints(lf.data(), lf.size()).size(), (size_t)2);

    // No STRINGS section, an unterminated one, nothing at all: empty, never a fault.
    CHECK_EQ(Dante::Checkpoints("BEGIN EXPORTS\nC 0 \"void checkpoint_X()\"\nEND EXPORTS\n", 60).size(), (size_t)0);
    CHECK_EQ(Dante::Checkpoints("BEGIN STRINGS\n00000000 \"void checkpoint_X()\"\n", 44).size(), (size_t)0);
    CHECK_EQ(Dante::Checkpoints("", 0).size(), (size_t)0);
    CHECK_EQ(Dante::Checkpoints(nullptr, 0).size(), (size_t)0);

    return check::Done("dante");
}
