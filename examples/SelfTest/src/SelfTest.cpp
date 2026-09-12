// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The ABI's in-game self-test: every group of the table is exercised against this module's own memory.
// A PASS or FAIL line per check lands in gbhook.log under TEST. Nothing here touches the engine.

#include <windows.h>
#include <cstdint>
#include <cstring>
#include <vector>

#include "gbhook/gbhook.h"
#include "gbhook/gbhook.hpp"

GBHOOK_PLUGIN("gb.selftest");

namespace
{
    int g_pass = 0, g_fail = 0;

    void Check(bool ok, const char* what)
    {
        (ok ? g_pass : g_fail)++;
        gbh::logf("TEST", "%s %s", ok ? "PASS" : "FAIL", what);
    }

    // ---- environment, memory, settings, introspection ----
    void TestBasics()
    {
        const GbhApi* a = gbh::api();
        Check(a->struct_size == sizeof(GbhApi) && a->abi_version == GBHOOK_ABI_VERSION, "table size and ABI version");
        Check(a->game_base() != nullptr, "game_base");
        gbh::logf("TEST", "framework %u.%u.%u, game_dir %s", a->framework_version() >> 16,
                  (a->framework_version() >> 8) & 0xFF, a->framework_version() & 0xFF, gbh::game_dir().c_str());
        gbh::logf("TEST", "mod_dir %s", gbh::mod_dir().c_str());
        Check(gbh::mod_dir().find("SelfTest") != std::string::npos, "mod_dir names this mod's folder");

        void* p = a->alloc(16);
        Check(p != nullptr, "alloc");
        if (p) memset(p, 0x5A, 16);
        p = a->realloc(p, 64);
        Check(p != nullptr && ((uint8_t*)p)[15] == 0x5A, "realloc keeps the bytes");
        a->free(p);

        gbh::logf("TEST", "setting greeting = \"%s\"", gbh::setting("greeting", "?").c_str());
        Check(gbh::setting("greeting", "?") != "?", "setting reads the mod.ini default");
        Check(gbh::setting("nonexistent", "dflt") == "dflt", "setting falls back to the caller's default");
        Check(gbh::setting_int("nonexistent", 42) == 42 && gbh::setting_bool("nonexistent", true), "typed defaults");

        const int n = a->mod_count();
        Check(n >= 1, "mod_count");
        bool self = false;
        for (int i = 0; i < n; ++i)
        {
            const char* id = a->mod_id_at(i);
            gbh::logf("TEST", "mod %d: %s (%s)", i, id ? id : "?", a->mod_is_loaded(id) ? "loaded" : "not loaded");
            if (id && strcmp(id, "gb.selftest") == 0) self = true;
        }
        Check(self, "mod_id_at lists this mod");
        Check(a->mod_is_loaded("gb.selftest") == 0, "mod_is_loaded is still false during our own init");
    }

    // ---- hooks: a function of our own, detoured through the framework's MinHook ----
    __declspec(noinline) int Target(int x)
    {
        volatile int v = x;   // keeps the body real, so there is a prologue to relocate
        return v * 3 + 1;
    }
    typedef int (*tTarget)(int);
    tTarget oTarget = nullptr;
    int Detour(int x) { return oTarget ? oTarget(x) + 1000 : -1; }

    void TestHooks()
    {
        GbhHook h = gbh::hook((void*)&Target, (void*)&Detour, (void**)&oTarget, GBH_HOOK_DEFERRED | GBH_HOOK_EXCLUSIVE);
        Check(h != nullptr, "hook_create deferred");
        Check(Target(2) == 7, "deferred hook is not live");
        Check(gbh::enable_batch(&h, 1), "hook_enable_batch");
        Check(Target(2) == 1007, "detour runs and reaches the original");
        Check(gbh::hook((void*)&Target, (void*)&Detour, (void**)&oTarget) == nullptr, "second hook on one target refused");
        Check(gbh::api()->hook_disable(h) == GBH_OK, "hook_disable");
        Check(Target(2) == 7, "disabled hook is inert");
        Check(gbh::api()->hook_enable(h) == GBH_OK, "hook_enable");
        Check(Target(2) == 1007, "re-enabled hook runs");
    }

