// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// A page on the game's own main menu: the Online slot relabelled, rows that log, relabel, nest and close.

#include <windows.h>
#include <cstdio>

#include "gbhook/gbhook.h"
#include "gbhook/gbhook.hpp"

GBHOOK_PLUGIN("menudemo");

namespace
{
    enum { kLog = 1, kCount = 2, kClose = 3, kNested = 4 };
    int g_count = 0;

    // The nested page: one close pops it, the other pops every page back to the main menu.
    void NestedBuild(void*)
    {
        gbh::native_submenu_add_row("A nested page", GBH_NATIVE_INERT);
        gbh::native_submenu_add_row("Close this page", 1);
        gbh::native_submenu_add_row("Close every page", 2);
    }

    int NestedActivate(int action, void*)
    {
        return action == 2 ? GBH_NATIVE_CLOSE_ALL : GBH_NATIVE_CLOSE;
    }

    // Called on open and after every activation; a label that changed is relabelled in place.
    void PageBuild(void*)
    {
        char c[GBH_NATIVE_LABEL_CAP];
        snprintf(c, sizeof c, "Count: %d", g_count);
        gbh::native_submenu_add_row("Log a line", kLog);
        gbh::native_submenu_add_row(c, kCount);
        gbh::native_submenu_add_row("A header row", GBH_NATIVE_INERT);
        gbh::native_submenu_add_row("Open a nested page", kNested);
        gbh::native_submenu_add_row("Close", kClose);
    }

    int PageActivate(int action, void*)
    {
        switch (action)
        {
        case kLog:    gbh::log("MENU", "row 'Log a line' activated"); return GBH_NATIVE_STAY;
        case kCount:  ++g_count;                                     return GBH_NATIVE_STAY;
        case kNested:
        {
            GbhNativeMenuDesc d = { sizeof d, NestedBuild, NestedActivate, nullptr, "Nested" };
            if (gbh::native_submenu_open(d) != GBH_OK) gbh::log("MENU", "the nested page could not be opened");
            return GBH_NATIVE_STAY;
        }
        default:      return GBH_NATIVE_CLOSE;
        }
    }

    void OnRow(int, void*)
    {
        GbhNativeMenuDesc d = { sizeof d, PageBuild, PageActivate, nullptr, "Menu Demo" };
        if (gbh::native_submenu_open(d) != GBH_OK) gbh::log("MENU", "the page could not be opened");
    }
}

extern "C" GBHOOK_EXPORT int GbhPluginInit(const GbhApi* api)
{
    if (!api || api->abi_version != GBHOOK_ABI_VERSION) return GBH_ERR;
    gbh::bind(api);

    // The claim is recorded now and goes live when the menu broker installs, whatever the stage.
    if (gbh::native_row_claim(GBH_ROW_ONLINE, OnRow) != GBH_OK)
    {
        gbh::log("MENU", "the Online slot is taken; no page this run");
        return GBH_OK;
    }
    gbh::native_row_label(GBH_ROW_ONLINE, "Menu Demo");
    return GBH_OK;
}
