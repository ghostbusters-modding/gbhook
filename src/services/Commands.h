// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// One dispatcher for the framework, every mod and the file channel. cmd/Table is the registry; this is the lock,
// the game-thread routing, the queue the tick and the pump drain, and <gamedir>\gbhook.cmd.

#include "gbhook/gbhook.h"

namespace Commands
{
    enum Flags
    {
        kAnyThread  = 0,
        kGameThread = 1 << 0    // routed through the queue when called from anywhere else
    };

    // `owner` is a mod id, or nullptr for a framework built-in. GbhStatus.
    int  Register(const char* owner, const char* name, GbhCommandFn fn, void* user, const char* help, unsigned flags);

    int  Execute(const char* line);      // now, or queued when the handler needs the game thread and this is not it
    int  ExecuteNow(const char* line);   // always in place
    int  Queue(const char* line);        // always queued, unless this is the game thread already

    // Runs a bounded batch of queued lines. The tick calls it; the pump calls it while the tick is parked.
    void Drain();

    // The file channel, once per loop tick.
    void Poll();

    void Init();     // the framework's own built-ins
    void LogHelp();
}
