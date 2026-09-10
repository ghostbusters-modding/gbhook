// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Vtable clones: one copy per original table, slot ownership by name, vptr swaps recorded. tests/registry/test_vtable.cpp is the spec.

#include "Memory.h"

#include <string>
#include <vector>

class VtableRegistry
{
public:
    struct Slot
    {
        int       index;
        uintptr_t fn;         // what the owner installed
        uintptr_t original;   // what the shipped table held
        char      owner[64];
    };

    struct Copy
    {
        uintptr_t         original;   // the shipped table, slot 0
        uintptr_t         clone;      // our copy, slot 0
        int               slots;
        char              owner[64];  // first to ask
        std::vector<Slot> patched;
    };

    struct Applied
    {
        uintptr_t object;
        uintptr_t clone;
        uintptr_t before;     // the vptr the object carried
        char      owner[64];
    };

    static constexpr int kMaxSlots = 4096;

    explicit VtableRegistry(Memory& mem);

    // Copies `slots` entries plus the RTTI locator at [-1], so the copy still identifies as the class.
    // A second request for the same table gets the same copy. Returns the copy's slot 0, or 0 with `why`.
    uintptr_t Clone(const char* owner, uintptr_t original, int slots, std::string* why);

    // Installs `fn` in one slot of a copy. A slot held by another owner is refused by name.
    bool      SetSlot(const char* owner, uintptr_t clone, int slot, uintptr_t fn, uintptr_t* originalFn, std::string* why);

    // The shipped table's entry for a slot of a copy, or 0 when unknown.
    uintptr_t Original(uintptr_t clone, int slot) const;

    // Points `object` at the copy. The object must carry the shipped table or the copy already.
    bool      Apply(const char* owner, uintptr_t object, uintptr_t clone, std::string* why);

    // Re-reads every patched slot and every swapped vptr. Returns the drift count; `what` is "slot" or "vptr".
    int       Verify(void (*onDrift)(const char* what, uintptr_t at, const char* owner, void* ctx), void* ctx);

    int CloneCount() const   { return (int)m_copies.size(); }
    int AppliedCount() const { return (int)m_applied.size(); }

private:
    Memory&              m_mem;
    std::vector<Copy>    m_copies;
    std::vector<Applied> m_applied;

    Copy*       FindCopy(uintptr_t clone);
    const Copy* FindCopy(uintptr_t clone) const;
};
