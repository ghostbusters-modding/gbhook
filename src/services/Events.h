// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The typed event buses: the contended detours hooked once and fanned out under a per-callback guard.
// bus/Bus is the table; this is the lock, the guard, the clock and the log lines around it.

#include "gbhook/gbhook.h"

namespace Events
{
    enum Kind { Frame = 0, Pump, Level, Actor, Key, Char, KindCount };

    // Creates the tables and their lock. Bootstrap calls it before any detour that fires a bus can install.
    void Init();

    // `owner` is a mod id, or nullptr for the framework. `fn` is the typedef for `kind`.
    GbhSub Subscribe(Kind kind, const char* owner, void* fn, void* user);
    void   Unsubscribe(GbhSub s);
    void   DisableOwner(const char* owner);

    // Dispatch, called only by the framework's own detours.
    void FireFrame();                                              // game thread
    void FirePump();                                               // main thread, front end and level
    void FireLevel(GbhLevelPhase phase, const char* level, bool ok);
    void FireActor(const char* cls, const char* name, void* ptr);
    // Message thread. True when a subscriber kept the message; the fan-out stops at the first one.
    bool FireKey(int vk, bool down);
    bool FireChar(unsigned int ch);

    int  LiveCount();
    void LogSummary();
}
