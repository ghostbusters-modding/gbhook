// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The table index is the make code with the extended flag in bit 8, under a live mask the engine keeps at 0x7F
// or 0x1FF. docs/engine/RE_INPUT_BINDINGS.md has the decompile trail.
#include "InputInject.h"
#include "Commands.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "gb/Globals.h"
#include "input/Dik.h"

#include <windows.h>
#include <cstdlib>
#include <cstring>

namespace
{
    unsigned ReadMask()
    {
        if (!gameBase) return Dik::kMaskDefault;
        GBH_SEH_TRY
        {
            const unsigned m = *reinterpret_cast<volatile unsigned*>(gameBase + Globals::scanTableMask);
            return (m == Dik::kMaskDefault || m == Dik::kMaskExtended) ? m : Dik::kMaskDefault;
        }
        GBH_SEH_EXCEPT { return Dik::kMaskDefault; }
    }

    void Write(int idx, unsigned char v)
    {
        if (!gameBase || idx < 0 || idx >= Dik::kTableEntries) return;
        GBH_SEH_TRY { reinterpret_cast<volatile unsigned char*>(gameBase + Globals::scanTable)[idx] = v; }
        GBH_SEH_EXCEPT {}
    }

    int ClampMs(int ms) { return ms < 1 ? 1 : (ms > 60000 ? 60000 : ms); }

    // Re-asserted every 16 ms: costs nothing, and survives a reader that clears the entry.
    void HoldFor(int dik, int ms)
    {
        const DWORD end = GetTickCount() + (DWORD)ms;
        do { InputInject::SetKey(dik, true); Sleep(16); } while ((int)(end - GetTickCount()) > 0);
        InputInject::SetKey(dik, false);
    }

    const char* kKeyUsage = "usage: key tap|down|up|hold|spam <NAME> [1..60000 ms] | key clear";

    int CmdKey(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = kKeyUsage; return GBH_ERR_ARG; }
        const char* op = argv[0];
        if (_stricmp(op, "clear") == 0) { InputInject::Clear(); return GBH_OK; }
        if (argc < 2) { *err = kKeyUsage; return GBH_ERR_ARG; }
        const int dik = Dik::FromName(argv[1]);
        if (dik < 0) { *err = "unknown key name"; return GBH_ERR_ARG; }
        const int ms = argc >= 3 ? atoi(argv[2]) : 0;

        if (_stricmp(op, "tap") == 0)       HoldFor(dik, 60);
        else if (_stricmp(op, "down") == 0) InputInject::SetKey(dik, true);
        else if (_stricmp(op, "up") == 0)   InputInject::SetKey(dik, false);
        else if (_stricmp(op, "hold") == 0)
        {
            if (ms < 1 || ms > 60000) { *err = kKeyUsage; return GBH_ERR_ARG; }
            HoldFor(dik, ms);
        }
        else if (_stricmp(op, "spam") == 0)
        {
            if (ms < 1 || ms > 60000) { *err = kKeyUsage; return GBH_ERR_ARG; }
            const DWORD end = GetTickCount() + (DWORD)ms;
            do { HoldFor(dik, 60); Sleep(90); } while ((int)(end - GetTickCount()) > 0);
        }
        else { *err = kKeyUsage; return GBH_ERR_ARG; }
        return GBH_OK;
    }

    int CmdSleep(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = "usage: sleep <1..60000 ms>"; return GBH_ERR_ARG; }
        Sleep((DWORD)ClampMs(atoi(argv[0])));
        return GBH_OK;
    }
}

namespace InputInject
{
    void SetKey(int dik, bool down)
    {
        Write(Dik::TableIndex(dik, ReadMask()), down ? 1 : 0);
    }

    void Clear()
    {
        for (int i = 0; i < Dik::kTableEntries; ++i) Write(i, 0);
    }

    void RegisterCommands()
    {
        Commands::Register(nullptr, "key",   CmdKey,   nullptr, "tap|down|up|hold|spam <NAME> [ms] | clear", Commands::kAnyThread);
        Commands::Register(nullptr, "sleep", CmdSleep, nullptr, "<ms> -- pause the command stream", Commands::kAnyThread);
    }
}
