// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "VtableRegistry.h"

#include <cstdio>
#include <cstring>

namespace
{
    constexpr uintptr_t W = sizeof(uintptr_t);

    void CopyOwner(char* dst, size_t cap, const char* owner)
    {
        const char* who = (owner && *owner) ? owner : "gbhook";
        size_t n = strlen(who);
        if (n >= cap) n = cap - 1;
        memcpy(dst, who, n);
        dst[n] = '\0';
    }

    std::string Hex(uintptr_t v)
    {
        char b[32];
        snprintf(b, sizeof b, "%llX", (unsigned long long)v);
        return b;
    }
}

VtableRegistry::VtableRegistry(Memory& mem) : m_mem(mem) {}

VtableRegistry::Copy* VtableRegistry::FindCopy(uintptr_t clone)
{
    for (Copy& c : m_copies) if (c.clone == clone) return &c;
    return nullptr;
}

const VtableRegistry::Copy* VtableRegistry::FindCopy(uintptr_t clone) const
{
    for (const Copy& c : m_copies) if (c.clone == clone) return &c;
    return nullptr;
}

uintptr_t VtableRegistry::Clone(const char* owner, uintptr_t original, int slots, std::string* why)
{
    std::string sink;
    if (!why) why = &sink;

    if (!original || slots <= 0 || slots > kMaxSlots) { *why = "bad argument"; return 0; }

    for (const Copy& c : m_copies)
    {
        if (c.original != original) continue;
        if (slots > c.slots)
        {
            *why = "already cloned with " + std::to_string(c.slots) + " slots by '" + c.owner + "'";
            return 0;
        }
        why->clear();
        return c.clone;
    }

    // The locator at [-1] rides along, so the copy still identifies as the class.
    const size_t bytes = (size_t)(slots + 1) * W;
    std::vector<uint8_t> buf(bytes);
    if (!m_mem.Read(original - W, buf.data(), bytes)) { *why = "table is not readable"; return 0; }

    void* copy = m_mem.Alloc(bytes);
    if (!copy) { *why = "could not alloc the copy"; return 0; }
    const uintptr_t base = (uintptr_t)copy;
    if (!m_mem.Write(base, buf.data(), bytes)) { *why = "copy is not writable"; return 0; }

    Copy c;
    c.original = original;
    c.clone    = base + W;
    c.slots    = slots;
    CopyOwner(c.owner, sizeof c.owner, owner);
    m_copies.push_back(c);
    why->clear();
    return c.clone;
}

bool VtableRegistry::SetSlot(const char* owner, uintptr_t clone, int slot, uintptr_t fn,
                             uintptr_t* originalFn, std::string* why)
{
    std::string sink;
    if (!why) why = &sink;

    Copy* c = FindCopy(clone);
    if (!c) { *why = "not a copy this registry made"; return false; }
    if (slot < 0 || slot >= c->slots || !fn) { *why = "bad argument"; return false; }

    char who[64];
    CopyOwner(who, sizeof who, owner);

    Slot* s = nullptr;
    for (Slot& x : c->patched) if (x.index == slot) s = &x;
    if (s && strcmp(s->owner, who) != 0)
    {
        *why = "slot " + std::to_string(slot) + " is held by '" + s->owner + "'";
        return false;
    }

    const uintptr_t at = clone + (uintptr_t)slot * W;
    uintptr_t orig = 0;
    if (s) orig = s->original;
    else if (!m_mem.Read(at, &orig, sizeof orig)) { *why = "copy is not readable"; return false; }

    if (!m_mem.Write(at, &fn, sizeof fn)) { *why = "copy is not writable"; return false; }

    if (!s)
    {
        Slot n;
        n.index    = slot;
        n.fn       = 0;
        n.original = orig;
        memcpy(n.owner, who, sizeof n.owner);
        c->patched.push_back(n);
        s = &c->patched.back();
    }
    s->fn = fn;
    if (originalFn) *originalFn = orig;
    why->clear();
    return true;
}

uintptr_t VtableRegistry::Original(uintptr_t clone, int slot) const
{
    const Copy* c = FindCopy(clone);
    if (!c || slot < 0 || slot >= c->slots) return 0;
    for (const Slot& s : c->patched) if (s.index == slot) return s.original;
    uintptr_t v = 0;
    if (!m_mem.Read(c->original + (uintptr_t)slot * W, &v, sizeof v)) return 0;
    return v;
}

bool VtableRegistry::Apply(const char* owner, uintptr_t object, uintptr_t clone, std::string* why)
{
    std::string sink;
    if (!why) why = &sink;

    if (!object) { *why = "bad argument"; return false; }
    const Copy* c = FindCopy(clone);
    if (!c) { *why = "not a copy this registry made"; return false; }

    uintptr_t vptr = 0;
    if (!m_mem.Read(object, &vptr, sizeof vptr)) { *why = "object is not readable"; return false; }

    for (const Applied& a : m_applied)
        if (a.object == object && a.clone == clone && vptr == clone) { why->clear(); return true; }

    if (vptr != c->original && vptr != clone)
    {
        *why = "object carries vtable 0x" + Hex(vptr) + ", not the table this copy came from";
        return false;
    }
    if (vptr != clone && !m_mem.Write(object, &clone, sizeof clone))
    {
        *why = "object is not writable";
        return false;
    }

    Applied a;
    a.object = object;
    a.clone  = clone;
    a.before = vptr;
    CopyOwner(a.owner, sizeof a.owner, owner);
    m_applied.push_back(a);
    why->clear();
    return true;
}

int VtableRegistry::Verify(void (*onDrift)(const char* what, uintptr_t at, const char* owner, void* ctx), void* ctx)
{
    int drift = 0;
    for (const Copy& c : m_copies)
        for (const Slot& s : c.patched)
        {
            const uintptr_t at = c.clone + (uintptr_t)s.index * W;
            uintptr_t now = 0;
            if (!m_mem.Read(at, &now, sizeof now) || now != s.fn)
            {
                ++drift;
                if (onDrift) onDrift("slot", at, s.owner, ctx);
            }
        }
    for (const Applied& a : m_applied)
    {
        uintptr_t now = 0;
        if (!m_mem.Read(a.object, &now, sizeof now) || now != a.clone)
        {
            ++drift;
            if (onDrift) onDrift("vptr", a.object, a.owner, ctx);
        }
    }
    return drift;
}
