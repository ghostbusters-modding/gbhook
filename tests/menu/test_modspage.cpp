// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for menu/Rows and menu/ModsPage: labels that fit, states as words, reasons wrapped onto rows.

#include "check.h"
#include "menu/ModsPage.h"

using ModsPage::Header;
using ModsPage::Mod;
using ModsPage::State;

int main()
{
    // Rows: fit and wrap.
    CHECK_EQ(Rows::Fit("short"), "short");
    CHECK_EQ(Rows::Fit(std::string(39, 'x')).size(), (size_t)39);
    CHECK_EQ(Rows::Fit(std::string(40, 'x')), std::string(38, 'x') + "~");
    {
        auto w = Rows::Wrap("requires 'gb.z', which is not present", 20);
        CHECK_EQ(w.size(), (size_t)2);
        CHECK_EQ(w[0], "requires 'gb.z',");
        CHECK_EQ(w[1], "which is not present");
        CHECK_EQ(Rows::Wrap("", 10).size(), (size_t)0);
        auto h = Rows::Wrap("abcdefghijkl end", 5);
        CHECK_EQ(h.size(), (size_t)4);
        CHECK_EQ(h[0], "abcde");
        CHECK_EQ(h[2], "kl");
        CHECK_EQ(h[3], "end");
        CHECK_EQ(Rows::Wrap("a b", 0).size(), (size_t)0);
    }

    Header h;
    h.version   = "0.1.0";
    h.targetMd5 = "0b89556c07e5b737efe444351227e747";
    h.roots     = { "X:\\game\\mods" };

    // No mods: the page says so and names the folder it looked in.
    {
        auto rows = ModsPage::List(h, {});
        CHECK_EQ(rows.size(), (size_t)2);
        CHECK_EQ(rows[0].label, "No mods installed");
        CHECK_EQ(rows[0].action, Rows::kInert);
        CHECK_EQ(rows[1].label, "Looked in X:\\game\\mods");
    }

    std::vector<Mod> mods = {
        { "gb.qol", "0.1.0", "QoL", "preboot", State::Loaded, "", "" },
        { "gb.duelarena", "", "DuelArena", "boot", State::NoCode, "", "20 files built and mounted" },
        { "gb.old", "2.0", "Old", "", State::Refused, "plugin 'Old.dll': built for ABI 2, this gbhook speaks ABI 1 -- rebuild the mod", "" },
        { "gb.off", "", "Off", "", State::OffIni, "", "" },
        { "gb.averyveryveryverylongidentifiername", "1.2.3", "Long", "boot", State::Failed, "GbhPluginInit returned -1", "" },
        { "gb.gbmm", "", "Gbmm", "boot", State::NoCode, "", "found in the chained PODs, the Mod Manager loads it" },
        { "plain", "", "Plain", "boot", State::OffIni, "", "", ModsPage::Switch::TurnOn },
        { "gb.late", "", "Late", "ready", State::Pending, "", "" },
    };
    h.missingRoots = { "D:\\nope" };

    // The list is ON or OFF and nothing else: word padded to four, id, version when it fits, action = index.
    {
        auto rows = ModsPage::List(h, mods);
        CHECK_EQ(rows.size(), mods.size() + 1);
        CHECK_EQ(rows[0].label, "ON  gb.qol 0.1.0");
        CHECK_EQ(rows[0].action, 0);
        CHECK_EQ(rows[1].label, "ON  gb.duelarena");
        CHECK_EQ(rows[2].label, "OFF gb.old 2.0");
        CHECK_EQ(rows[2].action, 2);
        CHECK_EQ(rows[3].label, "OFF gb.off");
        CHECK_EQ(rows[4].label, "OFF gb.averyveryveryverylongidentifier~");
        CHECK_EQ(rows[4].label.size(), (size_t)39);
        CHECK_EQ(rows[5].label, "ON  gb.gbmm");
        CHECK_EQ(rows[6].label, "OFF plain");
        CHECK_EQ(rows[7].label, "ON  gb.late");
        CHECK_EQ(rows[8].label, "Missing root D:\\nope");
        CHECK_EQ(rows[8].action, Rows::kInert);
    }

    // Detail: why, the reason wrapped, content, the switch, Back.
    {
        auto rows = ModsPage::Detail(mods[2]);
        CHECK_EQ(rows[0].label, "OFF: refused");
        CHECK_EQ(rows[1].label, "plugin 'Old.dll': built for ABI 2, this");
        CHECK_EQ(rows[2].label, "gbhook speaks ABI 1 -- rebuild the mod");
        CHECK_EQ(rows.back().label, "Back");
        CHECK_EQ(rows.back().action, ModsPage::kBack);
        for (size_t i = 0; i + 1 < rows.size(); ++i) CHECK_EQ(rows[i].action, Rows::kInert);
    }
    {
        auto rows = ModsPage::Detail(mods[1]);
        CHECK_EQ(rows[0].label, "ON: content only");
        CHECK_EQ(rows[1].label, "Content: 20 files built and mounted");
        CHECK_EQ(rows.size(), (size_t)3);
    }
    CHECK_EQ(ModsPage::Detail(mods[0])[0].label, "ON: loaded at preboot");
    CHECK_EQ(ModsPage::Detail(mods[3])[0].label, "OFF: disabled in gbhook.ini");
    CHECK_EQ(ModsPage::Detail(mods[4])[0].label, "OFF: error");
    CHECK_EQ(ModsPage::Detail(mods[4])[1].label, "GbhPluginInit returned -1");
    CHECK_EQ(ModsPage::Detail(mods[5])[1].label, "Content: found in the chained PODs, the");
    CHECK_EQ(ModsPage::Detail(mods[7])[0].label, "ON: loads at ready");
    {
        Mod bare = mods[0]; bare.state = State::NoCode;
        CHECK_EQ(ModsPage::Detail(bare)[0].label, "ON: nothing to load");
    }

    // The switch: Disable or Enable, and once pressed, what the next start does.
    {
        auto rows = ModsPage::Detail(mods[6]);
        CHECK_EQ(rows[0].label, "OFF: disabled in gbhook.ini");
        CHECK_EQ(rows[1].label, "Enable");
        CHECK_EQ(rows[1].action, ModsPage::kToggle);
        CHECK_EQ(rows.size(), (size_t)3);

        Mod m = mods[6]; m.toggle = ModsPage::Switch::TurnOff; m.changed = true;
        rows = ModsPage::Detail(m);
        CHECK_EQ(rows[1].label, "Disable");
        CHECK_EQ(rows[2].label, "Saved: ON after a restart");
        CHECK_EQ(rows[2].action, Rows::kInert);

        m = mods[0]; m.toggle = ModsPage::Switch::TurnOn; m.changed = true;
        rows = ModsPage::Detail(m);
        CHECK_EQ(rows[1].label, "Enable");
        CHECK_EQ(rows[2].label, "Saved: OFF after a restart");

        m.saveFailed = true;
        rows = ModsPage::Detail(m);
        CHECK_EQ(rows[2].label, "gbhook.ini could not be written");
    }

    return check::Done("modspage");
}
