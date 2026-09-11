// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The level lists under the Mods page: Career from the engine's table, Custom from every mounted archive, which is
// how a mod's loose .lvl appears with no rebuild. Choosing one closes every page, then chains the level.

namespace LevelsMenu
{
    // Pushes the chooser. From inside a page's activate, or a claimed row's callback.
    void Open();
}
