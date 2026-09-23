// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModsPage.h"

#include <string>

namespace ModsPage
{
    bool IsOn(State s) { return s == State::Loaded || s == State::NoCode || s == State::Pending; }

    const char* StateWord(State s) { return IsOn(s) ? "ON" : "OFF"; }

    static std::string Count(int n, const char* what)
    {
        return std::to_string(n) + " " + what + (n == 1 ? "" : "s");
    }

    std::vector<std::string> ContentLines(const Mod& m)
    {
        std::vector<std::string> out;
        out.push_back(Count(m.assetFiles, "asset file") + ", " + Count(m.codeFiles, "code file"));
        if (m.assetFiles == 0) return out;

        std::string state;
        if (m.origin == Origin::Cached)     state = "cached";
        else if (m.origin == Origin::Built) state = "built";
        const char* mount = nullptr;
        switch (m.mount)
        {
        case Mount::Mounted:     mount = "mounted"; break;
        case Mount::MountFailed: mount = "mount failed"; break;
        case Mount::BuildFailed: mount = "build failed"; break;
        case Mount::InChain:     mount = "left to the Mod Manager"; break;
        case Mount::NotYet:      mount = "not mounted yet"; break;
        case Mount::None:        break;
        }
        if (mount) state += (state.empty() ? "" : ", ") + std::string(mount);

        // The state gets its own row: counts and state together rarely fit a 39-character label.
        if (!state.empty()) out.push_back(state);
        return out;
    }

    std::vector<Rows::Row> List(const Header& h, const std::vector<Mod>& mods)
    {
        std::vector<Rows::Row> rows; 

        if (mods.empty())
        {
            rows.push_back({ "No mods installed", Rows::kInert });
            for (const std::string& r : h.roots) rows.push_back({ Rows::Fit("Looked in " + r), Rows::kInert });
        }

        for (size_t i = 0; i < mods.size(); ++i)
        {
            const Mod& m = mods[i];
            std::string word = StateWord(m.state);
            while (word.size() < 4) word += ' ';
            std::string label = word + m.id;
            if (!m.version.empty() && label.size() + 1 + m.version.size() <= Rows::kLabelMax) label += " " + m.version;
            rows.push_back({ Rows::Fit(label), (int)i });
        }

        for (const std::string& r : h.missingRoots) rows.push_back({ Rows::Fit("Missing root " + r), Rows::kInert });
        return rows;
    }

    std::vector<Rows::Row> Detail(const Mod& m)
    {
        std::vector<Rows::Row> rows; 

        rows.push_back({ StateWord(m.state), Rows::kInert });
        for (const std::string& l : Rows::Wrap(m.note, Rows::kLabelMax)) rows.push_back({ l, Rows::kInert });
        for (const std::string& l : ContentLines(m)) rows.push_back({ Rows::Fit(l), Rows::kInert });

        if (m.changed)                         rows.push_back({ "Restart to Apply Changes", Rows::kInert });
        else if (m.toggle == Switch::TurnOff)  rows.push_back({ "Disable", kToggle });
        else if (m.toggle == Switch::TurnOn)   rows.push_back({ "Enable", kToggle });
        if (m.saveFailed) rows.push_back({ "gbhook.ini could not be written", Rows::kInert });

        rows.push_back({ "Back", kBack });
        return rows;
    }
}
