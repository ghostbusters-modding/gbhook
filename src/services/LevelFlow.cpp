// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "LevelFlow.h"
#include "Commands.h"
#include "Events.h"
#include "FrameHook.h"
#include "Game.h"
#include "Registry.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "../core/Seh.h"
#include "gb/HookTargets.h"

#include <windows.h>
#include <cstdio>
#include <cstring>

namespace
{
    // Level flow globals, ghost-relative. docs/engine/RE_NOTES.md section 4, SOURCEMAP.md section 1.
    constexpr uintptr_t kPendingLevel = 0x23229D0;   // char[]      the pending level file, "NAME.LVL"
    constexpr uintptr_t kPendingCp    = 0x1671800;   // char[0x100] the pending checkpoint name
    constexpr uintptr_t kReloadFlag   = 0x16520FC;   // u8  reload the pending level now
    constexpr uintptr_t kActionSel    = 0x20D43D0;   // int front-end action, 5 = load pending
    constexpr uintptr_t kActionPend   = 0x20D43CD;   // u8  front-end action pending
    constexpr uintptr_t kChainFlag    = 0x4A9F1;     // CGame + 0x4A9F1, u8 "this load is a chain"

    // The live level's checkpoint table, filled by its setupLevel(): the engine's loads speak the display name.
    constexpr uintptr_t kCpTable      = 0x4A9F4;     // CGame + 0x4A9F4, 25 rows
    constexpr size_t    kCpRow        = 0x304;       // +0 display name, +0x100 level, +0x200 prototype, +0x300 valid
    constexpr int       kCpRows       = 25;

    // What the action poll itself does before returning 5; the menu loop then runs its own load-and-teardown case.
    constexpr uintptr_t kFnScreenClose = 0x246BA0;
    constexpr uintptr_t kFnMenuCleanup = 0x246A50;

    char          g_lastLevel[64]    = { 0 };
    char          g_feLoadLvl[64]    = { 0 };
    volatile LONG g_feLoadPending    = 0;
    char          g_feCheckpoint[128] = { 0 };

    // Begin-level's profile sync: the table is filled by then and the pending name is resolved right after 
    typedef void (__fastcall* tSync)(void*);
    tSync oSync = nullptr;
    int  CheckpointRow(void* gg, const char* name, char* out, size_t cap, int* rowOut, char* known, size_t knownCap);
    bool WritePendingCheckpoint(const char* cp);
    void __fastcall SyncDetour(void* gg)
    {
        if (g_feCheckpoint[0])
        {
            char display[0x100], known[1024];
            int row = -1;
            const int r = CheckpointRow(gg, g_feCheckpoint, display, sizeof display, &row, known, sizeof known);
            if (r == 0 && WritePendingCheckpoint(display))
                Log::Writef("LEVEL", "checkpoint '%s' armed at begin-level as '%s' (row %d)", g_feCheckpoint, display, row);
            else
                Log::Writef("LEVEL", "checkpoint '%s' is not one this level registers; it has: %s", g_feCheckpoint,
                            known[0] ? known : "(none)");
            g_feCheckpoint[0] = 0;
        }
        if (oSync) oSync(gg);
    }

