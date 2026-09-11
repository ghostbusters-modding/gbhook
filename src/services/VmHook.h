// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The Dante VM's global-registration detour, hooked once: feeds the registry and the actor bus.

namespace VmHook
{
    bool Install();
}
