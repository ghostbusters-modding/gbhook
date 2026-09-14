// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for actors/ActorList: the chain walk over a fake gGame, its bound, cycles and unreadable nodes.

#include "check.h"
#include "actors/ActorList.h"
#include "gb/Structs/CActor.h"
#include "gb/Structs/CActorBase.h"
#include "gb/Structs/CGame.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
    // A byte buffer standing in for the process: reads outside it fail, like an unmapped page.
    struct BufferMemory : Memory
    {
        std::vector<unsigned char> buf;
        explicit BufferMemory(size_t n) : buf(n, 0) {}

        uintptr_t Base() const { return (uintptr_t)buf.data(); }
        uintptr_t At(size_t off) const { return Base() + off; }
        bool Inside(uintptr_t at, size_t n) const { return at >= Base() && n <= buf.size() && at - Base() <= buf.size() - n; }

        bool  Read(uintptr_t at, void* out, size_t n) override { if (!Inside(at, n)) return false; memcpy(out, (const void*)at, n); return true; }
        bool  Write(uintptr_t at, const void* in, size_t n) override { if (!Inside(at, n)) return false; memcpy((void*)at, in, n); return true; }
        void* Alloc(size_t) override { return nullptr; }

        template <typename T> void Put(size_t off, const T& v) { memcpy(&buf[off], &v, sizeof v); }
    };

    constexpr size_t kGameAt    = 0;
    constexpr size_t kNodesAt   = 0x3B000;
    constexpr size_t kNodeSize  = 0x300;
    constexpr size_t kBufBytes  = kNodesAt + 8 * kNodeSize;

    size_t NodeOff(int i) { return kNodesAt + (size_t)i * kNodeSize; }

    void PutNode(BufferMemory& m, int i, const char* name, uint32_t cookie, float x, float y, float z,
                 int team, int frame, uintptr_t next, uintptr_t vtable = 0x140797520)
    {
        const size_t off = NodeOff(i);
        m.Put(off, vtable);
        if (name) memcpy(&m.buf[off + CActor::name], name, strlen(name));
        m.Put(off + CActorBase::cookie, cookie);
        const float pos[3] = { x, y, z }, orient[3] = { x / 10.0f, y / 10.0f, z / 10.0f };
        memcpy(&m.buf[off + CActorBase::pos], pos, sizeof pos);
        memcpy(&m.buf[off + CActorBase::orient], orient, sizeof orient);
        m.Put(off + CActor::next, next);
        m.Put(off + CActorBase::lastFrame, (int32_t)frame);
        m.Put(off + CActor::team, (int32_t)team);
    }

    void SetHead(BufferMemory& m, uintptr_t head) { m.Put(kGameAt + CGame::actorListHead, head); }

    // Egon (enabled) -> Scuttler_01 (disabled) -> an unnamed node with a cookie the engine never writes.
    void BuildThree(BufferMemory& m)
    {
        SetHead(m, m.At(NodeOff(0)));
        PutNode(m, 0, "Egon",        CActorBase::cookieEnabled,  1.0f, 2.0f, 3.0f, 1, 100, m.At(NodeOff(1)), 0x1000);
        PutNode(m, 1, "Scuttler_01", CActorBase::cookieDisabled, -4.5f, 0.0f, 9.25f, 2, 90, m.At(NodeOff(2)), 0x2000);
        PutNode(m, 2, nullptr,       0x12345678u,                0.0f, 0.0f, 0.0f, 0, 0, 0, 0x3000);
    }
}