    // ---- guarded writes into the engine, plain-C frames ----------------------
    // Begin-level consults the chain flag once: at 0 the pending checkpoint becomes the profile's resume row.
    bool SetChainFlag(void* gg)
    {
        if (!gg) return false;
        GBH_SEH_TRY { *(reinterpret_cast<unsigned char*>(gg) + kChainFlag) = 1; return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    bool ArmChain(const char* file)
    {
        GBH_SEH_TRY
        {
            strncpy_s(reinterpret_cast<char*>(gameBase + kPendingLevel), 64, file, _TRUNCATE);
            *reinterpret_cast<char*>(gameBase + kPendingCp)            = '\0';
            *reinterpret_cast<unsigned char*>(gameBase + kReloadFlag)  = 1;
            *reinterpret_cast<int*>(gameBase + kActionSel)             = 5;
            *reinterpret_cast<unsigned char*>(gameBase + kActionPend)  = 1;
            return true;
        }
        GBH_SEH_EXCEPT { return false; }
    }

    bool SetReloadFlag()
    {
        GBH_SEH_TRY { *reinterpret_cast<unsigned char*>(gameBase + kReloadFlag) = 1; return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    bool WritePendingCheckpoint(const char* cp)
    {
        GBH_SEH_TRY { strncpy_s(reinterpret_cast<char*>(gameBase + kPendingCp), 0x100, cp, _TRUNCATE); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // The row whose prototype is "void <name>()", or whose display name is `name` already: its index and name. 
    int CheckpointRow(void* gg, const char* name, char* out, size_t cap, int* rowOut, char* known, size_t knownCap)
    {
        // 0 = found, 1 = no such row (the registered display names land in `known`), -1 = the table is unreadable.
        if (!gg) return -1;
        GBH_SEH_TRY
        {
            char proto[0x110];
            _snprintf_s(proto, sizeof proto, _TRUNCATE, "void %s()", name);
            known[0] = 0;
            for (int i = 0; i < kCpRows; ++i)
            {
                const char* row = reinterpret_cast<const char*>(gg) + kCpTable + (size_t)i * kCpRow;
                if (*reinterpret_cast<const int*>(row + 0x300) == 0 || !row[0]) continue;
                if (_stricmp(row + 0x200, proto) == 0 || _stricmp(row, name) == 0)
                {
                    lstrcpynA(out, row, (int)cap);
                    *rowOut = i;
                    return 0;
                }
                if (known[0]) strncat_s(known, knownCap, ", ", _TRUNCATE);
                strncat_s(known, knownCap, row, _TRUNCATE);
            }
            return 1;
        }
        GBH_SEH_EXCEPT { return -1; }
    }

    bool ArmFrontEnd(const char* file)
    {
        GBH_SEH_TRY
        {
            lstrcpynA(reinterpret_cast<char*>(gameBase + kPendingLevel), file, 40);
            *reinterpret_cast<char*>(gameBase + kPendingCp)           = '\0';
            *reinterpret_cast<unsigned char*>(gameBase + kActionPend) = 0;   // nothing double-fires later
            *reinterpret_cast<int*>(gameBase + kActionSel)            = 0;
            return true;
        }
        GBH_SEH_EXCEPT { return false; }
    }

    bool FeCall1(const char* what, uintptr_t rva, void* a)
    {
        typedef void (__fastcall* F)(void*);
        GBH_SEH_TRY { ((F)(gameBase + rva))(a); return true; }
        GBH_SEH_EXCEPT { Log::Writef("LEVEL", "EXC in %s", what); return false; }
    }

    bool FeCall2(const char* what, uintptr_t rva, void* a, __int64 b)
    {
        typedef void (__fastcall* F)(void*, __int64);
        GBH_SEH_TRY { ((F)(gameBase + rva))(a, b); return true; }
        GBH_SEH_EXCEPT { Log::Writef("LEVEL", "EXC in %s", what); return false; }
    }

    // ---- the front-end load -----------------------------------------------------
    // Answering the poll with 5 hands the load to the menu loop's own case, which tears the level down and re-arms
    // the title when it ends. Running the loader from the pump left that loop mid-frame with no screen: a black title.
    typedef __int64 (__fastcall* tDispatch)(void*);
    tDispatch oDispatch = nullptr;

    __int64 __fastcall DispatchDetour(void* feMgr)
    {
        const bool pending = g_feLoadPending != 0;   // a request raised inside this poll waits for the next frame
        const __int64 r = oDispatch ? oDispatch(feMgr) : 0;
        if (!pending || !InterlockedExchange(&g_feLoadPending, 0)) return r;

        if (static_cast<int>(r) != 0)
        {
            Log::Writef("LEVEL", "front-end load of '%s' dropped: the menu chose action %d first", g_feLoadLvl, (int)r);
            return r;
        }
        void* gg = Game::Singleton();
        if (!gg)                 { Log::Write("LEVEL", "front-end load abort: no game singleton"); return r; }
        if (Game::LocalPlayer()) { Log::Write("LEVEL", "front-end load abort: a level is live"); return r; }
        if (!ArmFrontEnd(g_feLoadLvl)) { Log::Write("LEVEL", "EXC arming the pending level"); return r; }
        SetChainFlag(gg);   // a fresh start, not the profile's resume row
        FeCall2("screenClose", kFnScreenClose, feMgr, 0);
        FeCall1("menuCleanup", kFnMenuCleanup, feMgr);
        Log::Writef("LEVEL", "front-end load of '%s' handed to the menu loop as action 5", g_feLoadLvl);
        return 5;
    }

    // ---- level prepare --------------------------------------------------------
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

    // ---- commands -------------------------------------------------------------
    int CmdLevel(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = "usage: level <stem> [checkpoint]"; return GBH_ERR_ARG; }
        if (!LevelFlow::ChainToLevel(argv[0])) { *err = "could not arm the level chain"; return GBH_ERR; }
        if (argc >= 2) LevelFlow::DeferCheckpoint(argv[1]);
        return GBH_OK;
    }

    int CmdCheckpoint(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = "usage: checkpoint <name>"; return GBH_ERR_ARG; }
        if (!LevelFlow::LoadCheckpoint(argv[0])) { *err = "could not arm the checkpoint"; return GBH_ERR; }
        return GBH_OK;
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

        if (!HookBroker::Install(nullptr, gameBase + HookTargets::levelBeginSync, (void*)&SyncDetour, (void**)&oSync,
                                 GBH_HOOK_EXCLUSIVE))
            Log::Write("LEVEL", "begin-level sync hook FAILED to install; checkpoints cannot be armed");

        if (!HookBroker::Install(nullptr, gameBase + HookTargets::frontEndAction, (void*)&DispatchDetour,
                                 (void**)&oDispatch, GBH_HOOK_EXCLUSIVE))
            Log::Write("LEVEL", "action poll hook FAILED to install; levels cannot load from the front end");
    }

    void RegisterCommands()
    {
        Commands::Register(nullptr, "level",      CmdLevel,      nullptr, "<stem> [checkpoint] -- load a level, front end included", Commands::kGameThread);
        Commands::Register(nullptr, "checkpoint", CmdCheckpoint, nullptr, "<name> -- arm a checkpoint in the live level", Commands::kGameThread);
    }

    bool ChainToLevel(const char* level)
    {
        if (!gameBase || !level || !*level) return false;

        // At the front end the per-level loop is parked and an armed action is never consumed: the poll answers.
        if (!FrameHook::TickRecently() && !Game::LocalPlayer())
        {
            if (!oDispatch) { Log::Write("LEVEL", "no action poll hook; the front end cannot load a level"); return false; }
            char file[64];
            const size_t n = strlen(level);
            if (n > 4 && _stricmp(level + n - 4, ".lvl") == 0) lstrcpynA(file, level, (int)sizeof file);
            else _snprintf_s(file, sizeof file, _TRUNCATE, "%s.lvl", level);
            lstrcpynA(g_feLoadLvl, file, (int)sizeof g_feLoadLvl);
            InterlockedExchange(&g_feLoadPending, 1);
            Log::Writef("LEVEL", "front-end load of '%s' requested (the next action poll takes it)", file);
            return true;
        }

        // The pending globals directly: the engine's chainToLevel sets the same ones and then needs the loop.
        char file[128];
        _snprintf_s(file, sizeof file, _TRUNCATE, "%s.LVL", level);
        if (!ArmChain(file)) { Log::Writef("LEVEL", "chain to '%s' FAILED: the flow globals are unmapped", level); return false; }
        SetChainFlag(Game::Singleton());
        Log::Writef("LEVEL", "chain to '%s' armed", level);
        return true;
    }

    // Begin-level resolves the pending name against the same table and invokes the row's prototype 
    bool LoadCheckpoint(const char* checkpoint)
    {
        if (!gameBase || !checkpoint || !*checkpoint) return false;
        char display[0x100], known[1024];
        int row = -1;
        const int r = CheckpointRow(Game::Singleton(), checkpoint, display, sizeof display, &row, known, sizeof known);
        if (r < 0) { Log::Write("LEVEL", "checkpoint table unreadable: no level is live"); return false; }
        if (r > 0)
        {
            Log::Writef("LEVEL", "'%s' is not a checkpoint the live level registered; it has: %s", checkpoint,
                        known[0] ? known : "(none)");
            return false;
        }
        // The name itself is written at the reload's begin-level 
        lstrcpynA(g_feCheckpoint, checkpoint, (int)sizeof g_feCheckpoint);
        if (!SetReloadFlag()) return false;
        Log::Writef("LEVEL", "checkpoint '%s' ('%s') queued; the level reloads and begins there", checkpoint, display);
        return true;
    }

    void DeferCheckpoint(const char* checkpoint)
    {
        lstrcpynA(g_feCheckpoint, checkpoint ? checkpoint : "", (int)sizeof g_feCheckpoint);
        Log::Writef("LEVEL", "checkpoint '%s' queued for the level's begin", g_feCheckpoint);
    }

    bool FrontEndLoadPending() { return g_feLoadPending != 0; }

    const char* CurrentLevel() { return g_lastLevel; }
}
