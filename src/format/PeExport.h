// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Reads a named data export out of a DLL's bytes without executing it. tests/format/test_pe.cpp is the spec.
// Every dereference is bounds-checked: the input is whatever someone dropped in mods/.

#include <cstddef>
#include <cstdint>

namespace PeExport
{
    // Copies `n` bytes of the export `name` from the file image `file[0, size)` into `out`.
    // True only for a well-formed 64-bit PE DLL whose export exists and fits. `why` names the refusal.
    bool ReadDataExport(const uint8_t* file, size_t size, const char* name, void* out, size_t n, const char** why);
}
