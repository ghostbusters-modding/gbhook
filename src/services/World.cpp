// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The freeze is the level loop's own byte, never the front-end gate: forcing the gate blacks the frame and ticks
// the legacy menu stack. A direct write keeps audio running, and Esc still opens the real pause menu.
#include "World.h"
#include "FrameHook.h"
#include "Game.h"
#include "../core/Framework.h"
#include "../core/ProcessMemory.h"
#include "../core/Seh.h"
#include "gb/Globals.h"
#include "view/Project.h"

#include <windows.h>
#include <cstring>

namespace
{
    constexpr int kMaxHolders = 8;

    CRITICAL_SECTION g_lock;
    bool             g_ready = false;
    char             g_holders[kMaxHolders][64];
    int              g_holderCount = 0;
    bool             g_weSet = false;   // gbhook wrote the byte, so gbhook may clear it

    void EnsureReady()
    {
        if (g_ready) return;
        InitializeCriticalSection(&g_lock);
        g_ready = true;
    }

    bool ReadU8(uintptr_t at, unsigned char* out)
    {
        ProcessMemory mem;
        return mem.Read(at, out, 1);
    }

    // A plain store under the guard: the byte lives in CGame's heap block, already writable.
    bool Poke(uintptr_t at, unsigned char v)
    {
        GBH_SEH_TRY { *reinterpret_cast<volatile unsigned char*>(at) = v; return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // The engine's predicate, a byte back. -1 when it faulted.
    typedef unsigned char (*tScreenActive)(void* feMgr);
    int CallScreenActive(tScreenActive fn, void* fe)
    {
        GBH_SEH_TRY { return fn(fe) ? 1 : 0; }
        GBH_SEH_EXCEPT { return -1; }
    }

    uintptr_t FreezeAt()
    {
        void* g = Game::Singleton();
        return g ? reinterpret_cast<uintptr_t>(g) + Globals::cgameFreeze : 0;
    }

    void* ReadPtr(uintptr_t rva)
    {
        void*         p = nullptr;
        ProcessMemory mem;
        return (gameBase && mem.Read(reinterpret_cast<uintptr_t>(gameBase + rva), &p, sizeof p)) ? p : nullptr;
    }

    // Off the engine thread: the gate byte, or the active controller's own active bit.
    bool ScreenFromMemory(void* fe)
    {
        unsigned char b = 0;
        if (fe && ReadU8(reinterpret_cast<uintptr_t>(fe) + Globals::feGate, &b) && b) return true;
        void* ctl = ReadPtr(Globals::feController);
        return ctl && ReadU8(reinterpret_cast<uintptr_t>(ctl) + Globals::feControllerActive, &b) && b;
    }

    // Holding the lock. Sets the byte when it is clear, and remembers that gbhook did.
    void Assert()
    {
        const uintptr_t at = FreezeAt();
        unsigned char   b  = 0;
        if (!at || !ReadU8(at, &b) || b) return;
        if (Poke(at, 1)) g_weSet = true;
    }
}

namespace World
{
    void Init() { EnsureReady(); }

    int Paused()
    {
        if (!gameBase) return 0;
        int bits = 0;
        unsigned char b = 0;
        const uintptr_t at = FreezeAt();
        if (at && ReadU8(at, &b) && b) bits |= GBH_PAUSED_FREEZE;

        void* fe = ReadPtr(Globals::feManager);
        int screen = -1;
        if (fe && FrameHook::IsEngineThread())
            screen = CallScreenActive(reinterpret_cast<tScreenActive>(gameBase + Globals::feScreenActive), fe);
        if (screen < 0) screen = ScreenFromMemory(fe) ? 1 : 0;
        if (screen) bits |= GBH_PAUSED_SCREEN;

        if (ReadU8(reinterpret_cast<uintptr_t>(gameBase + Globals::focusLost), &b) && b) bits |= GBH_PAUSED_FOCUS;
        return bits;
    }

    int Pause(const char* owner, bool on)
    {
        EnsureReady();
        const char* id = owner ? owner : "";
        int  slot = -1;
        int  left = 0;
        bool changed = false, full = false;

        EnterCriticalSection(&g_lock);
        for (int i = 0; i < g_holderCount; ++i)
            if (strcmp(g_holders[i], id) == 0) { slot = i; break; }
        if (on && slot < 0)
        {
            if (g_holderCount >= kMaxHolders) full = true;
            else
            {
                strncpy_s(g_holders[g_holderCount++], id, _TRUNCATE);
                changed = true;
                Assert();
            }
        }
        else if (!on && slot >= 0)
        {
            memmove(g_holders[slot], g_holders[slot + 1], sizeof g_holders[0] * (size_t)(g_holderCount - slot - 1));
            --g_holderCount;
            changed = true;
            if (!g_holderCount && g_weSet)
            {
                const uintptr_t at = FreezeAt();
                unsigned char   b  = 0;
                if (at && ReadU8(at, &b) && b) Poke(at, 0);
                g_weSet = false;
            }
        }
        left = g_holderCount;
        LeaveCriticalSection(&g_lock);

        if (full)
        {
            Log::WritefFrom(owner, "WORLD", "cannot hold the freeze: %d mods already do", kMaxHolders);
            return GBH_ERR;
        }
        if (changed && on)  Log::WriteFrom(owner, "WORLD", "holds the freeze");
        if (changed && !on) Log::WritefFrom(owner, "WORLD", "releases the freeze (%d holder(s) left)", left);
        return GBH_OK;
    }

    int ToScreen(const float pos[3], float out[2])
    {
        if (!gameBase) return GBH_ERR_STATE;
        ProcessMemory mem;
        float m[Project::kMatrixFloats];
        Project::Camera cam = { m, 0, 0, 0, 0, 0 };
        const float* top = static_cast<const float*>(ReadPtr(Globals::matrixTop));
        const uintptr_t b = reinterpret_cast<uintptr_t>(gameBase);
        if (!top || !mem.Read(reinterpret_cast<uintptr_t>(top), m, sizeof m)
            || !mem.Read(b + Globals::projScaleX, &cam.scaleX, sizeof cam.scaleX)
            || !mem.Read(b + Globals::projScaleY, &cam.scaleY, sizeof cam.scaleY)
            || !mem.Read(b + Globals::nearPlane, &cam.nearZ, sizeof cam.nearZ)
            || !mem.Read(b + Globals::backbufferW, &cam.width, sizeof cam.width)
            || !mem.Read(b + Globals::backbufferH, &cam.height, sizeof cam.height)
            || cam.width <= 0 || cam.height <= 0)
            return GBH_ERR_STATE;
        return Project::ToScreen(cam, pos, out);
    }

    void Reassert()
    {
        if (!g_ready || !g_holderCount) return;
        EnterCriticalSection(&g_lock);
        if (g_holderCount) Assert();
        LeaveCriticalSection(&g_lock);
    }
}
