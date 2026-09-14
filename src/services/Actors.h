// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The engine's own actor list, gGame's intrusive chain of every CActor in the level. Guarded reads, any thread;
// actors/ is the pure walk and the RTTI decode, this is the process memory behind them.

#include "gbhook/gbhook.h"

#include <cstdint>

namespace Actors
{
    // Copies up to `cap` entries at `stride` bytes each, filling min(stride, sizeof) of each. The count, or negative.
    int Snapshot(GbhActorInfo* buf, int cap, uint32_t stride);
    int Count();

    // Exact, then substring, case-insensitive, on the engine-list name. GbhStatus.
    int Find(const char* name, GbhActorInfo* out, uint32_t stride);

    // 1 when the RTTI chain carries `cls`, 0 when not, GBH_ERR_STATE when the object is unreadable.
    int IsA(void* actor, const char* cls);

    // `actors [filter]`: the list, one line each; `actor <name>`: one entry in full.
    void RegisterCommands();
}
