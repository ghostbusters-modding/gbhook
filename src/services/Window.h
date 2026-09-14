// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The game window, subclassed once, on the main thread that created it. Keys and characters fan out on the key
// and char buses before the engine's own handler sees them; a subscriber answering 1 keeps the message.

namespace Window
{
    // Parked on the pump: finds the main thread's top-level window once it exists and subclasses it.
    void Install();
    bool Installed();
    void* Handle();   // HWND, null until installed
}
