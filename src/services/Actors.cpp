// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The chain out of live memory through ProcessMemory; actors/ does the walk and the RTTI decode, this shapes the
// ABI entries and locks the class cache. Every snapshot walks the whole chain: a command's cost, not a frame's.
#include "Actors.h"
#include "Commands.h"
#include "Game.h"
#include "Registry.h"
#include "../actors/ActorList.h"
#include "../actors/Rtti.h"
#include "../core/Framework.h"
#include "../core/ProcessMemory.h"

#include <windows.h>
#include <cstdint>
#include <cstring>
#include <vector>

namespace
{
    CRITICAL_SECTION g_lock;
    bool             g_lockReady = false;
    ProcessMemory    g_mem;
    Rtti::Cache*     g_cache = nullptr;

    constexpr int kListCap = 200;

    void Ensure()
    {
        if (g_lockReady) return;
        InitializeCriticalSection(&g_lock);
        g_cache = new Rtti::Cache(g_mem);
        g_lockReady = true;
    }

    bool IEquals(const char* a, const char* b) { return _stricmp(a, b) == 0; }

    bool IContains(const char* hay, const char* needle)
    {
        const size_t n = strlen(needle);
        for (const char* p = hay; *p; ++p) if (_strnicmp(p, needle, n) == 0) return true;
        return n == 0;
    }

    // Every node as plain data. GBH_ERR_STATE when gGame or its head is unreadable; the bound is a log line only.
    int Collect(std::vector<ActorList::Node>& nodes)
    {
        nodes.clear();
        void* game = Game::Singleton();
        if (!game) return GBH_ERR_STATE;
        nodes.resize((size_t)ActorList::kMaxNodes);
        const int n = ActorList::Walk(g_mem, (uintptr_t)game, nodes.data(), ActorList::kMaxNodes);
        if (n < 0) { nodes.clear(); return GBH_ERR_STATE; }
        if (n == ActorList::kMaxNodes) Log::Writef("ACT", "walk stopped at the %d node bound", n);
        nodes.resize((size_t)n);
        return n;
    }

    // One full entry. The caller holds g_lock for the cache.
    void Entry(const ActorList::Node& n, uint32_t generation, GbhActorInfo& e)
    {
        memset(&e, 0, sizeof e);
        e.struct_size = sizeof e;
        e.ptr = (void*)n.addr;
        strncpy_s(e.name, n.name, _TRUNCATE);
        e.cls[0] = '?';
        if (const Rtti::Chain* c = g_cache->Lookup(n.vtable))
        {
            strncpy_s(e.cls, c->leaf, _TRUNCATE);
            e.flags = c->flags;
        }
        if (n.enabled) e.flags |= GBH_ACTOR_ENABLED;
        memcpy(e.pos,    n.pos,    sizeof e.pos);
        memcpy(e.orient, n.orient, sizeof e.orient);
        e.team       = n.team;
        e.last_frame = n.lastFrame;
        e.generation = generation;
    }

    // min(stride, sizeof) bytes into the caller's slot, struct_size saying how many.
    void Emit(GbhActorInfo* dst, uint32_t stride, GbhActorInfo e)
    {
        const uint32_t fill = stride < sizeof e ? stride : (uint32_t)sizeof e;
        e.struct_size = fill;
        memcpy(dst, &e, fill);
    }

    int All(std::vector<GbhActorInfo>& out)
    {
        std::vector<ActorList::Node> nodes;
        const int total = Collect(nodes);
        if (total < 0) return total;
        const uint32_t gen = Registry::Generation();
        out.resize((size_t)total);
        Ensure();
        EnterCriticalSection(&g_lock);
        for (size_t i = 0; i < nodes.size(); ++i) Entry(nodes[i], gen, out[i]);
        LeaveCriticalSection(&g_lock);
        return total;
    }

    const char* FlagNames(uint32_t flags, char* out, size_t cap)
    {
        static const struct { uint32_t bit; const char* name; } kNames[] = {
            { GBH_ACTOR_CHARACTER, "character" }, { GBH_ACTOR_GHOSTBUSTER, "ghostbuster" }, { GBH_ACTOR_NPC, "npc" },
            { GBH_ACTOR_GHOST, "ghost" }, { GBH_ACTOR_BREAKER, "breaker" }, { GBH_ACTOR_ANIMODEL, "animodel" },
            { GBH_ACTOR_PHYSOBJ, "physobj" },
        };
        out[0] = 0;
        for (const auto& k : kNames)
        {
            if (!(flags & k.bit)) continue;
            if (out[0]) strcat_s(out, cap, " ");
            strcat_s(out, cap, k.name);
        }
        return out[0] ? out : "-";
    }

    const char* State(const GbhActorInfo& e) { return (e.flags & GBH_ACTOR_ENABLED) ? "enabled" : "pool"; }

