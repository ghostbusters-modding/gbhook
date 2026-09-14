// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The engine's intrusive actor chain, walked over Memory so it runs offline. tests/actors/test_actorlist.cpp is the spec.
// The game thread mutates the chain under a reader: every field is a guarded copy and the first bad node ends the walk.

#include "registry/Memory.h"

#include <cstdint>

namespace ActorList
{
    struct Node
    {
        uintptr_t addr;
        uintptr_t vtable;
        char      name[64];     // "?" when the node held no printable name
        uint32_t  cookie;
        bool      enabled;      // the cookie says enabled; clear is the spawn pool, or a torn read
        float     pos[3];
        float     orient[3];
        int32_t   team;
        int32_t   lastFrame;
    };

    constexpr int kMaxNodes = 16384;   // a level carries hundreds; the bound is for a torn chain, not a big one

    // From gGame's head into `out`, at most `cap` (clamped to kMaxNodes); a null `out` only counts. Ends at the tail,
    // the bound, a node seen before or the first unreadable one. The count, or -1 when the head itself is unreadable.
    int Walk(Memory& mem, uintptr_t game, Node* out, int cap);
}
