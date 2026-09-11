// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "LevelsMenu.h"
#include "Files.h"
#include "LevelFlow.h"
#include "NativeMenu.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "format/Dante.h"
#include "menu/LevelsPage.h"

#include <windows.h>
#include <string>
#include <vector>

namespace
{
    constexpr uintptr_t kTableRva  = 0x7BD740;   // 20 x { int chapter, int index, const char* file }, firehouse.lvl on
    constexpr int       kTableRows = 20;
    constexpr int       kNameCap   = 64;
    enum { kCareer = 1, kCustom = 2 };

    struct LevelEntry { int32_t chapter, index; const char* file; };

    LevelsPage::Page         g_career, g_custom;
    std::vector<std::string> g_checkpoints;   // of the level whose page is open
    char                     g_chain[kNameCap] = { 0 };
    char                     g_chainCp[kNameCap] = { 0 };

    // Plain-C frame: the table is read-only data in the exe, guarded all the same.
    int CopyTable(char (*names)[kNameCap])
    {
        GBH_SEH_TRY
        {
            const LevelEntry* t = reinterpret_cast<const LevelEntry*>(gameBase + kTableRva);
            int n = 0;
            for (int i = 0; i < kTableRows; ++i)
            {
                if (!t[i].file) break;
                lstrcpynA(names[n++], t[i].file, kNameCap);
            }
            return n;
        }
        GBH_SEH_EXCEPT { return 0; }
    }

    // The checkpoint rides the same route as `level <stem> <checkpoint>`: parked until the level is live.
    void Chain(void*)
    {
        if (!LevelFlow::ChainToLevel(g_chain)) { Log::Writef("LEVEL", "'%s' could not be chained from the menu", g_chain); return; }
        if (!g_chainCp[0]) return;
        if (LevelFlow::FrontEndLoadPending()) LevelFlow::DeferCheckpoint(g_chainCp);
        else if (!LevelFlow::LoadCheckpoint(g_chainCp)) Log::Writef("LEVEL", "checkpoint '%s' could not be armed", g_chainCp);
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

    // Both lists share one page pair; `user` says which.
    void BuildList(void* user)
    {
        const LevelsPage::Page& p = *static_cast<const LevelsPage::Page*>(user);
        for (const Rows::Row& r : p.rows) NativeMenu::AddRow(r.label.c_str(), r.action);
    }

    // A level whose script registers checkpoints gets a page of them; one that registers none loads at once.
    int ActivateList(int action, void* user)
    {
        const LevelsPage::Page& p = *static_cast<const LevelsPage::Page*>(user);
        if (action < 0 || action >= (int)p.levels.size()) return GBH_NATIVE_STAY;
        const std::string& stem = p.levels[(size_t)action];

        std::vector<unsigned char> script;
        const int n = Files::Read(("world\\" + stem + ".dante").c_str(), script);
        g_checkpoints = n > 0 ? Dante::Checkpoints(reinterpret_cast<const char*>(script.data()), (size_t)n)
                              : std::vector<std::string>();
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
        char names[kTableRows][kNameCap];
        const int n = CopyTable(names);
        std::vector<std::string> career;
        for (int i = 0; i < n; ++i) career.push_back(names[i]);
        if (n == 0) Log::Write("LEVEL", "the career table could not be read");

        if (action == kCareer)
        {
            g_career = LevelsPage::Career(career);
            GbhNativeMenuDesc d = { sizeof d, BuildList, ActivateList, &g_career, "@CMainMenu_Career" };
            if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("LEVEL", "the career list could not be opened");
        }
        else if (action == kCustom)
        {
            std::vector<std::string> found;
            Files::List("world", "*.lvl", found);
            g_custom = LevelsPage::Custom(career, found);
            GbhNativeMenuDesc d = { sizeof d, BuildList, ActivateList, &g_custom, "Custom" };
            if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("LEVEL", "the custom list could not be opened");
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
