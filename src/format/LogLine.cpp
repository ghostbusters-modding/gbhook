// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "LogLine.h"

#include <cstdio>

namespace LogLine
{
    size_t Format(char* out, size_t cap, int hour, int minute, int second,
                  const char* tag, const char* id, const char* text)
    {
        if (!out || cap == 0) return 0;
        if (!tag)  tag  = "";
        if (!text) text = "";

        int n = (id && *id)
            ? snprintf(out, cap, "[%02d:%02d:%02d] %-4s [%s] %s", hour, minute, second, tag, id, text)
            : snprintf(out, cap, "[%02d:%02d:%02d] %-4s %s", hour, minute, second, tag, text);

        if (n < 0) { out[0] = '\0'; return 0; }
        return (size_t)n < cap ? (size_t)n : cap - 1;
    }
}
