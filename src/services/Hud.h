// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The engine's own HUD message line, the one the game uses for its top-of-screen text. Needs a live level.

namespace Hud
{
    // Any thread: off the game thread it travels as a `hud` command and shows on the next frame.
    void Message(const char* text, float seconds);

    // Game thread. False when no level is live or the engine call faulted.
    bool ShowNow(const char* text, float seconds);
}
