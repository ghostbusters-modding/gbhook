// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>

namespace CGhostbuster
{
#pragma pack(push, 1)
    struct CGhostbuster
    {
        uint32_t pointer;
        uint32_t unk8;              // 0x0008
        uint32_t unkC;              // 0x000C
        uint32_t flags;             // 0x0010 (hex)
        uint32_t unk14;             // 0x0014
        uint32_t unk18;             // 0x0018
        uint32_t unk1C;             // 0x001C
        uint32_t unk20;             // 0x0020
        uint32_t unk24;             // 0x0024
        char GhostbusterName[13];   // 0x0028 (string)
        uint8_t unk35;              // 0x0035
        uint8_t unk36;              // 0x0036
        uint8_t unk37;              // 0x0037
        uint32_t unk38;             // 0x0038 (hex)
        uint32_t unk3C;             // 0x003C (hex)
        uint32_t unk40;             // 0x0040 (hex)
        uint32_t unk44;             // 0x0044
        uint32_t unk48;             // 0x0048
        uint32_t unk4C;             // 0x004C (hex)
        uint32_t unk50;             // 0x004C (hex)
        uint32_t unk54;             // 0x004C (hex)
        float posX;
        float posY;
        float posZ;
    };
#pragma pack(pop)

    // this + 0x2482A: the u8 enableGiantBossMode stores.
    static constexpr uintptr_t giantBoss = 0x2482A;

    // this + 0x24DE8: the u8 enableProtonTorpedo stores.
    static constexpr uintptr_t protonTorpedo = 0x24DE8;

    // this + 0x24FD8: the proton pack object, null between levels. toggleHuntMode stores its u8 at pack + 0x144C.
    static constexpr uintptr_t pack     = 0x24FD8;
    static constexpr uintptr_t packHunt = 0x144C;

    CGhostbuster* getLocalPlayer();
    CGhostbuster* flinch(CGhostbuster* actor);
    void getGhostbusters(char* Buffer, __int64 adr1);
}

extern CGhostbuster::CGhostbuster* egon;
extern CGhostbuster::CGhostbuster* winston;
extern CGhostbuster::CGhostbuster* venkman;
extern CGhostbuster::CGhostbuster* ray;
extern CGhostbuster::CGhostbuster* localPlayer;