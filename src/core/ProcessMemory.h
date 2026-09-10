// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The live process behind the Memory interface: guarded reads, unprotected writes, the framework heap.

#include "registry/Memory.h"

struct ProcessMemory : Memory
{
    bool  Read(uintptr_t at, void* out, size_t n) override;
    bool  Write(uintptr_t at, const void* in, size_t n) override;
    void* Alloc(size_t n) override;
};
