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
}

extern "C" DWORD WINAPI GbHookMain(LPVOID)
{
    gameBase = reinterpret_cast<char*>(GetModuleHandleW(nullptr));

    Log::Init();
    EnsureSaveFolder();
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
