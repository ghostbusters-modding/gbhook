// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Line.h"

namespace Line
{
    std::vector<std::string> Tokenize(const std::string& line)
    {
        std::vector<std::string> out;
        std::string cur;
        bool inQuote = false, have = false;
        for (char c : line)
        {
            if (c == '"') { inQuote = !inQuote; have = true; continue; }
            if (!inQuote && (c == ' ' || c == '\t'))
            {
                if (have) { out.push_back(cur); cur.clear(); have = false; }
                continue;
            }
            cur += c;
            have = true;
        }
        if (have) out.push_back(cur);
        return out;
    }

    bool IsSkippable(const std::string& line)
    {
        for (char c : line)
        {
            if (c == ' ' || c == '\t') continue;
            return c == '#';
        }
        return true;
    }

    std::string StripEol(std::string line)
    {
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
        return line;
    }
}
