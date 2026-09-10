// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The log line grammar, kept greppable: "[hh:mm:ss] TAG  text" and "[hh:mm:ss] TAG  [mod.id] text".

#include <cstddef>

namespace LogLine
{
    // Writes one line, no newline, into out[cap], truncating. Returns the length written.
    size_t Format(char* out, size_t cap, int hour, int minute, int second,
                  const char* tag, const char* id, const char* text);
}
