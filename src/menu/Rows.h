// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// What every native page is made of: a label that fits the row and an action, or none. Pure.

#include <string>
#include <vector>

namespace Rows
{
    constexpr int    kInert    = -1;   // a header or a text line; activating it does nothing
    constexpr size_t kLabelMax = 39;   // GBH_NATIVE_LABEL_CAP less the terminator

    struct Row
    {
        std::string label;
        int         action = kInert;
    };

    // Cut to the row width, marking the cut with a trailing '~'.
    std::string Fit(const std::string& s);

    // Word-wrapped into rows of at most `width`, a word longer than the width cut hard.
    std::vector<std::string> Wrap(const std::string& text, size_t width);
}
