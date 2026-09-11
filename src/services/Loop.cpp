// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Loop.h"
#include "Commands.h"
#include "../core/Framework.h"

#include <windows.h>

namespace Loop
{
    void Run()
    {
        Log::Write("BOOT", "loop thread running (gbhook.cmd is polled)");
        for (;;)
        {
            Commands::Poll();
            Sleep(16);
        }
    }
}
