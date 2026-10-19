// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The startup sequence, and the one place its order is decided. docs/ARCHITECTURE.md records why.

#include "Framework.h"
#include "FaultLogger.h"
#include "HookBroker.h"
#include "mod/Discovery.h"
#include "mod/ContentBuild.h"
#include "mod/Host.h"
#include "services/Commands.h"
#include "services/Events.h"
#include "services/Files.h"
#include "services/FrameHook.h"
#include "services/InputInject.h"
#include "services/LevelFlow.h"
#include "services/Loop.h"
#include "services/ModsMenu.h"
#include "services/NativeMenu.h"
#include "services/Pods.h"
#include "services/Pump.h"
#include "services/VmHook.h"
#include "services/Services.h"
#include "services/Actors.h"
#include "services/Attr.h"
#include "services/Bindings.h"
#include "services/Window.h"
#include "services/World.h"
#include "gb/HookTargets.h"
#include "target/Target.h"

#include <cstring>

namespace
{
    // The engine keeps settings.ini and the saves here and silently refuses to write when the folder is missing.
    void EnsureSaveFolder()
    {
        wchar_t dir[MAX_PATH];
        const DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", dir, MAX_PATH);
        if (n == 0 || n >= MAX_PATH || wcscat_s(dir, L"\\GHOSTBUSTERS") != 0)
        {
            Log::Write("SAVE", "LOCALAPPDATA is not set; the save folder was not checked");
            return;
        }
        if (GetFileAttributesW(dir) != INVALID_FILE_ATTRIBUTES) return;
        if (CreateDirectoryW(dir, nullptr)) Log::Writef("SAVE", "created %ls", dir);
        else Log::Writef("SAVE", "could not create %ls (error %lu)", dir, (unsigned long)GetLastError());
    }

    // A modded run never earns an achievement.
    void BlockAchievements()
    {
        static const uint8_t kExpect[] = { 0x40, 0x53, 0x48, 0x83, 0xEC, 0x30 };   // push rbx; sub rsp,30h
        static const uint8_t kRet = 0xC3;
        if (!HookTargets::achievementUnlock.present())
        {
            Log::Writef("BOOT", "achievements are not blocked: the block is for the Steam build only");
            return;
        }
        void* at = gameBase + HookTargets::achievementUnlock;
        if (memcmp(at, kExpect, sizeof kExpect) != 0)
            Log::Write("BOOT", "achievement unlock is not the expected code: achievements are NOT blocked");
        else if (HookBroker::PatchWrite(nullptr, at, &kRet, 1))
            Log::Write("BOOT", "achievements blocked for this run");
    }

    // A control saved as -1 with a key type reaches the key table as index -1, and the engine's bounds check kills the game.
    using ClearKeyFn = char(__fastcall*)(void*, int);
    ClearKeyFn    oClearKey = nullptr;
    volatile LONG g_badKeyLogged = 0;

    char __fastcall ClearKeyDetour(void* input, int code)
    {
        if ((unsigned)code < 600) return oClearKey(input, code);
        if (InterlockedExchange(&g_badKeyLogged, 1) == 0)
            Log::Writef("GUARD", "a control is bound to key %d, outside the key table: skipped, the game would have closed. "
                                 "Check the controls in settings.ini", code);
        return 0;
    }

    void GuardUnboundControls()
    {
        static const uint8_t kExpect[] = { 0x48, 0x83, 0xEC, 0x28, 0x48, 0x63, 0xCA };   // sub rsp,28h; movsxd rcx,edx
        void* at = gameBase + HookTargets::clearKey;
        if (memcmp(at, kExpect, sizeof kExpect) != 0)
            Log::Write("GUARD", "key clear is not the expected code: unbound controls are NOT guarded");
        else if (HookBroker::Install(nullptr, at, (void*)&ClearKeyDetour, (void**)&oClearKey, GBH_HOOK_EXCLUSIVE))
            Log::Write("GUARD", "unbound controls guarded (key clear)");
        else
            Log::Write("GUARD", "key clear hook FAILED to install: unbound controls are NOT guarded");
    }

