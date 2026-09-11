// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Every actor a level script exports passes through here during load, on the game thread, as "CGhostbuster Egon"
// plus the object pointer. Original: IE17 / sakis720.
#include "VmHook.h"
#include "Events.h"
#include "Registry.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "../core/Seh.h"
#include "gb/HookTargets.h"

#include <windows.h>
#include <cstring>

namespace
{
    typedef void (*tExport)(char*, __int64, __int64, __int64);
    tExport oExport = nullptr;

    // The buffer is engine memory: copied under a guard, and an unreadable one registers nothing.
    bool CopyBuffer(const char* src, char* dst, int n)
    {
        GBH_SEH_TRY { lstrcpynA(dst, src, n); return true; }
        GBH_SEH_EXCEPT { dst[0] = '\0'; return false; }
    }

    void Split(const char* buffer, char* cls, size_t clsN, char* name, size_t nameN)
    {
        cls[0] = name[0] = '\0';
        const char* sp = buffer;
        while (*sp && *sp != ' ' && *sp != '\t') ++sp;
        size_t n = (size_t)(sp - buffer);
        if (n >= clsN) n = clsN - 1;
        memcpy(cls, buffer, n);
        cls[n] = '\0';
        while (*sp == ' ' || *sp == '\t') ++sp;
        lstrcpynA(name, sp, (int)nameN);
    }

    void ExportDetour(char* buffer, __int64 adr1, __int64 adr2, __int64 adr3)
    {
        char copy[192];
        if (buffer && CopyBuffer(buffer, copy, sizeof copy) && adr1)
        {
            Registry::Register(copy, reinterpret_cast<void*>(adr1));
            char cls[64], name[128];
            Split(copy, cls, sizeof cls, name, sizeof name);
            Events::FireActor(cls, name, reinterpret_cast<void*>(adr1));
        }
        if (oExport) oExport(buffer, adr1, adr2, adr3);
    }
}

namespace VmHook
{
    bool Install()
    {
        if (!gameBase) return false;
        GbhHook h = HookBroker::Install(nullptr, gameBase + HookTargets::exportGlobalVariable, (void*)&ExportDetour,
                                        (void**)&oExport, GBH_HOOK_EXCLUSIVE);
        if (!h) { Log::Write("VM", "registration hook FAILED to install; the registry and on_actor_registered are off"); return false; }
        Log::Write("VM", "global registration hooked (registry, actor bus)");
        return true;
    }
}
