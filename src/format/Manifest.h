// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Checks on a GbhManifest read out of a DLL. tests/format/test_manifest.cpp is the spec.

#include "gbhook/gbhook.h"

#include <string>
#include <vector>

namespace Manifest
{
    // The refusal, or empty when this framework can load the binary.
    std::string Validate(const GbhManifest& m);

    // A fixed char array that is not required to be terminated when it fills its slot.
    std::string Field(const char* p, size_t cap);

    std::vector<std::string> ExclusiveHooks(const GbhManifest& m);
}