    // Past 40 scene passes in a frame the profiler reads a QPC value as an ID3D11Query and the game dies in d3d11.
    // The fix turns it off: the frame end writes 0 to its enable flag and every stamp returns at once.
    void DisableGpuProfiler()
    {
        if (!Settings::GetBool("gpu_profiler_off", true))
        {
            Log::Write("GUARD", "GPU profiler left on (gpu_profiler_off = 0): more than 40 scenes in a frame closes the game");
            return;
        }
        static const uint8_t kExpect[] = { 0x41, 0xC6, 0x46, 0x08, 0x01 };   // mov byte [r14+8], 1
        static const uint8_t kOff = 0x00;
        uint8_t* at = reinterpret_cast<uint8_t*>(gameBase + HookTargets::gpuProfilerEnable);
        if (memcmp(at, kExpect, sizeof kExpect) != 0)
            Log::Write("GUARD", "GPU profiler frame end is not the expected code: the profiler is NOT disabled");
        else if (HookBroker::PatchWrite(nullptr, at + 4, &kOff, 1))
            Log::Write("GUARD", "GPU profiler disabled");
        else
            Log::Write("GUARD", "GPU profiler patch FAILED: the profiler is NOT disabled");
    }
}

extern "C" DWORD WINAPI GbHookMain(LPVOID)
{
    gameBase = reinterpret_cast<char*>(GetModuleHandleW(nullptr));

    Log::Init();
    EnsureSaveFolder();
    Log::Writef("BOOT", "attached to ghost.exe at %p", gameBase);

    // Before any address is used. On an unknown build every address would be a guess, so nothing is written.
    const int build = Target::Identify(reinterpret_cast<const uint8_t*>(gameBase), 0x1000);
    if (build == Target::kUnknown)
    {
        Log::Write("BOOT", "ABORT -- this ghost.exe is not the Steam or the Epic build gbhook knows. "
                           "No hook is installed and no mod is loaded. gbhook is inert this run.");
        return 1;
    }
    GbhPluginBuild = build;
    Log::Writef("BOOT", "ghost.exe is the %s build (md5 %s)", Target::Name(build), Target::Md5(build));

    if (!HookBroker::Init())
    {
        Log::Write("BOOT", "ABORT -- MinHook would not initialise, so no hook can be installed. "
                           "gbhook is inert this run.");
        return 1;
    }

    // Armed before anything can fault, so a crash inside ghost.exe is reported as a ghost-relative address.
    FaultLogger::Install();
    BlockAchievements();
    GuardUnboundControls();

    Settings::Load();
    DisableGpuProfiler();

    // Discovery reads files only, so it runs before any service exists; a conflict is named while all parties are inert.
    Mods::Scan();
    Settings::Attach(Mods::Result());

    // PREBOOT straight after the scan: its one job is beating the engine's boot screens.
    Framework::NoteStage(GBH_STAGE_PREBOOT);
    Host::Init(GBH_STAGE_PREBOOT);

    // The pump first, then the content build on its own thread
    Pump::Install();
    ContentBuild::Start();

    // The other contended detours
    Events::Init();
    Bindings::Init();
    World::Init();
    FrameHook::Install();
    VmHook::Install();
    LevelFlow::InstallFlowHooks();
    Window::Install();

    // The service directory, before EARLY too: a mod publishes from its init and a later stage finds it.
    Services::Init();

    // The command channel: the framework's own, then each service's. Before EARLY, so a mod may register from init.
    Commands::Init();
    InputInject::RegisterCommands();
    Bindings::RegisterCommands();
    LevelFlow::RegisterCommands();
    Pods::RegisterCommands();
    Files::RegisterCommands();
    Services::RegisterCommands();
    Actors::RegisterCommands();
    Attr::RegisterCommands();

    Framework::NoteStage(GBH_STAGE_EARLY);
    Host::Init(GBH_STAGE_EARLY);
    Framework::NoteStage(GBH_STAGE_BOOT);
    Host::Init(GBH_STAGE_BOOT);

    // The game's own front end: the row broker, then gbhook's page on the row the shipped menu hides.
    NativeMenu::Install();
    ModsMenu::Install();

    Framework::NoteStage(GBH_STAGE_READY);
    Host::Init(GBH_STAGE_READY);

    HookBroker::VerifyAll("boot");
    Host::LogStatus();
    Events::LogSummary();
    Services::LogSummary();
    Log::Write("BOOT", "boot complete");

    Loop::Run();   // this thread is the loop thread from here on; it never returns
    return 0;
}
