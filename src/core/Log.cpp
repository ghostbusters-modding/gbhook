// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The single output channel. Held open and flushed per line: the lines just before a crash are the ones that matter.

#include "Framework.h"
#include "format/LogLine.h"

#include <cstdio>
#include <share.h>
#include <cstdarg>

namespace
{
    CRITICAL_SECTION g_lock;
    bool             g_lockReady = false;
    FILE*            g_file      = nullptr;
    char             g_path[MAX_PATH] = { 0 };

    void Emit(const char* id, const char* tag, const char* text)
    {
        if (!g_lockReady) return;

        SYSTEMTIME t;
        GetLocalTime(&t);
        char line[2200];
        LogLine::Format(line, sizeof line, t.wHour, t.wMinute, t.wSecond, tag, id, text);

        EnterCriticalSection(&g_lock);
        if (g_file)
        {
            fputs(line, g_file);
            fputc('\n', g_file);
            fflush(g_file);
        }
        LeaveCriticalSection(&g_lock);

        // Mirrored to the console when one is attached.
        puts(line);
    }
}

namespace Log
{
    void Init()
    {
        if (g_lockReady) return;
        InitializeCriticalSection(&g_lock);
        g_lockReady = true;

        _snprintf_s(g_path, sizeof g_path, _TRUNCATE, "%s\\gbhook.log", Framework::GameDir());

        // Truncated every launch. The previous run survives as .prev, which is all anyone has ever wanted.
        char prev[MAX_PATH];
        _snprintf_s(prev, sizeof prev, _TRUNCATE, "%s.prev", g_path);
        DeleteFileA(prev);
        MoveFileA(g_path, prev);

        // _fsopen with _SH_DENYNO, not fopen_s: fopen_s opens non-shareable, and then nothing can tail the log live.
        g_file = _fsopen(g_path, "w", _SH_DENYNO);

        Writef("BOOT", "gbhook %s -- %s", Framework::kVersionString, GBHOOK_TARGET_NAME);
    }

    void Write(const char* tag, const char* line)                      { Emit(nullptr, tag, line); }
    void WriteFrom(const char* id, const char* tag, const char* line)  { Emit(id, tag, line); }

    void Writef(const char* tag, const char* fmt, ...)
    {
        char buf[2048];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf_s(buf, sizeof buf, _TRUNCATE, fmt, ap);
        va_end(ap);
        Emit(nullptr, tag, buf);
    }

    void WritefFrom(const char* id, const char* tag, const char* fmt, ...)
    {
        char buf[2048];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf_s(buf, sizeof buf, _TRUNCATE, fmt, ap);
        va_end(ap);
        Emit(id, tag, buf);
    }
}
