// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Hud.h"
#include "Commands.h"
#include "FrameHook.h"
#include "Game.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "gb/Offsets.h"

#include <cstdio>
#include <string>

namespace
{
    constexpr int kTopLine = 11;   // the message id the game's own scripts use for the top HUD line

    typedef void (*tDisplayMessage)(int, const char*, float);

    bool Call(const char* text, float seconds)
    {
        GBH_SEH_TRY { ((tDisplayMessage)(gameBase + Offsets::DisplayText))(kTopLine, text, seconds); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
}

namespace Hud
{
    bool ShowNow(const char* text, float seconds)
    {
        if (!text || !gameBase) return false;
        if (!Game::LocalPlayer()) { Log::Writef("HUD", "no level is live; \"%s\" dropped", text); return false; }
        if (seconds <= 0) seconds = 3.0f;
        if (!Call(text, seconds)) { Log::Write("HUD", "the engine's message call faulted"); return false; }
        return true;
    }

    void Message(const char* text, float seconds)
    {
        if (!text) return;
        if (FrameHook::IsGameThread()) { ShowNow(text, seconds); return; }

        // A double quote would end the word early on the way through the tokenizer.
        std::string t(text);
        for (char& c : t) if (c == '"') c = '\'';
        char line[640];
        _snprintf_s(line, sizeof line, _TRUNCATE, "hud %.2f \"%s\"", seconds, t.c_str());
        Commands::Queue(line);
    }
}