int main()
{
    // The plain walk: names, cookies, positions, orientations, team and frame, in chain order.
    {
        BufferMemory m(kBufBytes);
        BuildThree(m);
        ActorList::Node out[8];
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 3);

        CHECK_EQ(out[0].addr, m.At(NodeOff(0)));
        CHECK_EQ(out[0].vtable, (uintptr_t)0x1000);
        CHECK_EQ(std::string(out[0].name), "Egon");
        CHECK_EQ(out[0].cookie, CActorBase::cookieEnabled);
        CHECK_EQ(out[0].enabled, true);
        CHECK_EQ(out[0].pos[0], 1.0f);
        CHECK_EQ(out[0].pos[1], 2.0f);
        CHECK_EQ(out[0].pos[2], 3.0f);
        CHECK_EQ(out[0].orient[0], 0.1f);
        CHECK_EQ(out[0].orient[2], 0.3f);
        CHECK_EQ(out[0].team, 1);
        CHECK_EQ(out[0].lastFrame, 100);

        CHECK_EQ(out[1].addr, m.At(NodeOff(1)));
        CHECK_EQ(out[1].vtable, (uintptr_t)0x2000);
        CHECK_EQ(std::string(out[1].name), "Scuttler_01");
        CHECK_EQ(out[1].cookie, CActorBase::cookieDisabled);
        CHECK_EQ(out[1].enabled, false);
        CHECK_EQ(out[1].pos[0], -4.5f);
        CHECK_EQ(out[1].pos[2], 9.25f);
        CHECK_EQ(out[1].team, 2);
        CHECK_EQ(out[1].lastFrame, 90);

        // No printable name reads as "?", and an unknown cookie is not enabled.
        CHECK_EQ(std::string(out[2].name), "?");
        CHECK_EQ(out[2].cookie, 0x12345678u);
        CHECK_EQ(out[2].enabled, false);

        // A null `out` counts; a small `cap` cuts.
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), nullptr, ActorList::kMaxNodes), 3);
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 2), 2);
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 0), 0);
    }

    // The name field ends at the cookie: 40 printable bytes with no NUL stay 40 and never pick up cookie bytes.
    {
        BufferMemory m(kBufBytes);
        BuildThree(m);
        const std::string forty(40, 'A');
        memcpy(&m.buf[NodeOff(0) + CActor::name], forty.data(), 40);
        m.Put(NodeOff(0) + CActorBase::cookie, (uint32_t)0x41414141);   // "AAAA", printable, must not be read
        ActorList::Node out[8];
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 3);
        CHECK_EQ(std::string(out[0].name), forty);

        // A control byte inside the name makes the whole name "?".
        m.buf[NodeOff(1) + CActor::name + 3] = 0x01;
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 3);
        CHECK_EQ(std::string(out[1].name), "?");
    }

    // Head cases: a null gGame, an unreadable head, a null head.
    {
        BufferMemory m(kBufBytes);
        BuildThree(m);
        ActorList::Node out[8];
        CHECK_EQ(ActorList::Walk(m, 0, out, 8), -1);
        CHECK_EQ(ActorList::Walk(m, m.At(kBufBytes - 0x100), out, 8), -1);
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, -1), -1);
        SetHead(m, 0);
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 0);
    }

    // Cycles: back to the head, back to an inner node, and a node pointing at itself. Each node is reported once.
    {
        BufferMemory m(kBufBytes);
        BuildThree(m);
        ActorList::Node out[8];
        m.Put(NodeOff(2) + CActor::next, m.At(NodeOff(0)));
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 3);
        m.Put(NodeOff(2) + CActor::next, m.At(NodeOff(1)));
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 3);
        m.Put(NodeOff(1) + CActor::next, m.At(NodeOff(1)));
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 2);
        CHECK_EQ(std::string(out[1].name), "Scuttler_01");
    }

    // Unreadable next: outside the buffer, and inside it but too close to the end for a whole node.
    {
        BufferMemory m(kBufBytes);
        BuildThree(m);
        ActorList::Node out[8];
        m.Put(NodeOff(1) + CActor::next, (uintptr_t)0xDEAD0000);
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 2);
        CHECK_EQ(std::string(out[1].name), "Scuttler_01");
        m.Put(NodeOff(1) + CActor::next, m.At(kBufBytes - 0x100));
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 2);
        m.Put(NodeOff(1) + CActor::next, m.At(kBufBytes - kNodeSize));
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out, 8), 3);
    }

    // The bound: a chain longer than kMaxNodes stops there, and a cap above it is clamped.
    {
        const int extra = 5;
        const int total = ActorList::kMaxNodes + extra;
        BufferMemory m(kNodesAt + (size_t)total * kNodeSize);
        SetHead(m, m.At(NodeOff(0)));
        for (int i = 0; i < total; ++i)
        {
            char name[32];
            snprintf(name, sizeof name, "node%d", i);
            const uintptr_t next = i + 1 < total ? m.At(NodeOff(i + 1)) : 0;
            PutNode(m, i, name, CActorBase::cookieEnabled, (float)i, 0.0f, 0.0f, 0, i, next);
        }
        std::vector<ActorList::Node> out((size_t)total);
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), out.data(), total), ActorList::kMaxNodes);
        char last[32];
        snprintf(last, sizeof last, "node%d", ActorList::kMaxNodes - 1);
        CHECK_EQ(std::string(out[(size_t)ActorList::kMaxNodes - 1].name), last);
        CHECK_EQ(ActorList::Walk(m, m.At(kGameAt), nullptr, total), ActorList::kMaxNodes);
    }

    return check::Done("actorlist");
}
