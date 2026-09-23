// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Project.h"

namespace Project
{
    int ToScreen(const Camera& c, const float pos[3], float out[2])
    {
        const float* m = c.m;
        const float dx = pos[0] - m[0x10], dy = pos[1] - m[0x11], dz = pos[2] - m[0x12];
        const float vz = dx * m[2] + dy * m[6] + dz * m[10];
        if (!(vz > c.nearZ)) return 0;
        const float vx = (dx * m[0] + dy * m[4] + dz * m[8]) * c.scaleX;
        const float vy = (dx * m[1] + dy * m[5] + dz * m[9]) * c.scaleY;
        const float nx = vx / vz, ny = vy / vz;
        out[0] = (nx * 0.5f + 0.5f) * (float)c.width;
        out[1] = (0.5f - ny * 0.5f) * (float)c.height;
        return (nx >= -1.0f && nx <= 1.0f && ny >= -1.0f && ny <= 1.0f) ? 1 : 0;
    }
}
