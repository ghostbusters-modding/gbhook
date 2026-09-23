// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for view/Project: a hand-built camera at the origin looking down +z.

#include "check.h"
#include "view/Project.h"

int main()
{
    float m[Project::kMatrixFloats] = {};
    m[0] = m[5] = m[10] = 1.0f;
    const Project::Camera cam = { m, 1.0f, 1.0f, 1.0f, 1920, 1080 };
    float out[2];

    const float centre[3] = { 0, 0, 10 };
    CHECK_EQ(Project::ToScreen(cam, centre, out), 1);
    CHECK_EQ(out[0], 960.0f);
    CHECK_EQ(out[1], 540.0f);

    const float right[3] = { 10, 0, 10 };
    CHECK_EQ(Project::ToScreen(cam, right, out), 1);
    CHECK_EQ(out[0], 1920.0f);
    CHECK_EQ(out[1], 540.0f);

    // Up in the world is up on screen, under the note's Y sign.
    const float up[3] = { 0, 5, 10 };
    CHECK_EQ(Project::ToScreen(cam, up, out), 1);
    CHECK_EQ(out[1], 270.0f);

    // In front but outside: 0, and out still filled.
    const float wide[3] = { 30, 0, 10 };
    CHECK_EQ(Project::ToScreen(cam, wide, out), 0);
    CHECK_EQ(out[0], 3840.0f);

    // Behind the near plane: 0, and out untouched.
    out[0] = out[1] = -7.0f;
    const float close[3] = { 0, 0, 0.5f };
    CHECK_EQ(Project::ToScreen(cam, close, out), 0);
    CHECK_EQ(out[0], -7.0f);
    const float behind[3] = { 0, 0, -10 };
    CHECK_EQ(Project::ToScreen(cam, behind, out), 0);
    CHECK_EQ(out[1], -7.0f);

    // The eye and the scales: a camera moved to z = -10 with a 2x zoom sees (5,0,0) at the right edge.
    m[0x12] = -10.0f;
    const Project::Camera zoom = { m, 2.0f, 2.0f, 1.0f, 1920, 1080 };
    const float five[3] = { 5, 0, 0 };
    CHECK_EQ(Project::ToScreen(zoom, five, out), 1);
    CHECK_EQ(out[0], 1920.0f);

    return check::Done("project");
}
