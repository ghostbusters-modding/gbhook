// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Maps a DLL file as data, never as an image, and hands its bytes to the export walk in format/PeExport.h.

#include <cstddef>

namespace PeFile
{
    // Copies the data export `name` out of the file at `path`. `why` names the refusal; a missing file is one.
    bool ReadDataExport(const char* path, const char* name, void* out, size_t n, const char** why);
}
