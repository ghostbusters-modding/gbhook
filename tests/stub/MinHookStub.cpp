// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// MinHook stand-in for the mingw link check only. The shipping build links the real library.

#include <MinHook.h>

extern "C"
{
    MH_STATUS WINAPI MH_Initialize(VOID)                        { return MH_OK; }
    MH_STATUS WINAPI MH_Uninitialize(VOID)                      { return MH_OK; }
    MH_STATUS WINAPI MH_CreateHook(LPVOID, LPVOID, LPVOID*)     { return MH_OK; }
    MH_STATUS WINAPI MH_RemoveHook(LPVOID)                      { return MH_OK; }
    MH_STATUS WINAPI MH_EnableHook(LPVOID)                      { return MH_OK; }
    MH_STATUS WINAPI MH_DisableHook(LPVOID)                     { return MH_OK; }
    MH_STATUS WINAPI MH_QueueEnableHook(LPVOID)                 { return MH_OK; }
    MH_STATUS WINAPI MH_QueueDisableHook(LPVOID)                { return MH_OK; }
    MH_STATUS WINAPI MH_ApplyQueued(VOID)                       { return MH_OK; }
}
