// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Byte patches, recorded and arbitrated. tests/registry/test_patch.cpp is the spec; HookBroker owns the one instance.

#include "Memory.h"
#include "gbhook/gbhook.h"

#include <string>
#include <vector>

class PatchRegistry
{
public:
    struct Patch
    {
        uintptr_t at;
        size_t    n;
        char      owner[64];
        uint8_t   original[GBH_MAX_PATCH_BYTES];
        uint8_t   written[GBH_MAX_PATCH_BYTES];
        bool      live;
    };

    explicit PatchRegistry(Memory& mem);

    // Records the bytes at `at`, writes `n` new ones, reads them back. Returns a 1-based handle, or 0 with `why`.
    int  Write(const char* owner, uintptr_t at, const void* bytes, size_t n, std::string* why);

    // Restores the original bytes. Refused when the site no longer holds what was written.
    bool Revert(int handle, std::string* why);

    const Patch* Get(int handle) const;

    // The live patch covering any byte of [at, at + n), or nullptr.
    const Patch* Overlapping(uintptr_t at, size_t n) const;

    // Re-reads every live patch. Returns how many differ from what was written; each goes to `onDrift`.
    int  Verify(void (*onDrift)(const Patch& p, const uint8_t* now, void* ctx), void* ctx);

    int  Count() const;   // live patches

private:
    Memory&            m_mem;
    std::vector<Patch> m_patches;
};
