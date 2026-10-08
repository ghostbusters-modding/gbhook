# gbhook

A mod framework and loader for Ghostbusters: The Video Game Remastered (Steam, x64). It installs as
one DLL beside the game, plus one folder per mod. It gives mods an API for talking to the
engine and installing hooks without conflicting, and adds an in-game menu to see mod status and load levels.

```
mods/<name>/art data world ...  ──▶  built into an archive, mounted at boot    a content mod
mods/<name>/gbhook/<Mod>.dll    ──▶  loaded at its stage, on a C ABI           a code mod
Main menu ▸ Mods                ──▶  Load Level (career, custom, checkpoints), View Mods
gbhook.log, gbhook.cmd          ──▶  everything it did, and a command channel
```

## Install

```
<gamedir>/
  ghost.exe
  dinput8.dll          gbhook
  gbhook.ini           optional settings
  gbhook.log           this run, rewritten every launch
  gbhook.log.prev      the run before
  gbhook.cmd           command channel, created by you
  mods/<name>/         one folder per mod
  gbhook/cache/<id>/   built content archives, gbhook's own
```

The game imports one function from `dinput8.dll`, so gbhook is installed as a `dinput8.dll`
in the game directory. It forwards that call to the real system `dinput8.dll` so the game keeps
working, and runs its own boot from inside the process. To uninstall, delete the file.

A mod is one folder in `mods/`. The format is a superset of the Ghostbusters Mod Manager's, so an
asset-only mod works under both. [docs/MOD_FORMAT.md](docs/MOD_FORMAT.md) is the reference.

- **Content mod**: loose assets in folders that match the game's archive layout.
- **Code mod**: a DLL in the mod's `gbhook/` folder, built against `sdk/include/gbhook/gbhook.h`.
- **Both**: one folder with the two. The `[gbhook]` section of `previews/modinfo.ini` holds the id, stage and settings. A folder without that section loads as a content mod under its folder name.

Mod folders are searched in the `mods_root` folders. A mod listed in `mods_disabled` is skipped.

### Boot stages

A code mod names its stage in `modinfo.ini`. The default is `boot`.

| Stage | When |
|---|---|
| `preboot` | Right after discovery, before any framework service. For beating the engine's boot screens. |
| `early` | Core services, commands and hooks are up, before any level can load. |
| `boot` | After `early`. The default. |
| `ready` | After the native menu and the Mods page are installed. |

### Content

On launch gbhook packs each content mod into a POD archive and mounts it, so the game reads the
mod's files as if they were retail ones. Only engine content is packed: files under the game's
asset roots with the extensions the retail archives carry. Docs, generators and zips stay out.

- The archive goes to `gbhook/cache/<id>/<hash>.POD`, named by a hash of the packed files.
  An unchanged mod mounts its cached archive. A changed one is rebuilt.
- The build runs on its own thread and each archive is mounted as soon as it is ready.
- Archives are written under `.tmp` and renamed when complete, so an interrupted build leaves nothing the engine could mount.
- A cache folder whose mod is gone is removed on the next launch.
- The archives are revision 1. They override the bulk archives and lose to the Mod Manager's rev-0 `MODS.POD`.
- gbhook follows the `PATCH.POD` chain. A mod whose every file is already in the chain is left to the Mod Manager.

## Use it

The main menu's **Mods** row opens gbhook's page:

- **Load Level**: the career levels, then every custom level a mod ships, then the
  checkpoints the chosen level's script registers.
- **View Mods**: every mod gbhook saw, ON or OFF. A mod's own page gives the reason, its asset and code file counts, how its content was cached and mounted, and a Disable or Enable row. That row writes `mods_disabled` and takes effect on the next start.
- Below those, one row per mod that added a page of its own with `mods_page_add`.

### gbhook.log

Everything that goes through gbhook is logged in `<gamedir>/gbhook.log`, each line tagged.
A mod's lines carry the mod's id. The file is truncated every launch and the previous run is kept as `gbhook.log.prev`.

Two things in the log are there to find faults:

- **Crash forensics.** A first-chance access violation, divide by zero, privileged or illegal instruction anywhere in the process is logged once per site as `FAULT`, named by module and offset (`ghost+0x...` for the game), with up to 24 callers. If the game dies right after, that is the site.
- **Callback faults and budget.** A mod callback that faults is named in the log and that one subscription is dropped for good. Every 600 frames, a `frame` subscriber that averaged over 250 microseconds per call is logged as `BUDGET`, because frame callbacks run on the engine's critical path.

### gbhook.cmd

`<gamedir>/gbhook.cmd` is a command channel: one command per line, polled while the game runs, no
window focus needed. gbhook renames the file to `gbhook.cmd.run`, runs it and deletes it, so a script can write the next batch at once. Replies go to the log.

```
help                          every command, mods' commands included
mods                          mod and subscription status
hooks                         verify every hook, patch and vtable copy
ping                          liveness and thread state
level <stem> [checkpoint]     load a level, from the front end or in play
checkpoint <name>             arm a checkpoint in the live level
files [dir] <pattern>         list assets across every mounted archive
filesize <path>               read an asset through the engine and report its size
pod list | mount <NAME.POD>   the mounted archives
hud <seconds> <text>          the HUD message line
key tap|down|up|hold|spam <NAME> [ms] | clear   inject keys
sleep <ms>                    pause the command stream
binds                         every action, its chord and its help
actors [filter]               the engine's own actor list, the spawn pool included
actor <name>                  one actor in full
attr [<key> [<value>]]        objective engine state: god, gravity, time, fov and the rest
services                      every table a mod has published for other mods
```

