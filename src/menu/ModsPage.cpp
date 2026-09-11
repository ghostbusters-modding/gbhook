// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModsPage.h"

namespace ModsPage
{
    const char* StateWord(State s)
    {
        switch (s)
        {
        case State::On:      return "ON";
        case State::Off:     return "OFF";
        case State::Refused: return "REFUSED";
        case State::Failed:  return "FAILED";
        }
        return "?";
    }

    std::vector<Rows::Row> List(const Header& h, const std::vector<Mod>& mods)
    {
        std::vector<Rows::Row> rows;
        rows.push_back({ Rows::Fit("gbhook " + h.version + " for ghost " + h.targetMd5.substr(0, 8)), Rows::kInert });

        if (mods.empty())
        {
            rows.push_back({ "No mods installed", Rows::kInert });
            for (const std::string& r : h.roots) rows.push_back({ Rows::Fit("Looked in " + r), Rows::kInert });
        }

        for (size_t i = 0; i < mods.size(); ++i)
        {
            const Mod& m = mods[i];
            std::string word = StateWord(m.state);
            while (word.size() < 8) word += ' ';
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
        rows.push_back({ Rows::Fit(m.id + (m.version.empty() ? "" : " " + m.version)), Rows::kInert });
        rows.push_back({ Rows::Fit("Folder " + m.folder), Rows::kInert });

        std::string state = StateWord(m.state);
        if (m.state == State::On && !m.stage.empty()) state += ", loads at " + m.stage;
        rows.push_back({ Rows::Fit(state), Rows::kInert });

        for (const std::string& l : Rows::Wrap(m.note, Rows::kLabelMax)) rows.push_back({ l, Rows::kInert });
        if (!m.content.empty())
            for (const std::string& l : Rows::Wrap("Content: " + m.content, Rows::kLabelMax)) rows.push_back({ l, Rows::kInert });

        rows.push_back({ "Back", kBack });
        return rows;
    }
}
