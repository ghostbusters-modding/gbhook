// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Game.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "gb/Globals.h"

#include <windows.h>

namespace
{
    // Plain-C frames for the guard: a torn pointer during a load reads as null, never as a crash.
    void* ReadPtr(uintptr_t rva)
    {
        if (!gameBase) return nullptr;
        GBH_SEH_TRY { return *reinterpret_cast<void**>(gameBase + rva); }
        GBH_SEH_EXCEPT { return nullptr; }
    }

    bool CopyStem(const char* src, char* out, size_t n)
    {
        GBH_SEH_TRY { lstrcpynA(out, src, (int)n); return true; }
        GBH_SEH_EXCEPT { out[0] = '\0'; return false; }
    }
}

namespace Game
{
    void* Singleton()   { return ReadPtr(Globals::gGame); }
    void* LocalPlayer() { return ReadPtr(Globals::localPlayer); }

    void LevelStem(char* out, size_t n)
    {
        if (!out || n == 0) return;
        out[0] = '\0';
        void* g = Singleton();
        if (!g) return;
        CopyStem(static_cast<const char*>(g) + Globals::cgameLevelStem, out, n);
    }
}
