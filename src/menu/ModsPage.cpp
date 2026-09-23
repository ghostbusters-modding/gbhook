// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModsPage.h"

namespace ModsPage
{
    bool IsOn(State s) { return s == State::Loaded || s == State::NoCode || s == State::Pending; }

    const char* StateWord(State s) { return IsOn(s) ? "ON" : "OFF"; }

    // The toggle names the button, so the next start is its opposite: Disable means it will start ON.
    static const char* NextWord(const Mod& m) { return m.toggle == Switch::TurnOff ? "ON" : "OFF"; }

    static std::string Headline(const Mod& m)
    {
        switch (m.state)
        {
        case State::Loaded:     return "Now: ON, loaded at " + m.stage;
        case State::NoCode:     return m.content.empty() ? "Now: ON, nothing to load" : "Now: ON, content only";
        case State::Pending:    return "Now: ON, loads at " + m.stage;
        case State::OffIni:     return "Now: OFF, disabled in gbhook.ini";
        case State::Refused:    return "Now: OFF, refused";
        case State::Failed:     return "Now: OFF, error";
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
            // Now>next once the switch was pressed; the list is the one page the player surely sees again.
            std::string word = StateWord(m.state);
            if (m.changed && m.toggle != Switch::None) word += std::string(">") + NextWord(m);
            while (word.size() < (m.changed ? 7u : 4u)) word += ' ';
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
            std::string next = std::string("Next start: ") + NextWord(m);
            if (m.changed) next += ", saved in gbhook.ini";
            rows.push_back({ Rows::Fit(next), Rows::kInert });
            rows.push_back({ m.toggle == Switch::TurnOff ? "Disable" : "Enable", kToggle });
            if (m.saveFailed) rows.push_back({ "gbhook.ini could not be written", Rows::kInert });
        }

        rows.push_back({ "Back", kBack });
        return rows;
    }
}
