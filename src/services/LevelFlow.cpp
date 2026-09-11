// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "LevelFlow.h"
#include "Events.h"
#include "Game.h"
#include "Registry.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "../core/Seh.h"
#include "gb/HookTargets.h"

#include <windows.h>
#include <cstring>

namespace
{
    char g_lastLevel[64] = { 0 };

    typedef __int64 (__fastcall* tPrepare)(void*, void*, void*, void*);
    tPrepare oPrepare = nullptr;

    constexpr int kMaxErrLines = 40;
    constexpr int kMaxErrLen   = 256;

    // prepare fills its second argument, a CStrList { vftable, int count +8, char** items +0x10 }, with readable
    // error text even in retail, which never prints it. Copied under a guard, then logged.
    int CopyPrepareErrors(void* list, char out[kMaxErrLines][kMaxErrLen], int* total)
    {
        *total = 0;
        GBH_SEH_TRY
        {
            if (!list) return 0;
            const int n = *reinterpret_cast<int*>(static_cast<char*>(list) + 8);
            char** items = *reinterpret_cast<char***>(static_cast<char*>(list) + 0x10);
            *total = n;
            if (n <= 0 || n > 4096 || !items) return 0;
            int shown = 0;
            for (int i = 0; i < n && shown < kMaxErrLines; ++i)
                if (items[i]) lstrcpynA(out[shown++], items[i], kMaxErrLen);
            return shown;
        }
        GBH_SEH_EXCEPT { return -1; }
    }

    void DumpPrepareErrors(void* list)
    {
        static char lines[kMaxErrLines][kMaxErrLen];
        int total = 0;
        const int n = CopyPrepareErrors(list, lines, &total);
        if (n < 0)  { Log::Write("LEVEL", "prepare error list unreadable"); return; }
        if (n == 0) { Log::Writef("LEVEL", "prepare error list empty (%d)", total); return; }
        for (int i = 0; i < n; ++i) Log::Writef("LEVEL", "prepare error %d/%d: %s", i + 1, total, lines[i]);
        if (total > n) Log::Writef("LEVEL", "... %d more prepare errors not shown", total - n);
    }

    __int64 __fastcall PrepareDetour(void* a, void* b, void* c, void* d)
    {
        char stem[64];
        Game::LevelStem(stem, sizeof stem);
        Registry::NewGeneration();
        Events::FireLevel(GBH_LEVEL_PREPARE_BEGIN, stem, true);

        const __int64 r = oPrepare ? oPrepare(a, b, c, d) : 0;

        // Re-read: the loader may have swapped the stem in during the call.
        Game::LevelStem(stem, sizeof stem);
        lstrcpynA(g_lastLevel, stem, (int)sizeof g_lastLevel);
        const bool ok = static_cast<char>(r) != 0;
        Log::Writef("LEVEL", "prepare '%s' %s", stem, ok ? "succeeded" : "FAILED");
        if (!ok) DumpPrepareErrors(b);
        Registry::LogSummary(stem);
        Events::FireLevel(GBH_LEVEL_PREPARE_END, stem, ok);
        return r;
    }

    // A script fault that is not logged is a level that silently does nothing.
    typedef void (__fastcall* tGtfo)(const char*, int);
    tGtfo oGtfo = nullptr;

    void __fastcall GtfoDetour(const char* msg, int code)
    {
        Log::Writef("GTFO", "script fault (%d): %s", code, msg ? msg : "(null)");
        if (oGtfo) oGtfo(msg, code);
    }
}

namespace LevelFlow
{
    void InstallFlowHooks()
    {
        if (!gameBase) return;
        if (HookBroker::Install(nullptr, gameBase + HookTargets::levelPrepare, (void*)&PrepareDetour,
                                (void**)&oPrepare, GBH_HOOK_EXCLUSIVE))
            Log::Write("LEVEL", "level prepare hooked (level bus)");
        else
            Log::Write("LEVEL", "prepare hook FAILED to install; on_level will never fire");

        if (!HookBroker::Install(nullptr, gameBase + HookTargets::GTFO, (void*)&GtfoDetour, (void**)&oGtfo,
                                 GBH_HOOK_EXCLUSIVE))
            Log::Write("LEVEL", "script-fault hook FAILED to install; script faults go unlogged");
    }

    const char* CurrentLevel() { return g_lastLevel; }
}
