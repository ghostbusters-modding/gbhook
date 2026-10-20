// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "LevelsMenu.h"
#include "LevelFlow.h"
#include "Levels.h"
#include "NativeMenu.h"
#include "../core/Framework.h"
#include "menu/LevelsPage.h"

#include <windows.h>
#include <string>
#include <vector>

namespace
{
    constexpr int kNameCap = 64;
    enum { kCareer = 1, kCustom = 2 };

    LevelsPage::Page         g_career, g_custom;
    struct View { const LevelsPage::Page* page; size_t first; const char* title; };
    View                     g_views[16];   // one per screen of the open list
    std::vector<std::string> g_checkpoints;   // of the level whose page is open
    char                     g_chain[kNameCap] = { 0 };
    char                     g_chainCp[kNameCap] = { 0 };

    void Chain(void*)
    {
        if (!LevelFlow::ChainToLevel(g_chain)) { Log::Writef("LEVEL", "'%s' could not be chained from the menu", g_chain); return; }
        if (g_chainCp[0]) LevelFlow::DeferCheckpoint(g_chainCp);
    }

    int Choose(const char* level, const char* checkpoint)
    {
        lstrcpynA(g_chain, level, kNameCap);
        lstrcpynA(g_chainCp, checkpoint ? checkpoint : "", kNameCap);
        Log::Writef("LEVEL", "'%s'%s%s chosen from the menu; it loads once every page has closed", g_chain,
                    g_chainCp[0] ? " at " : "", g_chainCp);
        NativeMenu::AfterClose(Chain, nullptr);
        return GBH_NATIVE_CLOSE_ALL;
    }

    void BuildCheckpoints(void*)
    {
        for (const Rows::Row& r : LevelsPage::Checkpoints(g_checkpoints)) NativeMenu::AddRow(r.label.c_str(), r.action);
    }

    int ActivateCheckpoints(int action, void*)
    {
        if (action == LevelsPage::kStart) return Choose(g_chain, nullptr);
        if (action < 1 || action > (int)g_checkpoints.size()) return GBH_NATIVE_STAY;
        return Choose(g_chain, g_checkpoints[(size_t)action - 1].c_str());
    }

    // Both lists share one page pair; `user` is the View: which list, from which row.
    void BuildList(void* user)
    {
        const View& v = *static_cast<const View*>(user);
        size_t next = 0;
        for (const Rows::Row& r : LevelsPage::Slice(*v.page, v.first, GBH_NATIVE_MAX_ROWS, &next))
            NativeMenu::AddRow(r.label.c_str(), r.action);
    }

    int ActivateList(int action, void* user);

    int OpenList(size_t view, const LevelsPage::Page* page, size_t first, const char* title)
    {
        if (view >= sizeof g_views / sizeof g_views[0]) { Log::Write("LEVEL", "the level list is too long to page further"); return GBH_NATIVE_STAY; }
        g_views[view] = { page, first, title };
        GbhNativeMenuDesc d = { sizeof d, BuildList, ActivateList, &g_views[view], title };
        if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Writef("LEVEL", "the level list could not be opened at row %zu", first);
        return GBH_NATIVE_STAY;
    }

    // A level whose script registers checkpoints gets a page of them; one that registers none loads at once.
    int ActivateList(int action, void* user)
    {
        const View& v = *static_cast<const View*>(user);
        const LevelsPage::Page& p = *v.page;
        if (action == LevelsPage::kMore)
        {
            size_t next = 0;
            LevelsPage::Slice(p, v.first, GBH_NATIVE_MAX_ROWS, &next);
            return next ? OpenList((size_t)(&v - g_views) + 1, &p, next, v.title) : GBH_NATIVE_STAY;
        }
        if (action < 0 || action >= (int)p.levels.size()) return GBH_NATIVE_STAY;
        const std::string& stem = p.levels[(size_t)action];

        const int n = Levels::Checkpoints(stem.c_str(), g_checkpoints);
        if (n <= 0) Log::Writef("LEVEL", "'%s' has no readable script; loading it from the start", stem.c_str());
        if (g_checkpoints.empty()) return Choose(stem.c_str(), nullptr);

        lstrcpynA(g_chain, stem.c_str(), kNameCap);
        GbhNativeMenuDesc d = { sizeof d, BuildCheckpoints, ActivateCheckpoints, nullptr, g_chain };
        if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("LEVEL", "the checkpoint page could not be opened");
        return GBH_NATIVE_STAY;
    }

    void BuildChooser(void*)
    {
        NativeMenu::AddRow("Career", kCareer);
        NativeMenu::AddRow("Custom", kCustom);
    }

    int ActivateChooser(int action, void*)
    {
        if (action == kCareer)
        {
            std::vector<std::string> career;
            Levels::Career(career);   // logs "could not be read" itself when the table is bad
            g_career = LevelsPage::Career(career);
            OpenList(0, &g_career, 0, "@CMainMenu_Career");
        }
        else if (action == kCustom)
        {
            std::vector<std::string> custom;
            Levels::Custom(custom);   // already filtered against the career table
            g_custom = LevelsPage::Custom({}, custom);
            OpenList(0, &g_custom, 0, "Custom");
        }
        return GBH_NATIVE_STAY;
    }
}

namespace LevelsMenu
{
    void Open()
    {
        GbhNativeMenuDesc d = { sizeof d, BuildChooser, ActivateChooser, nullptr, "@CLevelMenu_Title" };
        if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("LEVEL", "the level chooser could not be opened");
    }
}
