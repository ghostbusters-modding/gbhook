// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// A vectored handler that names every first-chance fault by module and offset, once per site, then lets it continue.

namespace FaultLogger
{
    void Install();
}
