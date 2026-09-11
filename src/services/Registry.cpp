// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Registry.h"
#include "../core/Framework.h"

#include <windows.h>
#include <cctype>
#include <cstring>

namespace
{
    CRITICAL_SECTION g_lock;
    bool             g_lockReady = false;
    std::vector<Registry::Entry>* g_entries = nullptr;
    uint32_t         g_generation = 1;

    void Ensure()
    {
        if (g_lockReady) return;
        InitializeCriticalSection(&g_lock);
        g_entries  = new std::vector<Registry::Entry>();
        g_lockReady = true;
    }

    bool IEquals(const std::string& a, const char* b)
    {
        if (a.size() != strlen(b)) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
        return true;
    }

    bool IContains(const std::string& hay, const char* needle)
    {
        std::string h = hay, n = needle;
        for (char& c : h) c = (char)tolower((unsigned char)c);
        for (char& c : n) c = (char)tolower((unsigned char)c);
        return h.find(n) != std::string::npos;
    }
}

namespace Registry
{
    void Register(const char* buffer, void* ptr)
    {
        if (!buffer || !ptr) return;
        const char* sp = strchr(buffer, ' ');
        if (!sp || sp == buffer || buffer[0] != 'C') return;
        const char* name = sp;
        while (*name == ' ') ++name;
        if (!*name) return;

        Ensure();
        EnterCriticalSection(&g_lock);
        g_entries->push_back(Entry{ std::string(buffer, (size_t)(sp - buffer)), std::string(name), ptr, g_generation });
        LeaveCriticalSection(&g_lock);
    }

    void NewGeneration()
    {
        Ensure();
        EnterCriticalSection(&g_lock);
        const size_t had = g_entries->size();
        const uint32_t gen = g_generation++;
        g_entries->clear();
        LeaveCriticalSection(&g_lock);
        if (had) Log::Writef("REG", "generation %u dropped (%d entries)", gen, (int)had);
    }

    void Snapshot(std::vector<Entry>& out)
    {
        Ensure();
        EnterCriticalSection(&g_lock);
        out = *g_entries;
        LeaveCriticalSection(&g_lock);
    }

    bool Find(const char* name, Entry& out)
    {
        if (!name || !*name) return false;
        std::vector<Entry> snap;
        Snapshot(snap);
        for (const Entry& e : snap) if (IEquals(e.name, name))   { out = e; return true; }
        for (const Entry& e : snap) if (IContains(e.name, name)) { out = e; return true; }
        return false;
    }

    uint32_t Generation() { Ensure(); return g_generation; }

    int Count()
    {
        Ensure();
        EnterCriticalSection(&g_lock);
        const int n = (int)g_entries->size();
        LeaveCriticalSection(&g_lock);
        return n;
    }

    void LogSummary(const char* level)
    {
        std::vector<Entry> snap;
        Snapshot(snap);
        std::vector<std::string> classes;
        for (const Entry& e : snap)
        {
            bool seen = false;
            for (const std::string& c : classes) if (c == e.cls) { seen = true; break; }
            if (!seen) classes.push_back(e.cls);
        }
        Log::Writef("REG", "%d actors in %d classes for '%s' (generation %u)", (int)snap.size(), (int)classes.size(),
                    level ? level : "", g_generation);
    }
}
