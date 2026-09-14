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
#include "services/Window.h"

extern "C" DWORD WINAPI GbHookMain(LPVOID)
{
    gameBase = reinterpret_cast<char*>(GetModuleHandleW(nullptr));

    Log::Init();
    Log::Writef("BOOT", "attached to ghost.exe at %p", gameBase);

    if (!HookBroker::Init())
    {
        Log::Write("BOOT", "ABORT -- MinHook would not initialise, so no hook can be installed. "
                           "gbhook is inert this run.");
        return 1;
    }

    // Armed before anything can fault, so a crash inside ghost.exe is reported as a ghost-relative address.
    FaultLogger::Install();

    Settings::Load();

    // Discovery reads files only, so it runs before any service exists; a conflict is named while all parties are inert.
    Mods::Scan();
    Settings::Attach(Mods::Result());

    // PREBOOT straight after the scan: its one job is beating the engine's boot screens, and the build below takes time.
    Framework::NoteStage(GBH_STAGE_PREBOOT);
    Host::Init(GBH_STAGE_PREBOOT);

    // Build each mod's loose tree into a cached POD and decide what to mount.
    ContentBuild::Build();

    // The mount needs the engine's pod object, so it is parked on the pump and runs from the first front-end ticks.
    Pump::Install();
    if (!ContentBuild::Plans().empty())
        Pump::Park([](void*) { return ContentBuild::Mount(); }, nullptr, "content mount");

    // The other contended detours, hooked once and fanned out. Before EARLY, so a mod may subscribe from its init.
    // The buses exist before any detour can fire one: a bus that waits for its first subscriber has no lock yet.
    Events::Init();
    FrameHook::Install();
    VmHook::Install();
    LevelFlow::InstallFlowHooks();
    Window::Install();

    // The service directory, before EARLY too: a mod publishes from its init and a later stage finds it.
    Services::Init();

    // The command channel: the framework's own, then each service's. Before EARLY, so a mod may register from init.
    Commands::Init();
    InputInject::RegisterCommands();
    LevelFlow::RegisterCommands();
    Pods::RegisterCommands();
    Files::RegisterCommands();
    Services::RegisterCommands();

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
