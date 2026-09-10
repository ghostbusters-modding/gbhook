// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ProcessMemory.h"
#include "Framework.h"
#include "Seh.h"

#include <cstring>

namespace
{
    bool GuardedCopy(void* dst, const void* src, size_t n)
    {
        GBH_SEH_TRY
        {
            memcpy(dst, src, n);
            return true;
        }
        GBH_SEH_EXCEPT
        {
            return false;
        }
    }
}

bool ProcessMemory::Read(uintptr_t at, void* out, size_t n)
{
    return at != 0 && GuardedCopy(out, (const void*)at, n);
}

bool ProcessMemory::Write(uintptr_t at, const void* in, size_t n)
{
    if (!at) return false;
    DWORD old = 0;
    if (!VirtualProtect((void*)at, n, PAGE_EXECUTE_READWRITE, &old)) return false;
    const bool ok = GuardedCopy((void*)at, in, n);
    DWORD restored = 0;
    VirtualProtect((void*)at, n, old, &restored);
    if (ok) FlushInstructionCache(GetCurrentProcess(), (void*)at, n);
    return ok;
}

void* ProcessMemory::Alloc(size_t n)
{
    return Framework::Alloc(n);
}
