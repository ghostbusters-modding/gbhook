// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "FaultLogger.h"
#include "Framework.h"

#include <cstdio>
#include <cstring>

namespace
{
    unsigned long long g_lo = 0, g_hi = 0;
    unsigned long long g_seen[32] = { 0 };
    int  g_seenN = 0;
    bool g_installed = false;
    volatile LONG g_inHandler = 0;   // a fault while walking the stack must not re-enter this handler

    // The instruction's module and offset, "ghost+0x..." for the exe.
    void Where(unsigned long long at, char* out, size_t cap)
    {
        if (at >= g_lo && at < g_hi) { _snprintf_s(out, cap, _TRUNCATE, "ghost+0x%llX", at - g_lo); return; }
        HMODULE m = nullptr;
        char file[MAX_PATH] = { 0 };
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)at, &m) && m && GetModuleFileNameA(m, file, sizeof file))
        {
            const char* leaf = strrchr(file, '\\');
            _snprintf_s(out, cap, _TRUNCATE, "%s+0x%llX", leaf ? leaf + 1 : file, at - (unsigned long long)m);
        }
        else
            _snprintf_s(out, cap, _TRUNCATE, "%llX (no module)", at);
    }

    // The callers of the faulting instruction, unwound from the exception context with the modules' own tables.
    void LogStack(const CONTEXT* faulting)
    {
        CONTEXT ctx = *faulting;
        for (int i = 0; i < 24; ++i)
        {
            DWORD64 base = 0;
            RUNTIME_FUNCTION* fn = RtlLookupFunctionEntry(ctx.Rip, &base, nullptr);
            if (!fn)
            {
                // A leaf function: the return address sits at the top of the stack.
                ctx.Rip = *reinterpret_cast<DWORD64*>(ctx.Rsp);
                ctx.Rsp += 8;
            }
            else
            {
                void* handlerData = nullptr;
                DWORD64 establisher = 0;
                RtlVirtualUnwind(UNW_FLAG_NHANDLER, base, ctx.Rip, fn, &ctx, &handlerData, &establisher, nullptr);
            }
            if (!ctx.Rip) break;
            char w[MAX_PATH + 32];
            Where(ctx.Rip, w, sizeof w);
            Log::Writef("FAULT", "  caller %2d: %s", i + 1, w);
        }
    }

    LONG CALLBACK FaultVeh(EXCEPTION_POINTERS* ep)
    {
        if (!ep || !ep->ExceptionRecord) return EXCEPTION_CONTINUE_SEARCH;

        const DWORD code = ep->ExceptionRecord->ExceptionCode;
        if (code != 0xC0000005 /* access violation */ &&
            code != 0xC0000094 /* integer divide by zero */ &&
            code != 0xC0000096 /* privileged instruction */ &&
            code != 0xC000001D /* illegal instruction */)
            return EXCEPTION_CONTINUE_SEARCH;

        const unsigned long long at =
            (unsigned long long)ep->ExceptionRecord->ExceptionAddress;

        // One line per site, or a repeating fault fills the file.
        for (int i = 0; i < g_seenN; ++i)
            if (g_seen[i] == at) return EXCEPTION_CONTINUE_SEARCH;
        if (g_seenN < 32) g_seen[g_seenN++] = at;

        const unsigned long long data =
            (code == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2)
                ? ep->ExceptionRecord->ExceptionInformation[1] : 0;

        if (InterlockedCompareExchange(&g_inHandler, 1, 0) != 0) return EXCEPTION_CONTINUE_SEARCH;

        char where[MAX_PATH + 32];
        Where(at, where, sizeof where);
        Log::Writef("FAULT", "code=%08X at %s data=0x%llX on thread %lu -- first chance. "
                             "If the game dies right after this, THIS is the site.",
                    code, where, data, GetCurrentThreadId());
        if (ep->ContextRecord) LogStack(ep->ContextRecord);

        InterlockedExchange(&g_inHandler, 0);
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

namespace FaultLogger
{
    void Install()
    {
        if (g_installed || !gameBase) return;

        // ghost.exe's code range, from its own PE header.
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)gameBase;
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return;
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(gameBase + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return;

        g_lo = (unsigned long long)gameBase;
        g_hi = g_lo + nt->OptionalHeader.SizeOfImage;

        // 0 = last in the chain, so a fault something else legitimately handles is not reported.
        if (AddVectoredExceptionHandler(0, FaultVeh))
        {
            g_installed = true;
            Log::Write("BOOT", "crash forensics armed (every first-chance fault is named by module)");
        }
    }
}
