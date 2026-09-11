// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// gbhook.cmd is renamed to .cmd.run before it is read, so a script writing the next batch cannot race this one.
#include "Commands.h"
#include "Events.h"
#include "FrameHook.h"
#include "Hud.h"
#include "Pump.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "../core/Seh.h"
#include "../mod/Host.h"
#include "cmd/Line.h"
#include "cmd/Table.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
    constexpr int kQueueCap = 64;
    constexpr int kLineCap  = 512;
    constexpr int kDrainPer = 8;     // a burst of queued lines must not turn one frame into a hitch

    CmdTable::Table* g_table = nullptr;
    CRITICAL_SECTION g_lock;
    bool             g_ready = false;

    char g_queue[kQueueCap][kLineCap];
    int  g_qHead = 0, g_qTail = 0, g_qCount = 0;

    void EnsureReady()
    {
        if (g_ready) return;
        InitializeCriticalSection(&g_lock);
        g_table = new CmdTable::Table();
        g_ready = true;
    }

    // Plain-C frame for the guard around a handler.
    int CallHandler(GbhCommandFn fn, int argc, const char* const* argv, const char** err, void* user, bool* faulted)
    {
        *faulted = false;
        GBH_SEH_TRY { return fn(argc, argv, err, user); }
        GBH_SEH_EXCEPT { *faulted = true; return GBH_ERR; }
    }

    int RunLine(const char* line)
    {
        EnsureReady();
        const std::vector<std::string> tok = Line::Tokenize(line);
        if (tok.empty()) return GBH_ERR_ARG;

        CmdTable::Entry copy;
        bool found = false;
        EnterCriticalSection(&g_lock);
        if (const CmdTable::Entry* e = g_table->Find(tok[0])) { copy = *e; found = true; }
        LeaveCriticalSection(&g_lock);
        if (!found)
        {
            Log::Writef("ERR", "%s: unknown command (try `help`)", line);
            return GBH_ERR_NOT_FOUND;
        }

        // argv excludes the command name, as the ABI documents.
        std::vector<const char*> argv;
        for (size_t i = 1; i < tok.size(); ++i) argv.push_back(tok[i].c_str());

        Log::Writef("CMD", "%s", line);
        const char* err = nullptr;
        bool faulted = false;
        const int rc = CallHandler((GbhCommandFn)copy.fn, (int)argv.size(), argv.empty() ? nullptr : argv.data(),
                                   &err, copy.user, &faulted);
        if (faulted)
        {
            Log::Writef("ERR", "%s: the handler faulted (owner '%s')", line, copy.owner.empty() ? "gbhook" : copy.owner.c_str());
            return GBH_ERR;
        }
        if (rc == GBH_OK) Log::Writef("OK", "%s", line);
        else              Log::Writef("ERR", "%s: %s", line, err ? err : "failed");
        return rc;
    }

    bool NeedsGameThread(const char* line)
    {
        const std::vector<std::string> tok = Line::Tokenize(line);
        if (tok.empty()) return false;
        EnterCriticalSection(&g_lock);
        const CmdTable::Entry* e = g_table->Find(tok[0]);
        const bool needs = e && (e->flags & Commands::kGameThread) != 0;
        LeaveCriticalSection(&g_lock);
        return needs;
    }

    bool DrainExists() { return FrameHook::Installed() || Pump::Installed(); }

    bool Enqueue(const char* line)
    {
        EnterCriticalSection(&g_lock);
        if (g_qCount >= kQueueCap)
        {
            LeaveCriticalSection(&g_lock);
            Log::Writef("ERR", "%s: queue full (%d), dropped", line, kQueueCap);
            return false;
        }
        strncpy_s(g_queue[g_qTail], kLineCap, line, _TRUNCATE);
        g_qTail = (g_qTail + 1) % kQueueCap;
        ++g_qCount;
        LeaveCriticalSection(&g_lock);
        Log::Writef("QUEUED", "%s%s", line, FrameHook::Active() ? "" : " (front end)");
        return true;
    }

    // ---- built-ins ----------------------------------------------------------
    int CmdHelp(int, const char* const*, const char**, void*)  { Commands::LogHelp(); return GBH_OK; }
    int CmdMods(int, const char* const*, const char**, void*)  { Host::LogStatus(); Events::LogSummary(); return GBH_OK; }
    int CmdHooks(int, const char* const*, const char**, void*) { HookBroker::VerifyAll("`hooks` command"); return GBH_OK; }

    int CmdPing(int, const char* const*, const char**, void*)
    {
        Log::Writef("RES", "pong -- gbhook %s, game thread %lu, %s, main thread %lu",
                    Framework::kVersionString, FrameHook::GameThreadId(),
                    FrameHook::TickRecently() ? "level live" : "no level ticking", Pump::ThreadId());
        return GBH_OK;
    }

    int CmdHud(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 2) { *err = "usage: hud <seconds> <text...>"; return GBH_ERR_ARG; }
        std::string text;
        for (int i = 1; i < argc; ++i) { if (i > 1) text += ' '; text += argv[i]; }
        return Hud::ShowNow(text.c_str(), (float)atof(argv[0])) ? GBH_OK : (*err = "no level is live", GBH_ERR_STATE);
    }
}

