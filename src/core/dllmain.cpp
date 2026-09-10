// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// DllMain starts a thread and returns. We map during process init, so the loader lock is held and little else is legal.

#include <windows.h>

extern "C" DWORD WINAPI GbHookMain(LPVOID);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE h = CreateThread(nullptr, 0, GbHookMain, hModule, 0, nullptr);
        if (h) CloseHandle(h);
        break;
    }
    case DLL_PROCESS_DETACH:
        // Nothing to unwind: hooks and mods live for the process, by design.
        break;
    }
    return TRUE;
}
