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

    using ModsPage::Mount;
    using ModsPage::Origin;
    using ModsPage::Switch;
    auto M = [](const char* id, const char* version, State st, const char* note, int assets, int code,
                Origin o, Mount mt, Switch sw = Switch::TurnOff)
    {
        Mod m;
        m.id = id; m.version = version; m.folder = id; m.state = st; m.note = note;
        m.assetFiles = assets; m.codeFiles = code; m.origin = o; m.mount = mt; m.toggle = sw;
        return m;
    };
    std::vector<Mod> mods = {
        M("gb.qol", "0.1.0", State::Loaded, "", 0, 1, Origin::None, Mount::None),
        M("duelarena", "", State::NoCode, "", 20, 0, Origin::Built, Mount::Mounted),
        M("gb.old", "2.0", State::Refused, "plugin 'Old.dll': built for ABI 2, this gbhook speaks ABI 1 -- rebuild the mod", 0, 1, Origin::None, Mount::None),
        M("gb.off", "", State::OffIni, "", 7, 1, Origin::Cached, Mount::Mounted, Switch::TurnOn),
        M("gb.averyveryveryverylongidentifiername", "1.2.3", State::Failed, "GbhPluginInit returned -1", 0, 1, Origin::None, Mount::None),
        M("gb.gbmm", "", State::NoCode, "", 3, 0, Origin::None, Mount::InChain),
        M("plain", "", State::OffIni, "", 1, 0, Origin::Cached, Mount::NotYet, Switch::TurnOn),
        M("gb.late", "", State::Pending, "", 0, 1, Origin::None, Mount::None),
    };
    h.missingRoots = { "D:\\nope" };

    // The list is ON or OFF and nothing else, switched or not: word padded to four, id, version when it fits.
    {
        std::vector<Mod> sw = mods;
        sw[0].changed = true;
        sw[6].changed = true;
        for (const std::vector<Mod>* set : { &mods, &sw })
        {
            auto rows = ModsPage::List(h, *set);
            CHECK_EQ(rows.size(), mods.size() + 1);
            CHECK_EQ(rows[0].label, "ON  gb.qol 0.1.0");
            CHECK_EQ(rows[0].action, 0);
            CHECK_EQ(rows[1].label, "ON  duelarena");
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
    }

    // The content row: counts with their plurals, then how the cache came to be and what became of it.
    {
        CHECK_EQ(ModsPage::ContentLines(mods[0])[0], "0 asset files, 1 code file");
        CHECK_EQ(ModsPage::ContentLines(mods[0]).size(), (size_t)1);
        Mod m = mods[6];
        auto l = ModsPage::ContentLines(m);
        CHECK_EQ(l[0], "1 asset file, 0 code files");
        CHECK_EQ(l[1], "cached, not mounted yet");
        m.mount = Mount::Mounted; m.codeFiles = 2;
        l = ModsPage::ContentLines(m);
        CHECK_EQ(l[0], "1 asset file, 2 code files");
        CHECK_EQ(l[1], "cached, mounted");
        m.origin = Origin::None; m.mount = Mount::None;
        CHECK_EQ(ModsPage::ContentLines(m).size(), (size_t)1);
    }
    {
        // The counts, then the state on the next row.
        auto l = ModsPage::ContentLines(mods[3]);
        CHECK_EQ(l.size(), (size_t)2);
        CHECK_EQ(l[0], "7 asset files, 1 code file");
        CHECK_EQ(l[1], "cached, mounted");
        l = ModsPage::ContentLines(mods[5]);
        CHECK_EQ(l[0], "3 asset files, 0 code files");
        CHECK_EQ(l[1], "left to the Mod Manager");
        Mod m = mods[3];
        m.origin = Origin::Built; m.mount = Mount::NotYet;
        CHECK_EQ(ModsPage::ContentLines(m)[1], "built, not mounted yet");
        m.mount = Mount::MountFailed;
        CHECK_EQ(ModsPage::ContentLines(m)[1], "built, mount failed");
        m.origin = Origin::None; m.mount = Mount::BuildFailed;
        CHECK_EQ(ModsPage::ContentLines(m)[1], "build failed");
        // The longest a row can get: every label fits.
        m.assetFiles = 99999; m.codeFiles = 99; m.origin = Origin::Built; m.mount = Mount::NotYet;
        l = ModsPage::ContentLines(m);
        CHECK_EQ(l[0], "99999 asset files, 99 code files");
        for (const std::string& x : l) CHECK(x.size() <= Rows::kLabelMax);
        m.origin = Origin::None; m.mount = Mount::InChain;
        for (const std::string& x : ModsPage::ContentLines(m)) CHECK(x.size() <= Rows::kLabelMax);
    }

    // Detail: ON or OFF, the reason when there is one, content, the switch, Back.
    {
        auto rows = ModsPage::Detail(mods[2]);
        CHECK_EQ(rows[0].label, "OFF");
        CHECK_EQ(rows[1].label, "plugin 'Old.dll': built for ABI 2, this");
        CHECK_EQ(rows[2].label, "gbhook speaks ABI 1 -- rebuild the mod");
        CHECK_EQ(rows[3].label, "0 asset files, 1 code file");
        CHECK_EQ(rows[4].label, "Disable");
        CHECK_EQ(rows.back().label, "Back");
        CHECK_EQ(rows.back().action, ModsPage::kBack);
    }
    {
        auto rows = ModsPage::Detail(mods[1]);
        CHECK_EQ(rows.size(), (size_t)5);
        CHECK_EQ(rows[0].label, "ON");
        CHECK_EQ(rows[1].label, "20 asset files, 0 code files");
        CHECK_EQ(rows[2].label, "built, mounted");
        CHECK_EQ(rows[3].label, "Disable");
        CHECK_EQ(rows[3].action, ModsPage::kToggle);
        for (int i = 0; i < 3; ++i) CHECK_EQ(rows[(size_t)i].action, Rows::kInert);
    }
    CHECK_EQ(ModsPage::Detail(mods[4])[0].label, "OFF");
    CHECK_EQ(ModsPage::Detail(mods[4])[1].label, "GbhPluginInit returned -1");
    CHECK_EQ(ModsPage::Detail(mods[7])[0].label, "ON");

    // The switch: Enable or Disable while the disk agrees with this boot, then an inert restart row either way.
    {
        auto rows = ModsPage::Detail(mods[3]);
        CHECK_EQ(rows.size(), (size_t)5);
        CHECK_EQ(rows[0].label, "OFF");
        CHECK_EQ(rows[3].label, "Enable");
        CHECK_EQ(rows[3].action, ModsPage::kToggle);

        // Pressed: the same row relabels, the count stays, and a second press has nothing to hit.
        Mod m = mods[3]; m.toggle = Switch::TurnOff; m.changed = true;
        auto after = ModsPage::Detail(m);
        CHECK_EQ(after.size(), rows.size());
        CHECK_EQ(after[0].label, "OFF");
        CHECK_EQ(after[3].label, "Restart to Apply Changes");
        CHECK_EQ(after[3].action, Rows::kInert);
        for (const Rows::Row& r : after) CHECK(r.action != ModsPage::kToggle);

        m = mods[0]; m.toggle = Switch::TurnOn; m.changed = true;
        after = ModsPage::Detail(m);
        CHECK_EQ(after[0].label, "ON");
        CHECK_EQ(after[2].label, "Restart to Apply Changes");
        CHECK_EQ(after.size(), ModsPage::Detail(mods[0]).size());

        m.saveFailed = true; m.changed = false; m.toggle = Switch::TurnOff;
        after = ModsPage::Detail(m);
        CHECK_EQ(after[2].label, "Disable");
        CHECK_EQ(after[3].label, "gbhook.ini could not be written");
        CHECK_EQ(after[3].action, Rows::kInert);
        CHECK_EQ(after[4].label, "Back");
        for (const Rows::Row& r : after) CHECK(r.label.size() <= Rows::kLabelMax);

        m.toggle = Switch::None; m.saveFailed = false;
        CHECK_EQ(ModsPage::Detail(m).size(), (size_t)3);
    }

    return check::Done("modspage");
}
