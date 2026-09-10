// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Internals shared by gbhook's own translation units. Nothing here crosses the DLL boundary.

#include <windows.h>
#include <cstdint>

#include "gbhook/gbhook.h"

// ghost.exe module base. Published by Bootstrap before any service starts, constant afterwards.
extern char* gameBase;

namespace Framework
{
    // (major << 16) | (minor << 8) | patch
    constexpr uint32_t    kVersion       = (0u << 16) | (1u << 8) | 0u;
    constexpr const char* kVersionString = "0.1.0";

    // Directory holding ghost.exe, no trailing separator.
    const char* GameDir();

    // True once Bootstrap has finished `stage`. A set of bits, so stage order is not load-bearing.
    bool StageReached(GbhStage stage);
    void NoteStage(GbhStage stage);

    // The framework heap. Every module has a static CRT and its own heap, so anything crossing the ABI comes from here.
    void* Alloc(size_t n);
    void* Realloc(void* p, size_t n);
    void  Free(void* p);
}

namespace Log
{
    // <gamedir>\gbhook.log. Line grammar in format/LogLine.h; safe from any thread.
    void Init();
    void Write(const char* tag, const char* line);
    void Writef(const char* tag, const char* fmt, ...);

    // Same, attributed to a mod id. `id` may be null for the framework.
    void WriteFrom(const char* id, const char* tag, const char* line);
    void WritefFrom(const char* id, const char* tag, const char* fmt, ...);
}

namespace Settings
{
    // <gamedir>\gbhook.ini. Loaded once; returned pointers stay valid for the life of the process.
    void        Load();
    const char* Get(const char* key, const char* dflt);
    int         GetInt(const char* key, int dflt);
    float       GetFloat(const char* key, float dflt);
    bool        GetBool(const char* key, bool dflt);
}
