// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModsPage.h"

namespace ModsPage
{
    bool IsOn(State s) { return s == State::Loaded || s == State::NoCode || s == State::Pending; }

    const char* StateWord(State s) { return IsOn(s) ? "ON" : "OFF"; }

    static std::string Headline(const Mod& m)
    {
        switch (m.state)
        {
        case State::Loaded:     return "ON: loaded at " + m.stage;
        case State::NoCode:     return m.content.empty() ? "ON: nothing to load" : "ON: content only";
        case State::Pending:    return "ON: loads at " + m.stage;
        case State::OffIni:     return "OFF: disabled in gbhook.ini";
        case State::Refused:    return "OFF: refused";
        case State::Failed:     return "OFF: error";
        }
        return "?";
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

        rows.push_back({ Rows::Fit(Headline(m)), Rows::kInert });

        for (const std::string& l : Rows::Wrap(m.note, Rows::kLabelMax)) rows.push_back({ l, Rows::kInert });
        if (!m.content.empty())
            for (const std::string& l : Rows::Wrap("Content: " + m.content, Rows::kLabelMax)) rows.push_back({ l, Rows::kInert });

        if (m.toggle != Switch::None)
        {
            rows.push_back({ m.toggle == Switch::TurnOff ? "Disable" : "Enable", kToggle });
            if (m.saveFailed) rows.push_back({ "gbhook.ini could not be written", Rows::kInert });
            else if (m.changed) rows.push_back({ Rows::Fit(std::string("Saved: ") + (m.toggle == Switch::TurnOn ? "OFF" : "ON") + " after a restart"), Rows::kInert });
        }

        rows.push_back({ "Back", kBack });
        return rows;
    }
}
