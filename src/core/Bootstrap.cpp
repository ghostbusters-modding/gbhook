// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The startup sequence, and the one place its order is decided. docs/ARCHITECTURE.md records why.

#include "Framework.h"
#include "FaultLogger.h"
#include "HookBroker.h"

extern "C" DWORD WINAPI GbHookMain(LPVOID)
{
    gameBase = reinterpret_cast<char*>(GetModuleHandleW(nullptr));

    Log::Init();
    Log::Writef("BOOT", "attached to ghost.exe at %p", gameBase);

    if (!HookBroker::Init())
    {
        Log::Write("BOOT", "ABORT -- MinHook would not initialise, so no hook can be installed. "
                           "gbhook is inert this run.");
        return 1;
    }

    // Armed before anything can fault, so a crash inside ghost.exe is reported as a ghost-relative address.
    FaultLogger::Install();

    Settings::Load();

    HookBroker::VerifyAll("boot");
    Log::Write("BOOT", "boot complete");
    return 0;
}
