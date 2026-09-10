// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "FaultLogger.h"
#include "Framework.h"

namespace
{
    unsigned long long g_lo = 0, g_hi = 0;
    unsigned long long g_seen[32] = { 0 };
    int  g_seenN = 0;
    bool g_installed = false;

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

        // Only ghost.exe's own code. Our guarded probes fault inside this module by design.
        if (at < g_lo || at >= g_hi) return EXCEPTION_CONTINUE_SEARCH;

        // One line per site, or a repeating fault fills the file.
        for (int i = 0; i < g_seenN; ++i)
            if (g_seen[i] == at) return EXCEPTION_CONTINUE_SEARCH;
        if (g_seenN < 32) g_seen[g_seenN++] = at;

        const unsigned long long rel = at - (unsigned long long)gameBase;
        const unsigned long long data =
            (code == 0xC0000005 && ep->ExceptionRecord->NumberParameters >= 2)
                ? ep->ExceptionRecord->ExceptionInformation[1] : 0;

        Log::Writef("FAULT", "code=%08X at ghost+0x%llX data=0x%llX -- first chance. "
                             "If the game dies right after this, THIS is the site.",
                    code, rel, data);
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
            Log::Write("BOOT", "crash forensics armed (VEH over ghost.exe code)");
        }
    }
}
