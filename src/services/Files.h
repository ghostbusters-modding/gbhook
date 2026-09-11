// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Game assets through the engine's own enumerator and stream, so a mod sees what the game sees.
// Engine main thread only: nothing in CPod or the stream family locks. docs/engine/RE_FILE_API.md.

#include <string>
#include <vector>

namespace Files
{
    // Everything under `dir` matching `pattern`, ("world", "*.lvl"): bare names, deduplicated, sorted.
    // Every mounted archive is walked; loose files are not. Returns the count, or a negative GbhStatus.
    int List(const char* dir, const char* pattern, std::vector<std::string>& out);

    // A whole asset by its engine-relative path, "world\\haunt1.dante", through the game's own resolution
    // chain, decompressed. Returns the byte count, or a negative GbhStatus.
    int Read(const char* path, std::vector<unsigned char>& out);

    // `files <dir> <pattern>` and `filesize <path>`, game thread.
    void RegisterCommands();
}