    // ---- commands --------------------------------------------------------------
    int CmdActors(int argc, const char* const* argv, const char** err, void*)
    {
        const char* filter = argc >= 1 ? argv[0] : nullptr;
        std::vector<GbhActorInfo> all;
        const int total = All(all);
        if (total < 0) { *err = "no level: gGame or its actor chain is unreadable"; return total; }

        int matched = 0, enabled = 0;
        for (const GbhActorInfo& e : all)
        {
            if (filter && !IContains(e.name, filter) && !IContains(e.cls, filter)) continue;
            ++matched;
            if (e.flags & GBH_ACTOR_ENABLED) ++enabled;
        }
        if (filter) Log::Writef("RES", "%d of %d actor(s) match '%s', %d enabled", matched, total, filter, enabled);
        else        Log::Writef("RES", "%d actor(s) in the engine chain, %d enabled", total, enabled);

        int shown = 0;
        for (const GbhActorInfo& e : all)
        {
            if (filter && !IContains(e.name, filter) && !IContains(e.cls, filter)) continue;
            if (shown >= kListCap) { Log::Writef("RES", "  ... cut at %d of %d", kListCap, matched); break; }
            Log::Writef("RES", "  %-24s %-20s %-7s (%.2f, %.2f, %.2f) team %d",
                        e.name, e.cls, State(e), e.pos[0], e.pos[1], e.pos[2], e.team);
            ++shown;
        }
        return GBH_OK;
    }

    int CmdActor(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = "usage: actor <name>   e.g. actor Egon"; return GBH_ERR_ARG; }
        GbhActorInfo e;
        const int rc = Actors::Find(argv[0], &e, sizeof e);
        if (rc == GBH_ERR_NOT_FOUND) { *err = "no actor in the engine chain has that name"; return rc; }
        if (rc == GBH_ERR_STATE)     { *err = "no level: gGame or its actor chain is unreadable"; return rc; }
        if (rc < 0)                  { *err = "lookup failed"; return rc; }

        char flags[96];
        Log::Writef("RES", "%s: %s at %p, %s, generation %u", e.name, e.cls, e.ptr, State(e), e.generation);
        Log::Writef("RES", "  pos (%.3f, %.3f, %.3f)  orient (%.3f, %.3f, %.3f)",
                    e.pos[0], e.pos[1], e.pos[2], e.orient[0], e.orient[1], e.orient[2]);
        Log::Writef("RES", "  team %d, last frame %d, flags 0x%02X: %s",
                    e.team, e.last_frame, e.flags, FlagNames(e.flags, flags, sizeof flags));
        return GBH_OK;
    }
}

namespace Actors
{
    int Snapshot(GbhActorInfo* buf, int cap, uint32_t stride)
    {
        if (!buf || cap <= 0 || stride < sizeof(uint32_t)) return GBH_ERR_ARG;
        std::vector<ActorList::Node> nodes;
        const int total = Collect(nodes);
        if (total < 0) return total;
        const uint32_t gen = Registry::Generation();
        const int n = total < cap ? total : cap;
        Ensure();
        EnterCriticalSection(&g_lock);
        for (int i = 0; i < n; ++i)
        {
            GbhActorInfo e;
            Entry(nodes[(size_t)i], gen, e);
            Emit((GbhActorInfo*)((char*)buf + (size_t)i * stride), stride, e);
        }
        LeaveCriticalSection(&g_lock);
        return n;
    }

    int Count()
    {
        void* game = Game::Singleton();
        if (!game) return GBH_ERR_STATE;
        const int n = ActorList::Walk(g_mem, (uintptr_t)game, nullptr, ActorList::kMaxNodes);
        return n < 0 ? GBH_ERR_STATE : n;
    }

    int Find(const char* name, GbhActorInfo* out, uint32_t stride)
    {
        if (!name || !*name || !out || stride < sizeof(uint32_t)) return GBH_ERR_ARG;
        std::vector<ActorList::Node> nodes;
        const int total = Collect(nodes);
        if (total < 0) return total;

        const ActorList::Node* hit = nullptr;
        for (const ActorList::Node& n : nodes) if (IEquals(n.name, name)) { hit = &n; break; }
        if (!hit) for (const ActorList::Node& n : nodes) if (IContains(n.name, name)) { hit = &n; break; }
        if (!hit) return GBH_ERR_NOT_FOUND;

        const uint32_t gen = Registry::Generation();
        Ensure();
        EnterCriticalSection(&g_lock);
        GbhActorInfo e;
        Entry(*hit, gen, e);
        LeaveCriticalSection(&g_lock);
        Emit(out, stride, e);
        return GBH_OK;
    }

    int IsA(void* actor, const char* cls)
    {
        if (!actor || !cls || !*cls) return GBH_ERR_ARG;
        uintptr_t vtable = 0;
        if (!g_mem.Read((uintptr_t)actor, &vtable, sizeof vtable) || !vtable) return GBH_ERR_STATE;
        Ensure();
        EnterCriticalSection(&g_lock);
        const Rtti::Chain* c = g_cache->Lookup(vtable);
        const int rc = !c ? GBH_ERR_STATE : (c->Has(cls) ? 1 : 0);
        LeaveCriticalSection(&g_lock);
        return rc;
    }

    void RegisterCommands()
    {
        Ensure();
        Commands::Register(nullptr, "actors", CmdActors, nullptr, "[filter] -- the engine's own actor chain, spawn pool included", Commands::kAnyThread);
        Commands::Register(nullptr, "actor",  CmdActor,  nullptr, "<name> -- one actor of the engine chain in full", Commands::kAnyThread);
    }
}
