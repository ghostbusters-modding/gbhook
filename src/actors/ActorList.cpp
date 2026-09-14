// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ActorList.h"
#include "gb/Structs/CActor.h"
#include "gb/Structs/CActorBase.h"
#include "gb/Structs/CGame.h"

#include <cstring>
#include <vector>

namespace
{
    // One read covers every field the walk uses; a node that cannot give all of them ends the walk.
    constexpr size_t kNodeBytes = CActor::team + sizeof(int32_t);
    constexpr size_t kNameCap   = CActorBase::cookie - CActor::name;

    template <typename T> T Field(const unsigned char* raw, uintptr_t off)
    {
        T v;
        memcpy(&v, raw + off, sizeof v);
        return v;
    }

    // Printable up to the first NUL or the cookie, else "?": the field is fixed-size and may be unterminated.
    void CopyName(const unsigned char* raw, char* out, size_t cap)
    {
        size_t n = 0;
        while (n < kNameCap && n + 1 < cap)
        {
            const unsigned char c = raw[CActor::name + n];
            if (c == 0) break;
            if (c < 0x20 || c > 0x7E) { n = 0; break; }
            out[n++] = (char)c;
        }
        if (n == 0) { out[0] = '?'; out[1] = 0; return; }
        out[n] = 0;
    }

    void Fill(ActorList::Node& e, uintptr_t addr, const unsigned char* raw)
    {
        memset(&e, 0, sizeof e);
        e.addr   = addr;
        e.vtable = Field<uintptr_t>(raw, 0);
        CopyName(raw, e.name, sizeof e.name);
        e.cookie  = Field<uint32_t>(raw, CActorBase::cookie);
        e.enabled = e.cookie == CActorBase::cookieEnabled;
        memcpy(e.pos,    raw + CActorBase::pos,    sizeof e.pos);
        memcpy(e.orient, raw + CActorBase::orient, sizeof e.orient);
        e.team      = Field<int32_t>(raw, CActor::team);
        e.lastFrame = Field<int32_t>(raw, CActorBase::lastFrame);
    }

    // Open addressing over the addresses walked so far. Twice the bound and a power of two, so it never fills.
    struct Seen
    {
        std::vector<uintptr_t> slots;
        Seen() : slots(2 * ActorList::kMaxNodes, 0) {}

        bool Insert(uintptr_t a)
        {
            const size_t mask = slots.size() - 1;
            size_t i = (a >> 4) & mask;
            while (slots[i])
            {
                if (slots[i] == a) return false;
                i = (i + 1) & mask;
            }
            slots[i] = a;
            return true;
        }
    };
}

namespace ActorList
{
    int Walk(Memory& mem, uintptr_t game, Node* out, int cap)
    {
        if (!game || cap < 0) return -1;
        if (cap > kMaxNodes) cap = kMaxNodes;

        uintptr_t node = 0;
        if (!mem.Read(game + CGame::actorListHead, &node, sizeof node)) return -1;

        Seen seen;
        unsigned char raw[kNodeBytes];
        int n = 0;
        while (node && n < cap)
        {
            if (!seen.Insert(node)) break;
            if (!mem.Read(node, raw, sizeof raw)) break;
            if (out) Fill(out[n], node, raw);
            ++n;
            node = Field<uintptr_t>(raw, CActor::next);
        }
        return n;
    }
}
