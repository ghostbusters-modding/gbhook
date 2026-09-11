// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// gbhook's own thread: a 16 ms tick that polls the file channel. Bootstrap hands its thread over; it never returns.

namespace Loop
{
    void Run();
}
