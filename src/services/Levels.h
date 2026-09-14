// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The level lists behind the Mods page and the ABI: the engine's career table, the archives' other .lvl files, and
// a level's registered checkpoints. One implementation; LevelsMenu draws it, Api hands it out.

#include <string>
#include <vector>

namespace Levels
{
    // The engine's own table, stems in its order. Any thread; the count, or a negative GbhStatus.
    int Career(std::vector<std::string>& out);

    // Every world\*.lvl a mounted archive holds that the table does not, sorted, stems. Engine main thread only.
    int Custom(std::vector<std::string>& out);

    // What world\<stem>.dante registers, in script order. Engine main thread only. Empty when unreadable.
    int Checkpoints(const char* stem, std::vector<std::string>& out);
}
