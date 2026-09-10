// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Process memory behind an interface, so the registries run offline against plain buffers.

#include <cstddef>
#include <cstdint>

struct Memory
{
    virtual ~Memory() = default;
    virtual bool  Read(uintptr_t at, void* out, size_t n) = 0;
    virtual bool  Write(uintptr_t at, const void* in, size_t n) = 0;
    // Lives for the process. A freed vtable under a live object is a use-after-free.
    virtual void* Alloc(size_t n) = 0;
};
