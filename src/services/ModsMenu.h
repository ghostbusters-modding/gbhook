// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// gbhook's own page on main-menu row 2: Load Level, View Mods with every mod's state and reason, then each mod's page.

#include "gbhook/gbhook.h"

namespace ModsMenu
{
    void Install();   // claims the row; needs NativeMenu::Install to have run or to run later

    // A mod's own row under View Mods, opening `desc` when chosen. The caller checks the mod is in its init.
    int  AddPage(const char* owner, const char* label, const GbhNativeMenuDesc* desc);
}
