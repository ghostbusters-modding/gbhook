// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "LevelsPage.h"

#include <algorithm>
#include <cctype>

namespace
{
    std::string Lower(std::string s)
    {
        for (char& c : s) c = (char)tolower((unsigned char)c);
        return s;
    }

    void Add(LevelsPage::Page& p, const std::string& stem)
    {
        p.rows.push_back({ Rows::Fit(stem), (int)p.levels.size() });
        p.levels.push_back(stem);
    }
}

namespace LevelsPage
{
    std::string Stem(const std::string& name)
    {
        if (name.size() > 4 && Lower(name.substr(name.size() - 4)) == ".lvl") return name.substr(0, name.size() - 4);
        return name;
    }

    Page Career(const std::vector<std::string>& career)
    {
        Page p;
        for (const std::string& c : career) Add(p, Stem(c));
        if (p.rows.empty()) p.rows.push_back({ "(the career table could not be read)", Rows::kInert });
        return p;
    }

    std::vector<Rows::Row> Checkpoints(const std::vector<std::string>& registered)
    {
        std::vector<Rows::Row> rows;
        rows.push_back({ "Level Start", kStart });
        for (size_t i = 0; i < registered.size(); ++i)
        {
            const std::string& r = registered[i];
            const std::string shown = r.compare(0, 11, "checkpoint_") == 0 ? r.substr(11) : r;
            rows.push_back({ Rows::Fit(shown), (int)i + 1 });
        }
        return rows;
    }

    Page Custom(const std::vector<std::string>& career, const std::vector<std::string>& found)
    {
        std::vector<std::string> careerLower;
        for (const std::string& c : career) careerLower.push_back(Lower(Stem(c)));

        std::vector<std::string> custom;
        for (const std::string& f : found)
        {
            const std::string s = Stem(f), l = Lower(s);
            if (std::find(careerLower.begin(), careerLower.end(), l) != careerLower.end()) continue;
            bool dup = false;
            for (const std::string& c : custom) if (Lower(c) == l) { dup = true; break; }
            if (!dup) custom.push_back(s);
        }
        std::sort(custom.begin(), custom.end(), [](const std::string& a, const std::string& b) { return Lower(a) < Lower(b); });

        Page p;
        for (const std::string& s : custom) Add(p, s);
        if (p.rows.empty()) p.rows.push_back({ "(none: a mod's world\\*.lvl lands here)", Rows::kInert });
        return p;
    }
}
