// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Rows.h"

namespace Rows
{
    std::string Fit(const std::string& s)
    {
        if (s.size() <= kLabelMax) return s;
        return s.substr(0, kLabelMax - 1) + "~";
    }

    std::vector<std::string> Wrap(const std::string& text, size_t width)
    {
        std::vector<std::string> out;
        if (width == 0) return out;
        std::string line, word;
        auto flushWord = [&]()
        {
            if (word.empty()) return;
            while (word.size() > width)
            {
                if (!line.empty()) { out.push_back(line); line.clear(); }
                out.push_back(word.substr(0, width));
                word.erase(0, width);
            }
            if (!line.empty() && line.size() + 1 + word.size() > width) { out.push_back(line); line.clear(); }
            if (!line.empty()) line += ' ';
            line += word;
            word.clear();
        };
        for (char c : text)
        {
            if (c == ' ' || c == '\t' || c == '\n') flushWord();
            else word += c;
        }
        flushWord();
        if (!line.empty()) out.push_back(line);
        return out;
    }
}
