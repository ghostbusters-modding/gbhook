// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Named actions a player binds to keys. input/BindTable is the table; this is the lock, the settings lookup,
// the fire ring the window procedure fills and the pump drains, the guard and the log lines.

#include "gbhook/gbhook.h"

namespace Bindings
{
    // Creates the table and its lock. Bootstrap calls it before the window is subclassed.
    void Init();

    // `owner` is a mod id, or nullptr for the framework. Each answers a GbhStatus.
    int Register(const char* owner, const char* name, GbhActionFn fn, void* user, const char* help);
    int Binding(const char* owner, const char* name, char* out, int cap);
    int Held(const char* owner, const char* name);
    int Enable(const char* owner, const char* name, bool on);
    int Capture(const char* owner, bool on);

    // Message thread, after Events::FireKey. True when the key is an action's and must not reach the engine.
    bool OnKey(int vk, bool down);
    void OnFocusLost();

    // Main thread, from the pump: runs the presses the window procedure queued.
    void Flush();

    // `binds`: every action, its chord and its help.
    void RegisterCommands();
}
