// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// A small code mod: a setting, a command, two events and a HUD line. The shape every mod starts from.

#include <windows.h>
#include <string>

#include "gbhook/gbhook.h"
#include "gbhook/gbhook.hpp"

GBHOOK_PLUGIN("gb.mymod");

namespace
{
    int g_frames = 0;

    // gb.mymod.hello [words...]: logs its arguments. Handlers run on the game thread.
    int CmdHello(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = "usage: hello <words>"; return GBH_ERR_ARG; }
        std::string words;
        for (int i = 0; i < argc; ++i) { if (i) words += ' '; words += argv[i]; }
        gbh::logf("RES", "%s, %s", gbh::setting("greeting", "hello").c_str(), words.c_str());
        return GBH_OK;
    }

    void OnLevel(int phase, const char* level, int ok, void*)
    {
        if (phase == GBH_LEVEL_PREPARE_END) gbh::logf("LEVEL", "'%s' prepared, ok=%d", level, ok);
    }

    // Once per frame while a level is live. Keep it short: every mod shares this budget.
    void OnFrame(void*)
    {
        if (++g_frames == 1) gbh::hud("hello from gb.mymod", 4.0f);
        if (g_frames % 600 == 0) gbh::logf("EVT", "%d frames in '%s'", g_frames, gbh::level_name().c_str());
    }
}

extern "C" GBHOOK_EXPORT int GbhPluginInit(const GbhApi* api)
{
    if (!api || api->abi_version != GBHOOK_ABI_VERSION) return GBH_ERR;
    gbh::bind(api);

    gbh::logf("MOD", "greeting is \"%s\"", gbh::setting("greeting", "hello").c_str());
    gbh::command_register("hello", CmdHello, nullptr, "<words> -- log a greeting");
    gbh::on_level(OnLevel).release();
    gbh::on_frame(OnFrame).release();
    return GBH_OK;
}
