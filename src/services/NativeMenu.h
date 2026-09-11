// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The game's own main-menu rows, brokered: a claim un-hides a row and routes its activation, and a claimed row
// may push real pages built on the engine's CLevelMenu, nested like the game's own. docs/engine/RE_NATIVE_MENUS.md.

#include "gbhook/gbhook.h"

namespace NativeMenu
{
    // The three CMainMenu detours, armed together. Claims recorded earlier become live here.
    void Install();
    bool Active();

    // Rows are slots on screen: Career, Online (GBH_ROW_ONLINE), Mods, a free slot (GBH_ROW_FREE), Options, Extras,
    // Exit. The game's own slots are refused; the second claimant of a slot is refused by name.
    int  ClaimRow(const char* owner, int row, GbhRowFn fn, void* user);
    int  SetRowLabel(const char* owner, int row, const char* label);   // the claimant only

    // Push a page onto the main menu from a claimed row's callback, or onto the page whose activate is running.
    int  OpenPage(const GbhNativeMenuDesc* desc);
    int  AddRow(const char* label, int action);   // from inside build() only
    void Refresh();                               // rebuild and re-label the top page; cheap when unchanged
    bool PageOpen();

    // Runs once on the main thread after the engine has destroyed the last open page: a level load is safe then.
    void AfterClose(void (*fn)(void*), void* user);

    void DropOwner(const char* owner);
}
