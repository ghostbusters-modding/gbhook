// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Process-wide facts and the framework heap.

#include "Framework.h"

#include <cstring>

char* gameBase = nullptr;

namespace
{
    char          g_gameDir[MAX_PATH] = { 0 };
    bool          g_dirReady = false;
    HANDLE        g_heap     = nullptr;
    volatile LONG g_stages   = 0;   // one bit per GbhStage

    // A private heap, not the CRT's: a static CRT per module means a cross-module free is corruption.
    HANDLE Heap()
    {
        if (!g_heap) g_heap = HeapCreate(0, 0, 0);
        return g_heap ? g_heap : GetProcessHeap();
    }

    void ResolveDir()
    {
        if (g_dirReady) return;
        g_dirReady = true;

        // The executable's own directory. Steam makes no promise about the working directory.
        GetModuleFileNameA(nullptr, g_gameDir, MAX_PATH);
        if (char* slash = strrchr(g_gameDir, '\\')) *slash = '\0';
    }
}

namespace Framework
{
    const char* GameDir() { ResolveDir(); return g_gameDir; }

    bool StageReached(GbhStage s)
    {
        return (InterlockedCompareExchange(&g_stages, 0, 0) & (1L << (int)s)) != 0;
    }

    void NoteStage(GbhStage s)
    {
        InterlockedOr(&g_stages, 1L << (int)s);
    }

    void* Alloc(size_t n)            { return HeapAlloc(Heap(), 0, n ? n : 1); }
    void  Free(void* p)              { if (p) HeapFree(Heap(), 0, p); }
    void* Realloc(void* p, size_t n)
    {
        if (!p) return Alloc(n);
        if (!n) { Free(p); return nullptr; }
        return HeapReAlloc(Heap(), 0, p, n);
    }
}
