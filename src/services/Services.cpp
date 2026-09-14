// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Entries are append-only in fixed storage, so a name or owner pointer handed out stays valid for the process.
#include "Services.h"
#include "Commands.h"
#include "../core/Framework.h"
#include "svc/ServiceTable.h"

#include <windows.h>
#include <string>

namespace
{
    ServiceTable*    g_table = nullptr;
    CRITICAL_SECTION g_lock;
    bool             g_ready = false;

    void EnsureReady()
    {
        if (g_ready) return;
        InitializeCriticalSection(&g_lock);
        g_table = new ServiceTable();
        g_ready = true;
    }

    const char* Shown(const char* owner) { return (owner && *owner) ? owner : "gbhook"; }

    int CmdServices(int, const char* const*, const char**, void*)
    {
        struct Row { const char* name; const char* owner; const void* table; uint32_t size; };
        Row row[ServiceTable::kMaxEntries];
        int n = 0;
        EnterCriticalSection(&g_lock);
        for (; n < g_table->Count(); ++n)
        {
            row[n].name  = g_table->NameAt(n);
            row[n].owner = g_table->OwnerOf(row[n].name);
            row[n].table = g_table->Find(row[n].name, &row[n].size);
        }
        LeaveCriticalSection(&g_lock);

        Log::Writef("RES", "%d service(s):", n);
        for (int i = 0; i < n; ++i)
            Log::Writef("RES", "  %-32s %-16s %6u bytes at %p", row[i].name, Shown(row[i].owner), row[i].size, row[i].table);
        return GBH_OK;
    }
}

namespace Services
{
    void Init() { EnsureReady(); }

    int Publish(const char* owner, const char* name, const void* table, uint32_t size)
    {
        EnsureReady();
        std::string why;
        EnterCriticalSection(&g_lock);
        const bool ok     = g_table->Publish(owner, name, table, size, &why);
        const bool taken  = !ok && g_table->Find(name, nullptr) != nullptr;
        const bool full   = !ok && !taken && g_table->Full();
        LeaveCriticalSection(&g_lock);

        if (ok)
        {
            Log::Writef("SVC", "'%s' published by %s (%u bytes)", name, Shown(owner), size);
            return GBH_OK;
        }
        Log::Writef("SVC", "%s could not publish: %s", Shown(owner), why.c_str());
        return taken ? GBH_ERR_CONFLICT : (full ? GBH_ERR : GBH_ERR_ARG);
    }

    const void* Find(const char* name, uint32_t* size)
    {
        if (!g_ready) { if (size) *size = 0; return nullptr; }
        EnterCriticalSection(&g_lock);
        const void* t = g_table->Find(name, size);
        LeaveCriticalSection(&g_lock);
        return t;
    }

    int Count()
    {
        if (!g_ready) return 0;
        EnterCriticalSection(&g_lock);
        const int n = g_table->Count();
        LeaveCriticalSection(&g_lock);
        return n;
    }

    const char* NameAt(int i)
    {
        if (!g_ready) return nullptr;
        EnterCriticalSection(&g_lock);
        const char* s = g_table->NameAt(i);
        LeaveCriticalSection(&g_lock);
        return s;
    }

    const char* OwnerOf(const char* name)
    {
        if (!g_ready) return nullptr;
        EnterCriticalSection(&g_lock);
        const char* s = g_table->OwnerOf(name);
        LeaveCriticalSection(&g_lock);
        return s;
    }

    void RegisterCommands()
    {
        EnsureReady();
        Commands::Register(nullptr, "services", CmdServices, nullptr, "every published service table, its owner and size", Commands::kAnyThread);
    }

    void LogSummary()
    {
        Log::Writef("SVC", "%d service(s) published", Count());
    }
}
