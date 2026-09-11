// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Synthetic keys written into the engine's own scan-code table, the one its WM_KEYDOWN handler fills and the
// game reads. SendInput and DirectInput never reach it. Level semantics: 1 from down to up, like a real key.

namespace InputInject
{
    void SetKey(int dik, bool down);
    void Clear();

    // `key tap|down|up|hold|spam <NAME> [ms]`, `key clear`, `sleep <ms>`. hold, spam and sleep block on purpose.
    void RegisterCommands();
}
