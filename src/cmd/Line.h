// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// One command line into its words. Pure; tests/cmd/test_line.cpp is the spec.

#include <string>
#include <vector>

namespace Line
{
    // Split on blanks, honouring "quoted words" so a name with a space survives. A lone '"' opens to the end.
    std::vector<std::string> Tokenize(const std::string& line);

    // Blank, or a '#' comment: the file channel skips it.
    bool IsSkippable(const std::string& line);

    // Trailing CR and LF removed, nothing else.
    std::string StripEol(std::string line);
}