namespace Commands
{
    void Init()
    {
        EnsureReady();
        Register(nullptr, "help",  CmdHelp,  nullptr, "list every command", kAnyThread);
        Register(nullptr, "mods",  CmdMods,  nullptr, "mod and subscription status", kAnyThread);
        Register(nullptr, "hooks", CmdHooks, nullptr, "verify every hook, patch and vtable copy", kAnyThread);
        Register(nullptr, "ping",  CmdPing,  nullptr, "liveness and thread state", kAnyThread);
        Register(nullptr, "hud",   CmdHud,   nullptr, "<seconds> <text> -- the HUD message line", kGameThread);
    }

    int Register(const char* owner, const char* name, GbhCommandFn fn, void* user, const char* help, unsigned flags)
    {
        EnsureReady();
        std::string why;
        EnterCriticalSection(&g_lock);
        const bool ok = g_table->Add(owner, name, (void*)fn, user, help, flags, &why);
        LeaveCriticalSection(&g_lock);
        if (!ok)
        {
            Log::Writef("CMD", "'%s' from '%s' refused: %s", name ? name : "", (owner && *owner) ? owner : "gbhook", why.c_str());
            return why.find("already registered") != std::string::npos ? GBH_ERR_CONFLICT : GBH_ERR_ARG;
        }
        return GBH_OK;
    }

    int ExecuteNow(const char* line)
    {
        if (!line || !*line) return GBH_ERR_ARG;
        return RunLine(line);
    }

    int Execute(const char* line)
    {
        if (!line || !*line) return GBH_ERR_ARG;
        EnsureReady();
        if (NeedsGameThread(line) && !FrameHook::IsGameThread() && DrainExists())
            return Enqueue(line) ? GBH_OK : GBH_ERR_STATE;
        return RunLine(line);
    }

    int Queue(const char* line)
    {
        if (!line || !*line) return GBH_ERR_ARG;
        EnsureReady();
        if (FrameHook::IsGameThread()) return RunLine(line);
        if (!DrainExists())
        {
            Log::Writef("ERR", "%s: nothing drains the queue yet", line);
            return GBH_ERR_STATE;
        }
        return Enqueue(line) ? GBH_OK : GBH_ERR_STATE;
    }

    void Drain()
    {
        if (!g_ready) return;
        char batch[kDrainPer][kLineCap];
        int  n = 0;
        EnterCriticalSection(&g_lock);
        while (n < kDrainPer && g_qCount > 0)
        {
            memcpy(batch[n], g_queue[g_qHead], kLineCap);
            g_qHead = (g_qHead + 1) % kQueueCap;
            --g_qCount;
            ++n;
        }
        LeaveCriticalSection(&g_lock);
        for (int i = 0; i < n; ++i) RunLine(batch[i]);
    }

    void Poll()
    {
        static char src[MAX_PATH], run[MAX_PATH];
        static bool paths = false;
        if (!paths)
        {
            _snprintf_s(src, sizeof src, _TRUNCATE, "%s\\gbhook.cmd", Framework::GameDir());
            _snprintf_s(run, sizeof run, _TRUNCATE, "%s\\gbhook.cmd.run", Framework::GameDir());
            paths = true;
        }
        if (GetFileAttributesA(src) == INVALID_FILE_ATTRIBUTES) return;
        DeleteFileA(run);
        if (!MoveFileA(src, run)) return;

        FILE* f = nullptr;
        if (fopen_s(&f, run, "r") != 0 || !f) return;
        char line[1024];
        while (fgets(line, sizeof line, f))
        {
            const std::string s = Line::StripEol(line);
            if (Line::IsSkippable(s)) continue;
            Execute(s.c_str());
        }
        fclose(f);
        DeleteFileA(run);
    }

    void LogHelp()
    {
        EnsureReady();
        EnterCriticalSection(&g_lock);
        const std::vector<CmdTable::Entry> sorted = g_table->Sorted();
        LeaveCriticalSection(&g_lock);
        Log::Writef("RES", "%d command(s):", (int)sorted.size());
        for (const CmdTable::Entry& c : sorted)
            Log::Writef("RES", "  %-28s %-12s %s%s", c.name.c_str(), c.owner.empty() ? "gbhook" : c.owner.c_str(),
                        c.help.c_str(), (c.flags & kGameThread) ? "  [game thread]" : "");
    }
}
