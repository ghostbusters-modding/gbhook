// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Loads each accepted mod's DLL at its stage and calls GbhPluginInit under a guard. Nothing is ever unloaded.
// Caller identity is derived from a return address here, so no ABI entry ever takes a mod handle.

#include "gbhook/gbhook.h"

#include <string>

namespace Host
{
    enum class State { Pending, Loaded, NoCode, Off, Failed };   // Off: disabled in mod.ini, listed and never loaded

    struct Status
    {
        std::string id;
        State       state = State::Pending;
        std::string note;    // Failed: why. Kept for the Mods page.
    };

    // LoadLibrary and GbhPluginInit for every accepted mod at `stage`, in code order. Returns how many came up.
    int  Init(GbhStage stage);

    // Mark a loaded mod inert after a fault or a refused call. Its code stays mapped.
    void Disable(const char* id, const char* why);

    // The mod owning the module that contains `addr`, or nullptr for gbhook itself. Its gbhook/ folder likewise.
    const char* OwnerOfAddress(const void* addr);
    const char* DirOfAddress(const void* addr);

    // Every mod discovery saw, accepted ones first in code order, then the refused.
    int           Count();
    const char*   IdAt(int i);
    const Status* StatusOf(const char* id);
    bool          IsLoaded(const char* id);   // Loaded, or accepted with no code

    void LogStatus();
}
