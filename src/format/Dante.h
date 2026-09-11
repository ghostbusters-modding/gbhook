// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The compiled script container, world\<level>.dante, read for what the front end needs from it. Text sections,
// fixed order. Pure; tests/format/test_dante.cpp is the spec.

#include <string>
#include <vector>

namespace Dante
{
    // The checkpoints a level registers, "checkpoint_Start", in script order. A checkpoint_ export named only in
    // EXPORTS is a helper thread that places nobody, so only the STRINGS section counts.
    std::vector<std::string> Checkpoints(const char* text, size_t n);
}
