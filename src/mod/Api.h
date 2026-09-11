// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The GbhApi table handed to every mod. Built once; each identity-sensitive entry resolves its own caller.

#include "gbhook/gbhook.h"

namespace Api
{
    const GbhApi* Table();
}
