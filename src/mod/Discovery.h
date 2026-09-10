// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Walks every configured mods root, gathers what each folder carries, and hands the set to modset/ModSet.
// Nothing found here runs: the judgement is made, and logged, while every party is inert.

#include "modset/ModSet.h"

#include <string>
#include <vector>

namespace Mods
{
    // Once, at boot. Logs every folder's verdict, every conflict, and the resolved code order.
    void Scan();

    const ModSet::Result&           Result();
    const std::vector<std::string>& MissingRoots();

    // <root>\<folder>\gbhook, no trailing separator.
    std::string GbhookDir(const ModSet::Record& r);
}
