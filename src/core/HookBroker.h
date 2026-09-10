// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The only MinHook owner in the process, plus the byte-patch and vtable registries, under one integrity sweep.
// docs/HOOKING.md says why one owner, why one detour per address, and what a drift line means.

#include <cstddef>
#include <cstdint>
#include "gbhook/gbhook.h"

namespace HookBroker
{
    // MH_Initialize, once. False means abort: a half-hooked framework is worse than an inert one.
    bool Init();

    // ---- detours ----------------------------------------------------------
    // `owner` is a mod id, or nullptr for the framework. GBH_HOOK_DEFERRED creates without enabling.
    GbhHook Install(const char* owner, void* target, void* detour, void** original, uint32_t flags);
    bool    Enable(GbhHook h);
    bool    Disable(GbhHook h);
    // MH_QueueEnableHook for each, then one MH_ApplyQueued: no window with one detour live and its sibling not.
    bool    EnableBatch(const GbhHook* hooks, int count);
    const char* OwnerOf(void* target);
    int     Count();

    // ---- byte patches -----------------------------------------------------
    // Records the site, writes, re-reads. Refused by name when it overlaps a patch or a detour's prologue.
    GbhPatch PatchWrite(const char* owner, void* at, const void* bytes, size_t n);
    bool     PatchRevert(GbhPatch p);

    // ---- vtable clones ----------------------------------------------------
    // One copy per shipped table, owned by the broker for the process. Slots are claimed by name.
    void* VtableClone(const char* owner, void* original, int slots);
    bool  VtableSlot(const char* owner, void* clone, int slot, void* fn, void** originalFn);
    bool  VtableApply(const char* owner, void* object, void* clone);
    void* VtableOriginal(void* clone, int slot);

    // Re-reads every detour, patch and clone. Logs and returns how many drifted.
    int VerifyAll(const char* why);
}