    // ---- patches: bytes in our own data ----
    uint8_t g_bytes[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    void TestPatches()
    {
        const uint8_t nu[2] = { 0xAA, 0xBB };
        {
            gbh::Patch p(g_bytes, nu, 2);
            Check(p.ok(), "patch_write");
            Check(g_bytes[0] == 0xAA && g_bytes[1] == 0xBB && g_bytes[2] == 3, "patched bytes read back");
        }
        Check(g_bytes[0] == 1 && g_bytes[1] == 2, "patch_revert restored the original bytes");
        Check(gbh::api()->patch_write(g_bytes, nu, GBH_MAX_PATCH_BYTES + 1) == nullptr, "oversize patch refused");
    }

    // ---- vtables: a three-slot table of our own, with the locator slot the clone copies at [-1] ----
    struct Obj { void** vt; int x; };
    int Slot0(Obj* o)         { return o->x; }
    int Slot1(Obj* o)         { return o->x * 2; }
    int Slot2(Obj* o)         { return o->x * 3; }
    int Slot1Override(Obj* o) { return 100 + o->x; }
    void* g_table[4] = { nullptr, (void*)&Slot0, (void*)&Slot1, (void*)&Slot2 };

    void TestVtables()
    {
        typedef int (*tSlot)(Obj*);
        gbh::VtableOverride vt(&g_table[1], 3);
        Check(vt.ok(), "vtable_clone");
        tSlot orig = nullptr;
        Check(vt.set<tSlot>(1, &Slot1Override, &orig), "vtable_slot");
        Check(orig == &Slot1, "vtable_slot hands back the shipped entry");
        Check(vt.original<tSlot>(1) == &Slot1, "vtable_original");
        // Static: the sweep re-verifies this object's vptr for the rest of the process.
        static Obj o = { &g_table[1], 7 };
        Check(vt.apply(&o), "vtable_apply");
        Check(o.vt == vt.clone(), "object now carries the copy");
        Check(((tSlot)o.vt[0])(&o) == 7 && ((tSlot)o.vt[1])(&o) == 107 && ((tSlot)o.vt[2])(&o) == 21,
              "calls through the copy: one slot overridden, two untouched");
        Check(gbh::api()->vtable_clone(&g_table[1], 3) == vt.clone(), "a second clone request gets the same copy");
    }

    // ---- events: subscribed at init, proven once a level runs ----
    int  g_frames = 0, g_pumps = 0, g_actors = 0, g_helloCalls = 0;
    bool g_frameChecked = false;

    void OnPump(void*)
    {
        if (++g_pumps == 1) gbh::log("EVT", "first pump tick");
    }

    void OnFrame(void*)
    {
        ++g_frames;
        if (!g_frameChecked)
        {
            g_frameChecked = true;
            Check(gbh::is_game_thread(), "on_frame runs on the game thread");
            Check(gbh::api()->game_thread_id() != 0, "game_thread_id is set by the first frame");
            Check(!gbh::level_name().empty(), "level_name is set inside a level");
            gbh::logf("EVT", "first frame, level '%s', local player %p", gbh::level_name().c_str(), gbh::local_player());
            const std::vector<std::string> lvls = gbh::files("world", "*.lvl");
            Check(!lvls.empty(), "file_list finds world\\*.lvl");
            const std::string me = "world\\" + gbh::level_name() + ".lvl";
            Check(!gbh::file_read(me.c_str()).empty(), "file_read reads the live level's .lvl");
        }
        if (g_frames % 600 == 0) gbh::logf("EVT", "%d frames", g_frames);
    }

    void OnActor(const char* cls, const char* name, void* ptr, void*)
    {
        if (cls[0] == 'C' && name[0]) ++g_actors;
        if (g_actors <= 3) gbh::logf("EVT", "actor %s '%s' at %p", cls, name, ptr);
    }

    void OnLevel(int phase, const char* level, int ok, void*)
    {
        if (phase == GBH_LEVEL_PREPARE_BEGIN)
        {
            g_actors = 0;
            gbh::logf("EVT", "prepare begin '%s'", level);
            return;
        }
        gbh::logf("EVT", "prepare end '%s' ok=%d, %d registrations seen", level, ok, g_actors);
        std::vector<GbhRegistryEntry> reg = gbh::registry();
        Check((int)reg.size() == g_actors, "registry_snapshot count matches on_actor_registered");
        GbhRegistryEntry e;
        Check(!reg.empty() && gbh::registry_find(reg[0].name, e) && e.ptr == reg[0].ptr, "registry_find finds the first entry");
        Check(g_helloCalls == 2, "the two commands queued at init ran on the pump before the level");
        gbh::logf("TEST", "%d passed, %d failed so far", g_pass, g_fail);
    }

    // ---- commands: registered as gb.selftest.hello, run and queued from init, drained by the pump ----
    int CmdHello(int argc, const char* const* argv, const char** err, void*)
    {
        ++g_helloCalls;
        std::string args;
        for (int i = 0; i < argc; ++i) { if (i) args += ' '; args += argv[i]; }
        gbh::logf("CMD", "hello #%d on the %s thread: %s", g_helloCalls, gbh::is_game_thread() ? "game" : "main", args.c_str());
        if (argc >= 1 && strcmp(argv[0], "fail") == 0) { *err = "asked to fail"; return GBH_ERR; }
        return GBH_OK;
    }

    void TestCommands()
    {
        Check(gbh::command_register("hello", CmdHello, nullptr, "log the arguments; `fail` returns an error") == GBH_OK, "command_register");
        Check(gbh::command_register("hello", CmdHello) == GBH_ERR_CONFLICT, "duplicate command refused");
        Check(gbh::run("gb.selftest.hello from init") == GBH_OK, "command_run queues a game-thread handler from init");
        Check(gbh::queue("gb.selftest.hello queued") == GBH_OK, "command_queue");
        Check(gbh::run("no.such.command") == GBH_ERR_NOT_FOUND, "unknown command reported");
        Check(gbh::dik("W") == 0x11 && gbh::dik("nope") == -1, "input_dik_from_name");
    }

    // ---- the native front end: the free slot claimed and relabelled; examples/MenuDemo is the page ----
    void PageBuild(void*) { gbh::native_submenu_add_row("Close", 1); }
    int  PageActivate(int, void*) { return GBH_NATIVE_CLOSE; }

    void OnRow(int row, void*)
    {
        gbh::logf("MENU", "row %d activated", row);
        GbhNativeMenuDesc d = { sizeof d, PageBuild, PageActivate, nullptr, "SelfTest" };
        Check(gbh::native_submenu_open(d) == GBH_OK, "native_submenu_open");
    }

    void TestNativeMenu()
    {
        Check(gbh::native_row_claim(GBH_ROW_FREE, OnRow) == GBH_OK, "native_row_claim on the free slot");
        Check(gbh::native_row_claim(GBH_ROW_FREE, OnRow) == GBH_ERR_CONFLICT, "second claim on the slot refused");
        Check(gbh::native_row_claim(0, OnRow) == GBH_ERR_CONFLICT, "claim on the game's Career slot refused");
        Check(gbh::native_row_label(GBH_ROW_FREE, "SelfTest") == GBH_OK, "native_row_label");
        Check(gbh::native_row_label(0, "x") != GBH_OK, "relabelling a row that is not ours refused");
    }

    // Faults on purpose, once: the guard must name this mod, drop only this subscription, and let the game run.
    void FaultOnce(void*)
    {
        volatile int* p = nullptr;
        *p = 1;
    }

    void SubscribeEvents()
    {
        Check(gbh::api()->game_thread_id() == 0, "game_thread_id is 0 before the first frame");
        gbh::on_pump(OnPump).release();
        gbh::on_frame(OnFrame).release();
        gbh::on_actor_registered(OnActor).release();
        gbh::on_level(OnLevel).release();
        if (gbh::setting_bool("selftest.fault", true)) gbh::on_frame(FaultOnce).release();

        gbh::Sub tmp = gbh::on_pump(OnPump);
        Check(tmp.active(), "on_pump subscribes");
        tmp.reset();
        gbh::api()->unsubscribe(nullptr);
        Check(true, "unsubscribe of a dropped or null handle is harmless");
    }
}

extern "C" GBHOOK_EXPORT int GbhPluginInit(const GbhApi* api)
{
    if (!api || api->abi_version != GBHOOK_ABI_VERSION) return GBH_ERR;
    gbh::bind(api);
    gbh::log("MOD", "hello from gb.selftest");

    TestBasics();
    TestHooks();
    TestPatches();
    TestVtables();
    TestCommands();
    TestNativeMenu();
    SubscribeEvents();
    gbh::logf("TEST", "%d passed, %d failed at init", g_pass, g_fail);

    if (gbh::setting_bool("selftest.fail_init", false))
    {
        gbh::log("TEST", "returning GBH_ERR on purpose (selftest.fail_init = 1)");
        return GBH_ERR;
    }
    return GBH_OK;
}