A mod's commands are `<id>.<name>`, `mymod.mycommand` for example.

## Settings

`<gamedir>/gbhook.ini`, flat `key = value`, `#` or `;` comments. There are no sections. Lists are comma-separated. A double-quoted value keeps `#` and `;`. [gbhook.ini.example](gbhook.ini.example) is a starting copy.

| Key | Default | Meaning |
|---|---|---|
| `mods_root` | `mods` | Folders searched for mods, `<gamedir>`-relative or absolute. |
| `mods_disabled` | empty | Ids or folder names to leave off. The Mods page writes this key. |
| `<id>.<key>` | the mod's own | A mod's setting. The mod's `modinfo.ini` lists its keys and defaults. |
| `<id>.bind.<action>` | the mod's own | The chord for a mod's key action, `F5` or `CTRL+SHIFT+F5`. `NONE` or empty leaves it unbound. |

gbhook creates `%LOCALAPPDATA%\GHOSTBUSTERS` at boot when it is missing, because the game refuses to write its settings and saves without it.

## Writing a mod

A content mod is a folder of loose assets and gbhook's keys in its `modinfo.ini`. A code mod adds a DLL
built against `sdk/include/gbhook/gbhook.h`.

```cpp
#include "gbhook/gbhook.h"
#include "gbhook/gbhook.hpp"

GBHOOK_PLUGIN("mymod");

static void OnFrame(void*) { /* game thread, once per frame in a level */ }

extern "C" GBHOOK_EXPORT int GbhPluginInit(const GbhApi* api)
{
    if (!api || api->abi_version != GBHOOK_ABI_VERSION) return GBH_ERR;
    gbh::bind(api);
    gbh::log("MOD", "hello");
    gbh::on_frame(OnFrame).release();
    return GBH_OK;
}
```

[docs/MODDING_GUIDE.md](docs/MODDING_GUIDE.md) walks through both kinds.

## Build

Visual Studio 2022 or later with the C++ workload, or the Build Tools alone.

```
.\build.ps1                 # Release x64 -> build/x64/Release/dinput8.dll
.\build.ps1 -Install        # also copy it next to ghost.exe (Steam library auto-detected, or -GameDir)
```

`build.ps1` also takes `-Configuration Debug` and `-Clean`, and refuses to install while `ghost.exe` runs. It wraps MSBuild:

```
MSBuild.exe GbHook.vcxproj -p:Configuration=Release -p:Platform=x64
MSBuild.exe examples/MyMod/MyMod.vcxproj -p:Configuration=Release -p:Platform=x64
                            # a mod; the DLL lands in its gbhook/ folder
```

The pure packages build and test anywhere with g++ (C++20), and the Windows half links under mingw
as a check:

```
cd tests
make test      # lint, then every suite
make cross     # link the DLL with mingw, as a check only
make podcheck  # cross-check an emitted POD against gbtvgr-py, skipped when that is not beside this repo
```

`make lint` fails if `windows.h` appears in a pure package. Test output lands in `tests/bin/`.

## Layout

```
sdk/include/gbhook/     gbhook.h, the ABI; gbhook.hpp, C++ conveniences over it
sdk/include/gb/         engine facts: offsets, detour sites, globals, the generated script API
sdk/GbHookPlugin.props  the build settings every mod DLL imports
src/core/               proxy, bootstrap, log, settings, hook broker, fault logger
src/mod/                discovery, the DLL host, the API table, the content build
src/services/           commands, events, level flow, native menu, files, input, hud, pods, levels, actors, attributes, services, window
src/format/ registry/ modset/ pod/ bus/ cmd/ input/ menu/ svc/ actors/ attr/ view/     pure packages, tested offline
tests/                  one suite per package, check.h, the Makefile
tools/harness/          launch, watch the log, send commands, take screenshots
examples/               MyMod, a small code mod; SelfTest, the ABI's in-game self-test; MenuDemo, a menu page
templates/plugin/       reserved for a mod template, empty today
docs/                   the three reference docs below
third_party/minhook/    the one MinHook in the process
build.ps1, GbHook.vcxproj   the build
```

## Docs

| | |
|---|---|
| [docs/MOD_FORMAT.md](docs/MOD_FORMAT.md) | the folder, `modinfo.ini`, ids, load order, settings |
| [docs/MODDING_GUIDE.md](docs/MODDING_GUIDE.md) | a content mod, then a code mod, then the menu |
| [docs/ABI.md](docs/ABI.md) | the C ABI: versioning, threads, memory, every entry |

## Special Thanks

- **sakis720**: creator of ImmortalPatch and IE17; tester and contributor of knowledge.
- **Malte0621**: creator of termpod; cool modder.
- **KeyofBlueS**: creator of Ghostbusters Mod Manager; cool modder.
- [MinHook](https://github.com/TsudaKageyu/minhook), Tsuda Kageyu, BSD-2

## AI Disclaimer
The development of gbhook employed the use of AI coding tools to assist planning and implementation. 