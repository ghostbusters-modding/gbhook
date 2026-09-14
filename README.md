# gbhook

A mod framework/loader for GBTVGR (Steam, x64). Installs as one DLL file beside
the game, and then one folder per mod. Provides an API for interacting with the 
game engine and installing hooks, to prevent conflicts across multiple mods. Includes a custom in-game menu to view mod status and quickly load installed levels.

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
  gbhook.log           written every run
  mods/<name>/         one folder per mod 
```

The game imports one function from `dinput8.dll`, so we proxy the system version by
placing an indentically named file in the game directory. Then, we forward the real
`dinput8.dll` so the game can use it. With this, we can inject code for mods.

The mod format is a superset of the Ghostbusters Mod Manager's, so an asset-only mod works under both.
[docs/MOD_FORMAT.md](docs/MOD_FORMAT.md) is the reference. Each mod is one folder, with any content assets contained in sub-folders which match the game's package structure. 

Mods are installed by dropping their folder into the `mods` folder in the game directory. 
Then on launch, each mod will be wrapped into a .POD for the game to load. 
If a POD exists for a mod, the game will check its content and use the cached version if no updates are found. Finally, to uninstall a mod and clear its cache, simply remove the mods folder from `mods` and relaunch the game.

## Use it

The main menu's **Mods** row opens gbhook's page:

- **Load Level**: the career levels, then every custom level a mod ships, then the
  checkpoints the chosen level's script registers.
- **View Mods**: every mod gbhook saw, its state, and the reason when it was refused.

Everything that goes through gbhook is logged in `<gamedir>/gbhook.log`, with each line tagged. A mod's log lines are prefixed with the mod's id. 

`<gamedir>/gbhook.cmd` is a command channel: one command per line, polled while the game
runs, no window focus needed. Replies go to the log file.

```
help                          every command, mods' commands included
mods                          mod and subscription status
level <stem> [checkpoint]     load a level, from the front end or in play
checkpoint <name>             reload the live level at one of its checkpoints
files [dir] <pattern>         list assets across every mounted archive
filesize <path>               read an asset through the engine and report its size
pod list | mount <NAME.POD>   the mounted archives
hud <seconds> <text>          the HUD message line
key tap|down|up|hold|spam <NAME> [ms] | clear
services                      every table a mod has published for other mods
hooks                         verify every hook, patch and vtable copy
ping                          liveness and thread state
sleep <ms>                    pause the command stream
```

A mod's commands are `<id>.<name>`, `gb.mymod.mycommand` for example.

## Settings

`<gamedir>/gbhook.ini`, flat `key = value`, `#` or `;` comments.

```ini
mods.root = mods            ; comma-separated, <gamedir>-relative or absolute
gb.mymod.mycommand = 0         ; a mod's setting, under its id; the mod's own default otherwise
```

## Writing a mod

A content mod is a folder of loose assets and a three-line `mod.ini`. A code mod adds a DLL
built against `sdk/include/gbhook/gbhook.h`.

```cpp
#include "gbhook/gbhook.h"
#include "gbhook/gbhook.hpp"

GBHOOK_PLUGIN("gb.mymod");

static void OnFrame(void*) { /* game thread, once per frame in a level */ }

extern "C" GBHOOK_EXPORT int GbhPluginInit(const GbhApi* api)
{
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
MSBuild.exe GbHook.vcxproj -p:Configuration=Release -p:Platform=x64
                                        # -> build/x64/Release/dinput8.dll
MSBuild.exe examples/MyMod/MyMod.vcxproj -p:Configuration=Release -p:Platform=x64
                                        # a mod; the DLL lands beside its mod.ini
```

The pure packages build and test anywhere with g++, and the Windows half links under mingw
as a check:

```
cd tests
make test      # lint, then every suite
make cross     # link the DLL with mingw, unused
```

## Layout

```
sdk/include/gbhook/     gbhook.h, the ABI; gbhook.hpp, C++ conveniences over it
sdk/include/gb/         engine facts: offsets, detour sites, globals, the generated script API
sdk/GbHookPlugin.props  the build settings every mod DLL imports
src/core/               proxy, bootstrap, log, settings, hook broker, fault logger
src/mod/                discovery, the DLL host, the API table, the content build
src/services/           commands, events, level flow, native menu, files, input, hud, pods, services, window
src/format/ registry/ modset/ pod/ bus/ cmd/ input/ menu/ svc/     pure packages, tested offline
tests/                  one suite per package, check.h, the Makefile
tools/harness/          launch, watch the log, send commands, take screenshots
examples/               MyMod, a small code mod; SelfTest, the ABI's in-game self-test; MenuDemo, a menu page
third_party/minhook/    the one MinHook in the process
```

## Docs

| | |
|---|---|
| [docs/MOD_FORMAT.md](docs/MOD_FORMAT.md) | the folder, `mod.ini`, ids, load order, settings |
| [docs/MODDING_GUIDE.md](docs/MODDING_GUIDE.md) | a content mod, then a code mod, then the menu |
| [docs/ABI.md](docs/ABI.md) | the C ABI: versioning, threads, memory, every entry |

## Special Thanks

- **sakis720**: creator of ImmortalPatch and IE17; tester and contributor of knowledge.
- **Malte0641**: creator of termpod; cool modder.
- **KeyofBlueS**: creator of Ghostbusters Mod Manager; cool modder.
- [MinHook](https://github.com/TsudaKageyu/minhook), Tsuda Kageyu, BSD-2
