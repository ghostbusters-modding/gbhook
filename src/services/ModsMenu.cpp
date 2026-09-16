// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModsMenu.h"
#include "LevelsMenu.h"
#include "NativeMenu.h"
#include "../core/Framework.h"
#include "../mod/ContentBuild.h"
#include "../mod/Discovery.h"
#include "../mod/Host.h"
#include "format/Ini.h"
#include "menu/ModsPage.h"

#include <string>
#include <vector>

namespace
{
    constexpr int kRow = 2;
    enum { kLoadLevel = 1, kViewMods = 2 };

    std::vector<ModsPage::Mod> g_mods;
    ModsPage::Header           g_header;
    int                        g_detail = -1;

    ModsPage::State StateOf(const ModSet::Record& r, const Host::Status* s)
    {
        if (r.disabled) return ModsPage::State::Off;
        if (!r.accepted) return ModsPage::State::Refused;
        if (s && s->state == Host::State::Failed) return ModsPage::State::Failed;
        return ModsPage::State::On;
    }

    void Gather()
    {
        g_mods.clear();
        for (const ModSet::Record& r : Mods::Result().records)
        {
            const Host::Status* s = r.mod.id.empty() ? nullptr : Host::StatusOf(r.mod.id.c_str());
            ModsPage::Mod m;
            m.id      = r.mod.id.empty() ? r.folder : r.mod.id;
            m.version = r.mod.version;
            m.folder  = r.folder;
            m.stage   = ModIni::StageName(r.mod.stage);
            m.state   = StateOf(r, s);
            m.note    = r.disabled ? "disabled in modinfo.ini" : (!r.accepted ? r.refusal : (s ? s->note : ""));
            m.content = r.mod.id.empty() ? "" : ContentBuild::Summary(r.mod.id.c_str());
            g_mods.push_back(m);
        }
        g_header.version      = Framework::kVersionString;
        g_header.targetMd5    = GBHOOK_TARGET_MD5;
        g_header.roots        = Ini::List(Settings::Get("mods.root", "mods"));
        g_header.missingRoots = Mods::MissingRoots();
    }

    // One mod in full; its Back row pops the page.
    void BuildDetail(void*)
    {
        if (g_detail < 0 || g_detail >= (int)g_mods.size()) return;
        for (const Rows::Row& r : ModsPage::Detail(g_mods[(size_t)g_detail])) NativeMenu::AddRow(r.label.c_str(), r.action);
    }
    int ActivateDetail(int action, void*)
    {
        return action == ModsPage::kBack ? GBH_NATIVE_CLOSE : GBH_NATIVE_STAY;
    }

    void BuildList(void*)
    {
        for (const Rows::Row& r : ModsPage::List(g_header, g_mods)) NativeMenu::AddRow(r.label.c_str(), r.action);
    }
    int ActivateList(int action, void*)
    {
        if (action < 0 || action >= (int)g_mods.size()) return GBH_NATIVE_STAY;
        g_detail = action;
        GbhNativeMenuDesc d = { sizeof d, BuildDetail, ActivateDetail, nullptr, g_mods[(size_t)action].id.c_str() };
        if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("MODS", "the mod's page could not be opened");
        return GBH_NATIVE_STAY;
    }

    void BuildRoot(void*)
    {
        NativeMenu::AddRow("Load Level", kLoadLevel);
        NativeMenu::AddRow("View Mods", kViewMods);
    }
    int ActivateRoot(int action, void*)
    {
        if (action == kLoadLevel) LevelsMenu::Open();
        else if (action == kViewMods)
        {
            Gather();
            GbhNativeMenuDesc d = { sizeof d, BuildList, ActivateList, nullptr, "Mods" };
            if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("MODS", "the mods list could not be opened");
        }
        return GBH_NATIVE_STAY;
    }

    void OnRow(int, void*)
    {
        GbhNativeMenuDesc d = { sizeof d, BuildRoot, ActivateRoot, nullptr, "Mods" };
        if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("MODS", "the Mods page could not be opened");
    }
}

namespace ModsMenu
{
    void Install()
    {
        if (NativeMenu::ClaimRow(nullptr, kRow, OnRow, nullptr) != GBH_OK) return;
        NativeMenu::SetRowLabel(nullptr, kRow, "Mods");
    }
}
