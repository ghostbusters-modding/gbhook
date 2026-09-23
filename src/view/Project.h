// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The engine's own world-to-screen, CMini3D::isSphereVisible's math over plain inputs. Pure; tests/view is the spec.

namespace Project
{
    constexpr int kMatrixFloats = 0x34;   // one matrix-stack entry: basis, view translation, eye, projected 4x4

    struct Camera
    {
        const float* m;        // kMatrixFloats floats
        float        scaleX;   // 1 / xFov
        float        scaleY;
        float        nearZ;
        int          width;    // backbuffer pixels
        int          height;
    };

    // 1 on screen. 0 behind the near plane (out untouched) or in front but outside (out filled, for edge arrows).
    // The Y line is the one step not traced in the engine: a vertically mirrored marker means flipping it.
    int ToScreen(const Camera& cam, const float pos[3], float out[2]);
}
