// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// gbhook's own page on main-menu row 2: Load Level, and View Mods with every mod's state and reason. Read-only.

namespace ModsMenu
{
    void Install();   // claims the row; needs NativeMenu::Install to have run or to run later
}
