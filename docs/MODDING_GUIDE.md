# Modding guide

A mod is one folder under `<gamedir>/mods/`. A content mod is loose game assets plus a
`modinfo.ini`. A code mod adds a DLL that talks to gbhook through a C table. A mod can be both.

[MOD_FORMAT.md](MOD_FORMAT.md) is the reference for the folder and the ini. [ABI.md](ABI.md)
is the reference for every function in the table. This guide is the path through them, in the
order a mod gets written.

```
1.  Install and first run
2.  A content mod
3.  A code mod: the files, the build, the entry point
4.  Boot stages
5.  Events
6.  Hooks, patches, vtables and exclusive claims
7.  The log: tags, ids and the lines you will see
8.  Commands
9.  Actions and key binds
10. Services between mods
11. Native menu pages and the Mods page row
12. Settings
13. Game files, levels, the game model, pausing
14. Testing a mod
15. Distributing a mod
16. Common failures
```

---

## 1. Install and first run

gbhook is one file, `dinput8.dll`, placed next to `ghost.exe`. The game imports that name, so
it loads gbhook, and gbhook forwards the calls to the system `dinput8.dll`. Build it from this
repo (see the README) and copy `build/x64/Release/dinput8.dll` into the game folder.

```
<gamedir>/
  ghost.exe
  dinput8.dll        gbhook
  gbhook.ini         optional, copy gbhook.ini.example
  gbhook.log         written every run, the previous run is gbhook.log.prev
  gbhook.cmd         optional command channel, see section 8
  mods/<name>/       one folder per mod
```

Start the game and open `gbhook.log`. A healthy start includes these lines, in this order:

```
[21:04:10] BOOT gbhook <version> -- Ghostbusters: TVG Remastered (Steam, x64)
[21:04:10] HOOK MinHook initialised (gbhook is the sole owner)
[21:04:10] BOOT crash forensics armed (every first-chance fault is named by module)
[21:04:10] MODS root <gamedir>\mods
[21:04:10] MODS  1. mymod 0.1.0 (boot, priority 100) from MyMod, plugin MyMod.dll
[21:04:11] MODS loaded mymod 0.1.0 at boot
[21:04:11] BOOT boot complete
```

The main menu then has a **Mods** row. Its page lists **Load Level** and **View Mods**, and one
row per loaded mod that added a page of its own.

To uninstall gbhook, delete `dinput8.dll` from the game folder. `gbhook.ini` holds two keys
of gbhook's own:

```ini
mods_root = mods            ; comma-separated, <gamedir>-relative or absolute
mods_disabled = mymod       ; ids or folder names to leave off
```

The Mods page writes `mods_disabled` for you. Every other key is a mod's setting, see section 12.

## 2. A content mod

A content mod is a folder of assets that mirror the game's archive tree.

```
mods/DuelArena/
├── previews/modinfo.ini
├── world/duel_arena.lvl          the level
├── world/duel_arena.dante        its script
├── world/en/duel_arena.txt       its text
├── sets/duel.bst
└── data/biped1/fiend_wb2.cib
```

```ini
# previews/modinfo.ini
version="1.0.1"
compatibility="PC"
description="A bare 200x200ft test range for 1-on-1 play."
link=""
```

That is the whole mod. It has no gbhook keys, so its id comes from the folder name:
`DuelArena` becomes `duelarena`. [MOD_FORMAT.md](MOD_FORMAT.md) lists the rule.

At boot gbhook walks the folder, builds the files into
`<gamedir>/gbhook/cache/<id>/<hash>.POD`, and mounts that archive above the game's own. A
build runs on its own thread, so a mod that needs rebuilding is late for that mod alone. The
archive is written under a `.tmp` name and renamed when complete, so a cut-short build leaves
nothing half-written behind.

What goes into the archive:

- Only the twelve asset roots: `animations art cinemats data fx materials models physics sets skeletal sound world`.
- Only the file types the engine reads: `.ani .bfm .bst .cib .cinemat .dante .fnt .fxa .fxe .hbb .jug .lvl .mtb .phys2b .sbs .sec .skb .smb .smp .snb .subb .tex .tfb .txt .ui`.
- Nothing else. `gbhook/`, `previews/`, a README and a generator tree stay out, and the log names what was left out.

The cache is keyed on a hash of each packed file's name, size and modified time. An unchanged
mod is mounted from its cache without a rebuild. Touching a file rebuilds the archive. A cache folder whose mod is
gone is removed at the next boot.

The archive is mounted at revision 1. That puts it above the game's bulk archives and below a
Mod Manager `PATCH.POD` chain. When every file of a mod is already in that chain, gbhook
stands down and the log says so.

Check the result in the log:

```
MODS content duelarena: built 5 file(s) -> gbhook\cache\duelarena\<hash>.POD
MODS content: 1 built, 0 cached, 0 left to the Mod Manager, 0 without content
MODS content: 1 of 1 archive(s) mounted
```

A level appears under **Mods ▸ Load Level ▸ Custom** as soon as its `world\<stem>.lvl` is in a
mounted archive and is not a career level. Choosing it shows a page of checkpoints when its
script has any (section 13), otherwise it loads at once. Two mods that ship one path are
named in the log as a `CONFLICT content` line.

## 3. A code mod

Start from `examples/MyMod`. It is the smallest complete mod and the layout every example
follows. `templates/plugin/` is an empty folder and holds nothing to copy.

```
examples/MyMod/
├── MyMod.vcxproj
├── src/MyMod.cpp
├── previews/modinfo.ini
└── gbhook/                   holds only a .gitignore (*.dll) until the first build
```

Copy the folder, rename the files, then change the project's `ProjectGuid`, `RootNamespace`,
`TargetName` and its `ClCompile` line.

### The project file

`MyMod.vcxproj` is short because `sdk/GbHookPlugin.props` carries the settings:

```xml
<Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
<Import Project="..\..\sdk\GbHookPlugin.props" />
<PropertyGroup>
  <TargetName>MyMod</TargetName>
</PropertyGroup>
<ItemDefinitionGroup>
  <PostBuildEvent>
    <Command>copy /Y "$(TargetPath)" "$(ProjectDir)gbhook\"</Command>
  </PostBuildEvent>
</ItemDefinitionGroup>
<ItemGroup>
  <ClCompile Include="src\MyMod.cpp" />
</ItemGroup>
```

The project is a `DynamicLibrary` for `x64` with `PlatformToolset` `v145`. Set the toolset your
Visual Studio provides. A mod kept outside the gbhook tree changes the first import path to
point at the props file, for example `..\gbhook\sdk\GbHookPlugin.props`. The props file
finds the SDK relative to itself, and `GbHookRoot` overrides that.

The props file sets:

| setting | value |
|---|---|
| language | C++20, conformance mode on |
| include paths | `<gbhook>\sdk\include` and the project's own `src` |
| defines | `WIN32_LEAN_AND_MEAN`, `_CRT_SECURE_NO_WARNINGS` |
| runtime | static CRT: `MultiThreaded` in Release, `MultiThreadedDebug` in Debug |
| exceptions | `Async`, so `__try` works around your own engine calls |
| output | `build\x64\<Configuration>\` inside the project folder |

The static CRT matters. Every module owns its own heap, so nothing may be allocated on one
side of the table and freed on the other. The table's `alloc`, `realloc` and `free` exist for
the few buffers that must cross.

Build:

```
MSBuild.exe examples\MyMod\MyMod.vcxproj -p:Configuration=Release -p:Platform=x64
```

The post-build step copies `MyMod.dll` into the project's `gbhook/` folder, so the project
folder is already an installable mod. Create `gbhook/` first if you started without it.
Install by copying the folder into `<gamedir>/mods/`. The `build/`, `src/` and `.vcxproj`
files can stay behind. The content build skips them and the log lists them as left out.

### The source

`examples/MyMod/src/MyMod.cpp`, without its licence header:

```cpp
#include <windows.h>
#include <string>

#include "gbhook/gbhook.h"
#include "gbhook/gbhook.hpp"

GBHOOK_PLUGIN("mymod");

namespace
{
    int g_frames = 0;

    // mymod.hello [words...]: logs its arguments. Handlers run on the game thread.
    int CmdHello(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = "usage: hello <words>"; return GBH_ERR_ARG; }
        std::string words;
        for (int i = 0; i < argc; ++i) { if (i) words += ' '; words += argv[i]; }
        gbh::logf("RES", "%s, %s", gbh::setting("greeting", "hello").c_str(), words.c_str());
        return GBH_OK;
    }

    void OnLevel(int phase, const char* level, int ok, void*)
    {
        if (phase == GBH_LEVEL_PREPARE_END) gbh::logf("LEVEL", "'%s' prepared, ok=%d", level, ok);
    }

    // Once per frame while a level is live. Keep it short: every mod shares this budget.
    void OnFrame(void*)
    {
        if (++g_frames == 1) gbh::hud("hello from mymod", 4.0f);
        if (g_frames % 600 == 0) gbh::logf("EVT", "%d frames in '%s'", g_frames, gbh::level_name().c_str());
    }
}

extern "C" GBHOOK_EXPORT int GbhPluginInit(const GbhApi* api)
{
    if (!api || api->abi_version != GBHOOK_ABI_VERSION) return GBH_ERR;
    gbh::bind(api);

    gbh::logf("MOD", "greeting is \"%s\"", gbh::setting("greeting", "hello").c_str());
    gbh::command_register("hello", CmdHello, nullptr, "<words> -- log a greeting");
    gbh::on_level(OnLevel).release();
    gbh::on_frame(OnFrame).release();
    return GBH_OK;
}
```

Three things make a DLL a mod:

- `GBHOOK_PLUGIN("mymod")` at file scope, in exactly one source file. It exports a data block, the manifest, that carries the id, the ABI version and the `ghost.exe` md5 the SDK was built for. gbhook reads it out of the file without running the DLL.
- `GbhPluginInit`, exported with `GBHOOK_EXPORT` and `extern "C"`. gbhook calls it once, at the mod's stage. Anything but `GBH_OK` leaves the mod inert and the log says `FAIL <id>`.
- `gbh::bind(api)` as the first call. Every `gbh::` wrapper goes through the table that call stores.

Do no work in `DllMain`. It runs under the loader lock, before gbhook can talk to you.
`GbhPluginInit` runs on gbhook's boot thread, not the game thread. Register commands, events,
actions, services and pages there. Engine calls belong in a callback.

gbhook knows which mod a call came from by the caller's return address, so the table has no
mod handle. This is why a `gbh::` call must come from the mod's own code. It is also why the
log lines are prefixed with your id.

### The manifest in `modinfo.ini`

`examples/MyMod/previews/modinfo.ini`:

```ini
version="0.1.0"
compatibility="PC"
description="A small example mod."
link=""

; The keys above are the Mod Manager's. The keys below are gbhook's: docs/MOD_FORMAT.md is the reference.
; A '#' or ';' starts a comment, mid-line too, except inside double quotes. Lists are comma-separated.

id       = mymod       ; unique; the command, settings and log namespace; matches GBHOOK_PLUGIN in the DLL
abi      = 1              ; GBHOOK_ABI_VERSION from gbhook.h; must match the DLL's manifest
plugin   = MyMod.dll      ; a DLL under gbhook/, or leave the key out
stage    = boot           ; preboot | early | boot | ready
priority = 100            ; within a stage, low runs first
;requires = othermod   ; mod ids that must be present and loaded

greeting = hello          ; this mod's default; gbhook.ini overrides it as mymod.greeting
```

`id`, `abi` and the DLL's manifest are checked against each other before the DLL is loaded.
A mismatch refuses the mod with both values in the log. The id uses `a-z`, `0-9` and `_`, at
most 63 characters. No dots: a dot splits a setting or command name from its id.

Every other key in the file is a setting default (section 12). Section headers such as
`[gbhook]` are accepted and ignored: the file is flat.

### Run it

Install, start the game, and read the log. The `hello` command is reachable at once, from
the front end too:

```
echo mymod.hello from the command file >> "<gamedir>/gbhook.cmd"
```

```
[21:07:02] QUEUED mymod.hello from the command file (front end)
[21:07:02] CMD  mymod.hello from the command file
[21:07:02] RES  [mymod] hello, from the command file
[21:07:02] OK   mymod.hello from the command file
```

## 4. Boot stages

`stage` in `modinfo.ini` says when `GbhPluginInit` runs. Within a stage, `priority` orders,
low first, then the id.

| stage | when | pick it for |
|---|---|---|
| `preboot` | straight after discovery, before any framework service is up | a detour that must beat the engine's boot screens. Hooks, patches, settings and the log work. Treat the rest as unavailable. |
| `early` | core services up, before any level can load | a hook or subscription that must exist before the first level loads |
| `boot` | the default, after `early` | everything else |
| `ready` | after the native menu broker is installed | a mod that consumes another mod's service or needs the menu broker present |

Names, never numbers, go in `modinfo.ini`. A mod loads after a mod it needs only when its stage
is later or its priority is higher in the same stage. `requires` states the dependency and
refuses the mod when the other is absent or refused. It orders nothing, and the log warns when
the dependency initialises later.

A claim on a main-menu row made before `ready` is recorded and goes live when the broker
installs, so `boot` is enough for menu mods.

## 5. Events

```cpp
void OnFrame(void*)                                        { /* a level is live */ }
void OnPump(void*)                                         { /* every frame, front end included */ }
void OnLevel(int phase, const char* level, int ok, void*)  { /* prepare begin and end */ }
void OnActor(const char* cls, const char* name, void* ptr, void*) { /* each VM global */ }

gbh::on_frame(OnFrame).release();
gbh::on_pump(OnPump).release();
gbh::on_level(OnLevel).release();
gbh::on_actor_registered(OnActor).release();
```

| event | thread | fires |
|---|---|---|
| `on_frame` | game | once per frame while a level ticks, before the engine's own tick |
| `on_pump` | main | every frame, front end included, after the engine's pump |
| `on_level` | game | around level prepare. `phase` is `GBH_LEVEL_PREPARE_BEGIN` or `GBH_LEVEL_PREPARE_END`, `ok` is meaningful at the end |
| `on_actor_registered` | game | during a level load, once per Dante VM global (`"CGhostbuster"`, `"Egon"`) |
| `on_key`, `on_char` | message | see section 9 |

Every callback runs under a guard. The first fault names the mod and the event in the log, drops
that one subscription for the process, and the game continues. Each event holds at most 64
subscribers.

`on_frame` and `on_pump` sit on the engine's critical path and are timed. An `on_frame`
subscriber averaging over 250 microseconds per call is named every 600 frames. Keep both short.

`release()` keeps a subscription for the life of the process, which is the usual case. A
`gbh::Sub` that goes out of scope unsubscribes.

## 6. Hooks, patches, vtables and exclusive claims

gbhook owns the one MinHook in the process. A mod never links its own.

```cpp
#include "gb/HookTargets.h"

typedef __int64 (__fastcall* tFn)(void*, void*, void*, void*);   // match the real signature
tFn oMovie = nullptr;

__int64 __fastcall MovieDetour(void* a, void* b, void* c, void* d)
{
    return oMovie(a, b, c, d);
}

// in GbhPluginInit
GbhHook h = gbh::hook_at(HookTargets::playMovie, (void*)&MovieDetour, (void**)&oMovie,
                         GBH_HOOK_DEFERRED | GBH_HOOK_EXCLUSIVE);
if (h) gbh::enable_batch(&h, 1);
```

`hook_at` takes a `ghost.exe`-relative offset and `hook` takes an absolute address. Both return
`nullptr` when refused, and the log says why. `GBH_HOOK_DEFERRED` creates the hook without
enabling it, and `enable_batch` arms a set together, so a group of hooks is never half-armed.

There is one detour per address. A second request is refused whichever flags it carries, and
the log names the current owner. The addresses gbhook hooks itself are in this state from the start:
the tick, the pump, level prepare, VM registration, the script-fault reporter, begin-level
and the front-end action poll. They reach mods as events. `sdk/include/gb/HookTargets.h` names
the known detour sites.

A byte patch is recorded and re-verified:

```cpp
const uintptr_t kPatchRva = 0;                      // a ghost.exe-relative offset from your own RE
static const uint8_t nop2[] = { 0x90, 0x90 };
gbh::Patch p(gbh::at(kPatchRva), nop2, sizeof nop2);   // at most GBH_MAX_PATCH_BYTES (64)
p.release();                                        // else it reverts when p dies
```

A patch inside a detoured prologue is refused, and so is a detour over a patch.

A vtable override is a copy of the shipped table, with slots claimed per mod. A table gbhook already cloned with another slot count is refused.

```cpp
typedef void (__fastcall* tActivate)(void*, int);
tActivate oActivate = nullptr;
void __fastcall MyActivate(void* self, int row) { oActivate(self, row); }

const uintptr_t kTableRva = 0;                     // the shipped vtable, ghost.exe-relative
gbh::VtableOverride vt(gbh::at(kTableRva), 116);   // the table and its slot count
vt.set(83, &MyActivate, &oActivate);
vt.apply(object);                                  // the object's vptr now points at the copy
```

Apply it only to objects that live for the process. A slot another mod already claimed is
refused with the owner named.

### Exclusive claims

A mod that detours an address nobody else may touch declares it in the manifest, using the
long macro in place of `GBHOOK_PLUGIN`:

```cpp
GBHOOK_PLUGIN_EXCLUSIVE("fastboot") { "ghost+0x248190", "ghost+0x2487E0", "" } GBHOOK_PLUGIN_END;
```

The list ends at the first empty string and holds at most 16 addresses. At discovery gbhook
compares every accepted mod's list as text, so spell each address the same way in both mods.
Two mods that claim one address both load, and the log says:

```
MODS CONFLICT hook ghost+0x248190 claimed by both 'a' and 'b' -- whichever loads first wins and the other is refused at install time
```

The mod that loads second gets `nullptr` from `hook_at`.

### Integrity

gbhook re-reads the first bytes of every enabled hook, every patch and every applied vtable
pointer after each mod loads and at the end of boot. The `hooks` command runs the same sweep on
demand. A clean sweep logs `HOOK integrity ok: N hook(s), N patch(es), N vtable copy(ies)
verified after <mod id>`. Drift logs a `HOOK TAMPER` line with the owner and what the sweep
followed.

## 7. The log

`gbhook.log` is the record of every run. A line is the time, a tag padded to four characters,
the mod id in brackets when a mod wrote it, then the text:

```
[21:04:11] MOD  [mymod] greeting is "hello"
[21:04:11] HOOK installed ghost+0x1F1210 by 'gbhook' [exclusive]
```

```cpp
gbh::log("MOD", "one line");
gbh::logf("MOD", "%d things", n);       // a line is cut at 1024 characters
```

The tag names the subsystem. The id comes from gbhook. The tags a mod author meets:

| tag | from |
|---|---|
| `BOOT` | startup, version, crash forensics |
| `MODS` | discovery, load order, content build, refusals, mod pages |
| `INI` | `gbhook.ini` and `modinfo.ini` reading |
| `HOOK` | hook, patch and vtable installs, conflicts, integrity |
| `FRAME`, `PUMP`, `VM`, `WND` | the engine hooks behind the events |
| `EVT` | event faults and budget reports |
| `CMD`, `OK`, `ERR`, `QUEUED`, `RES` | the command channel and command replies |
| `LEVEL`, `REG`, `GTFO` | level flow, the VM registry, script faults |
| `NMENU` | native menu pages and rows |
| `BIND` | actions and chords |
| `SVC` | published services |
| `WORLD` | the pause hold |
| `FILE`, `POD` | file reads and listings, archive mounts |
| `FAULT` | first-chance crash forensics |
| `TEST` | the SelfTest example |

Pick your own tags for your own lines. A short stable tag per feature makes `grep` work.

### Time budget

```
EVT BUDGET 'mymod' averages 412us per frame over 600 calls (soft limit 250us) -- this is on the engine's critical path
```

A frame subscriber over the soft limit is reported once per 600 frames. Nothing is cut off.
Move the work out of the callback, or spread it over frames.

### Crash forensics

A vectored handler logs every first-chance access violation, divide by zero, illegal
instruction and privileged instruction, once per address and for up to 32 addresses:

```
FAULT code=C0000005 at ghost+0x1F1230 data=0x0 on thread 4120 -- first chance. If the game dies right after this, THIS is the site.
FAULT   caller  1: MyMod.dll+0x1A40
FAULT   caller  2: ghost+0x1F1210
```

A site in `ghost.exe` is `ghost+0x<offset>`. A site in your DLL is `MyMod.dll+0x<offset>`.
A fault your own `__try` handles still appears here, since the line is written on the first
chance. It is a lead, and the game may carry on. A fault inside a callback is also caught and
reported as `EVT FAULT in 'mymod' frame callback -- that subscription is off for this session; the
game continues`.

### Hook integrity

See section 6: `HOOK integrity ok`, `HOOK TAMPER`, `HOOK CONFLICT`.

### Refused and failed mods

Discovery writes `MODS REFUSED <folder>: <reason>` for a mod it will not load. The same reason is on
the mod's page under View Mods. A DLL that loads and then fails its init is
`MODS FAIL <id>: <reason> -- the mod is inert`. Section 16 lists the texts.

## 8. Commands

A command is a function a mod registers by name. It can be run from `gbhook.cmd`, from another
mod and from gbhook itself.

```cpp
int CmdSpawn(int argc, const char* const* argv, const char** err, void*)
{
    if (argc < 1) { *err = "usage: spawn <name>"; return GBH_ERR_ARG; }
    gbh::logf("RES", "spawning %s", argv[0]);
    return GBH_OK;
}

gbh::command_register("spawn", CmdSpawn, nullptr, "<name> -- spawn something");
```

The command is `mymod.spawn`, lowercased. `argv` holds the arguments only. A handler sets `*err`
to a static string and returns a `GbhStatus` to report failure. A name cannot hold blanks or
quotes, and a second registration of a name is refused.

Every mod command runs on the game thread, so engine calls are safe inside it. Called from
another thread, it is queued and runs at the next frame, or at the next pump while no level
ticks. The queue holds 64 lines and drains 8 per frame.

```cpp
gbh::run("mymod.spawn egon");     // now, or queued when the caller is not the game thread
gbh::queue("mymod.spawn egon");   // always waits for the next frame
```

### The command channel

`<gamedir>/gbhook.cmd` is polled about every 16 ms for as long as the game runs. Each line is
one command. Blank lines and lines starting with `#` are skipped, and `"double quotes"` keep a
word with spaces together. gbhook renames the file to `gbhook.cmd.run` before reading it and
deletes it after, so a script can write the next batch while this one runs.

Each command writes `CMD`, then its `RES` lines, then `OK` or `ERR`. A command that has to wait for
the game thread also writes `QUEUED` first. The commands gbhook ships:

```
help                          every command, mods' commands included
mods                          mod and subscription status
hooks                         verify every hook, patch and vtable copy
ping                          liveness and thread state
level <stem> [checkpoint]     load a level, from the front end or in play
checkpoint <name>             reload the live level at one of its checkpoints
files [dir] <pattern>         list assets across every mounted archive
filesize <path>               read an asset through the engine and report its size
pod list | mount <NAME.POD>   the mounted archives
hud <seconds> <text>          the HUD message line
key tap|down|up|hold|spam <NAME> [ms] | clear
sleep <ms>                    pause the command stream
actors [filter] | actor <name>
attr [<key> [<value>]]       engine state: list, read or set one
binds                         every action and its chord
services                      every table a mod has published
```

## 9. Actions and key binds

A key the player should be able to rebind is an action. The mod names it. The player binds it.

```cpp
bool g_open = false;
void OnMenu(const char* name, void*) { g_open = !g_open; }

gbh::action_register("menu", OnMenu, nullptr, "open the overlay");
```

```ini
# previews/modinfo.ini
bind.menu = F1
```

The default chord is `bind.<action>` in the mod's `modinfo.ini`. The player overrides it in
`gbhook.ini` as `mymod.bind.menu = CTRL+SHIFT+M`, or turns it off with `NONE`. A chord is
an optional `CTRL`, `SHIFT` and `ALT` plus one key, joined with `+`. Key names include `A` to `Z`,
`0` to `9`, `F1` to `F12`, `ENTER`, `SPACE`, `TAB`, `ESC`, the arrows, `HOME`, `END`, `PGUP`,
`PGDN`, `INSERT`, `DELETE` and `NUMPAD0` to `NUMPAD9`.

- The handler runs on the main thread, once per press. `name` is the action's own, so one function can serve many actions.
- A bound key never reaches the game.
- The first claim on a chord wins. The loser stays unbound and the log names both: `BIND menu wants F1, held by other menu; unbound`.
- An action name has no whitespace and at most 31 characters.
- `gbh::action_held("menu")` answers while the whole chord is down.
- `gbh::action_enable("menu", false)` hands the key back to the engine until re-enabled.
- `gbh::action_capture(true)` lets a text field take the keyboard: only the caller's actions fire until it lets go.
- The `binds` command lists every action and its chord.

### Raw keys

A mod that owns a key while some mode is on subscribes to the window's keys and answers `1`:

```cpp
int OnKey(int vk, int down, void*)
{
    return g_flying && (vk == 'W' || vk == 'A' || vk == 'S' || vk == 'D') ? 1 : 0;
}
gbh::on_key(OnKey).release();
```

`vk` is a Windows virtual-key code. The callback runs on the message thread. Subscribers run in
registration order and the first to answer `1` ends the fan-out, before actions see the key.
`on_char` carries typed characters the same way. gbhook subclasses the game window once, so a
mod never subclasses it.

To press a key in the game, use the engine's own scan-code table. SendInput never reaches it:

```cpp
gbh::key(gbh::dik("SPACE"), true);    // held until key(..., false)
```

## 10. Services between mods

A mod that offers a table to other mods publishes it by name, and a consumer finds it:

```cpp
struct MyTable { uint32_t struct_size; int (*answer)(void); };
static const MyTable g_table = { sizeof g_table, Answer };

gbh::service_publish("table", &g_table, sizeof g_table);          // listed as mymod.table

const MyTable* t = gbh::service_find<MyTable>("mymod.table");     // null until it is published
```

The table must live for the process, and it must start with a `uint32_t struct_size` so a
consumer can check what it carries. A lookup ignores case. A name cannot hold blanks or quotes,
and `<id>.<name>` must fit in 63 characters. Publishing a name twice returns `GBH_ERR_CONFLICT`.
At most 64 services exist.

A consumer must load after its publisher: a later stage, or a higher `priority` in the same
stage. `services` in `gbhook.cmd` lists every table with its owner and size.

## 11. Native menu pages and the Mods page row

The game's main menu has seven rows: Career, Online, Mods, a free slot, Options, Extras, Exit.
Mods is gbhook's. Online and the free slot are hidden until a mod claims one with
`GBH_ROW_ONLINE` or `GBH_ROW_FREE`. The rest are refused with `GBH_ERR_CONFLICT`.

A claimed row's callback can push a page built on the engine's own screen class. A page is
data: `build` publishes labels with actions, and `activate` answers with a verdict.
`examples/MenuDemo` is the complete example, and this is its core:

```cpp
int g_count = 0;

void PageBuild(void*)
{
    char c[GBH_NATIVE_LABEL_CAP];
    snprintf(c, sizeof c, "Count: %d", g_count);
    gbh::native_submenu_add_row("Log a line", 1);
    gbh::native_submenu_add_row(c, 2);
    gbh::native_submenu_add_row("A header row", GBH_NATIVE_INERT);
    gbh::native_submenu_add_row("Close", 3);
}

int PageActivate(int action, void*)
{
    if (action == 1) { gbh::log("MENU", "row 'Log a line' activated"); return GBH_NATIVE_STAY; }
    if (action == 2) { ++g_count; return GBH_NATIVE_STAY; }
    return GBH_NATIVE_CLOSE;
}

void OnRow(int, void*)
{
    GbhNativeMenuDesc d = { sizeof d, PageBuild, PageActivate, nullptr, "Menu Demo" };
    if (gbh::native_submenu_open(d) != GBH_OK) gbh::log("MENU", "the page could not be opened");
}

// in GbhPluginInit
if (gbh::native_row_claim(GBH_ROW_ONLINE, OnRow) == GBH_OK)
    gbh::native_row_label(GBH_ROW_ONLINE, "Menu Demo");
```

What `activate` returns:

| verdict | effect |
|---|---|
| `GBH_NATIVE_STAY` | rebuild and redraw this page. Changed labels are relabelled in place. |
| `GBH_NATIVE_CLOSE` | pop this page |
| `GBH_NATIVE_CLOSE_ALL` | pop every page back to the main menu. Use it for a row that starts a level load. |

A row action of `GBH_NATIVE_INERT` makes a header or a text line that never activates. A page
may open another page from inside its `activate`, up to 16 deep. ESC pops one page. A page holds
at most 40 rows and a label at most 39 characters. A title is shown verbatim, and a leading `@`
names a localisation key. `native_submenu_open` works only inside a row callback or a page's
`activate`. `native_submenu_refresh()` rebuilds the open page when your state changes.

### A row on the Mods page

A mod that only wants a settings page does not need a main-menu row. `mods_page_add` puts one
row for the mod on the Mods page, below View Mods:

```cpp
GbhNativeMenuDesc d = { sizeof d, PageBuild, PageActivate, nullptr, "MyMod" };
if (gbh::mods_page_add("MyMod", d) != GBH_OK) gbh::log("MENU", "no Mods page row");
```

- Call it from `GbhPluginInit` only. After init it returns `GBH_ERR_STATE`.
- One row per mod. A second call logs `a second Mods page refused: one per mod` and returns `GBH_ERR_CONFLICT`.
- The row shows only while the mod is loaded, in code order. The page holds up to 38 mod rows.
- The label is cut to 39 characters and cannot be empty or hold a line break. A null `title` shows the label.
- The descriptor is copied.

The wrappers `mods_page_add` and `setting_set` are the newest part of the table. A mod that
must run on an older gbhook gates them with `gbh_api_has(api, mods_page_add)`, which
`gbh::has_mod_pages()` wraps. The `gbh::` wrappers answer `GBH_ERR_UNSUPPORTED` on an older table.

## 12. Settings

```cpp
const bool on   = gbh::setting_bool("bootskip", true);
const int  rate = gbh::setting_int("spawn_rate", 4);
const auto font = gbh::setting("overlay.font", "");
```

A key is read as `<id>.<key>` from `gbhook.ini`, then as `<key>` from the mod's own
`modinfo.ini`, then the default passed in. Keys are case-insensitive. A mod ships its defaults
in `modinfo.ini` and the player overrides them in `gbhook.ini`:

```ini
# mods/MyMod/previews/modinfo.ini
spawn_rate = 4

# <gamedir>/gbhook.ini
mymod.spawn_rate = 12
```

A `modinfo.ini` key that already starts with the mod id is read doubled, and the log warns:
`INI warning mymod: modinfo.ini key 'mymod.x' repeats the mod id; it is read as 'mymod.mymod.x'`.
Leave the id off the key.

`setting_set` writes a value back to `gbhook.ini` as `<id>.<key>` and makes the next read see it:

```cpp
gbh::setting_set("spawn_rate", "12");
gbh::setting_clear("spawn_rate");        // removes the line, the modinfo.ini default applies again
```

A key may hold letters, digits, `_`, `.` and `-`. A value cannot hold a line break or a double
quote. Anything else returns `GBH_ERR_ARG`. The file is replaced through a temporary, so a crash
never leaves half of it. An unwritable file logs `gbhook.ini could not be written; <key> is unchanged`.

## 13. Game files, levels, the game model, pausing

### Files

The file calls use the engine's own resolution, so a mod's archive and the game's answer alike.

```cpp
std::vector<std::string> lvls  = gbh::files("world", "*.lvl");
std::vector<unsigned char> src = gbh::file_read("world\\duel_arena.dante");
```

They run on the engine's main thread only: a command handler, `on_frame` or `on_pump`. From
`GbhPluginInit` or a thread of your own they fail with `GBH_ERR_WRONG_THREAD`, and the wrapper
returns an empty vector. Paths use backslashes and are relative to the archive root. A path with
`..`, an empty path, or a path of 240 characters or more is refused. A read is capped at 64 MiB.
The `files` and `filesize` commands do the same from `gbhook.cmd`.

### Loading levels

```cpp
gbh::level("duel_arena");                            // from the front end or in play
gbh::level("library1b", "checkpoint_OldStacks");     // starting at a checkpoint
```

This is the `level` command. In the front end the load is handed to the menu loop. In play it
chains to the level. A checkpoint is a function named `checkpoint_<Name>` in the level's
`.dante` script, registered by the script with `defineCheckpoint`. The call accepts either that
function name or the display name the script registered. An unknown checkpoint logs the ones the
level has. The `checkpoint` command reloads the live level at one.

The lists the Mods page shows are available to mods, on the engine's main thread:

```cpp
std::vector<std::string> career = gbh::levels(GBH_LEVELS_CAREER);    // the engine's table, in order
std::vector<std::string> custom = gbh::levels(GBH_LEVELS_CUSTOM);    // every other world\*.lvl
std::vector<std::string> cps    = gbh::checkpoints("library1b");     // from the level's script
```

### The game model

```cpp
void*       game = gbh::game_singleton();      // CGame*, null very early
void*       hero = gbh::local_player();        // null in menus and during loads
std::string stem = gbh::level_name();          // "" at the front end
```

The registry is every Dante VM global the level registered, class and name to pointer:

```cpp
for (const GbhRegistryEntry& e : gbh::registry()) { /* e.cls, e.name, e.ptr */ }
GbhRegistryEntry egon;
if (gbh::registry_find("Egon", egon)) { /* exact match, then substring, any case */ }
```

The engine's own actor list is wider. It holds every `CActor` in the level, the spawn pool
included, with the class read from the RTTI:

```cpp
for (const GbhActorInfo& a : gbh::actors())
{
    const bool live  = a.flags & GBH_ACTOR_ENABLED;     // clear: pooled, built and switched off
    const bool ghost = a.flags & GBH_ACTOR_GHOST;
    /* a.name, a.cls, a.pos, a.orient, a.team, a.ptr */
}
GbhActorInfo slimer;
if (gbh::actor_find("Slimer", slimer) && gbh::actor_is_a(slimer.ptr, "CCharacter")) { /* ... */ }
```

A pointer from either list is live game memory. Use it on the game thread and drop it at the
next level prepare. `generation` says which level it belongs to.

Objective engine state has a key. `attr` reads it out of memory and sets it through the engine's
own native:

```cpp
bool god = false;
if (gbh::attr_bool("god", god)) { /* the live flag, whatever set it */ }
gbh::attr_set("god", "toggle");            // game thread: a command handler or on_frame
gbh::attr_set("gravity", "-1.6");          // in range, else GBH_ERR_ARG
gbh::attr_set("time", "reset");
std::string fov = gbh::attr("fov");        // the display form with its unit, read-only
for (const GbhAttrInfo& a : gbh::attrs()) { /* a.key, a.type, a.writable, a.min, a.max, a.unit, a.help */ }
```

The keys are `god giant torpedo hunt gravity time fov camdist cammode`. `fov`, `camdist` and
`cammode` are read-only and `attr_set` answers `GBH_ERR_UNSUPPORTED` for them. The `attr`
command does the same from `gbhook.cmd`. A toggle the engine has no getter for stays the mod's
own state and never becomes a key.

`gbh::read<T>(src, out)` is a guarded copy out of game memory. It answers `false` on an unmapped
or torn pointer and never crashes.

The engine's script-visible methods are typed wrappers in
`sdk/include/gb/Generated/GBApi.generated.h`. They call each method through the engine's own
script thunk. The header expects a global `char* gameBase` and the mod defines it:

```cpp
#include "gb/Generated/GBApi.generated.h"
char* gameBase = nullptr;                  // define once, then set it in GbhPluginInit
// gameBase = static_cast<char*>(gbh::game_base());

GB::CCharacter::setInvulnerableFlag(hero, true);
```

Guard every call into engine code with `GBH_SEH_TRY` and `GBH_SEH_EXCEPT` from
`gbhook/seh.h`. A guarded function must stay a leaf with no C++ objects to unwind.

### HUD

```cpp
gbh::hud("Objective updated", 3.0f);       // the top message line, needs a live level
```

Without a live level the line is dropped and the log says `HUD no level is live`.

### Pausing the world

An overlay that should stop the game holds the freeze while it is open:

```cpp
gbh::pause(true);     // the level loop freezes and audio carries on
gbh::pause(false);    // the world ticks again once no mod holds it
```

Esc still opens the game's own pause menu during the freeze. Several mods can hold the freeze
at once, up to eight, and each has to release its own hold. `gbh::paused()` returns bits:
`GBH_PAUSED_FREEZE` (the freeze is set), `GBH_PAUSED_SCREEN` (a front-end screen is up) and
`GBH_PAUSED_FOCUS` (the window lost focus). `gbh::world_to_screen(pos, out)` turns a world
position into pixels for a marker and returns `false` behind the camera or off screen.

## 14. Testing a mod

### In the game

`gbhook.log` is the first tool. Read it from the top after each run, and look for lines tagged
`MODS`, `HOOK`, `EVT`, `FAULT` and `ERR`. Drive the mod through `gbhook.cmd`, since every
command, level load and key press has a command form.

`tools/harness/gb.sh` wraps the loop from WSL:

```
gb.sh build | install | start | stop | status | cmd "<line>" | wait "<re>" [sec]
      | log [n] | shot [name] | keys <step ...> | post <KEY|'wait N'> ...
      | menuskip | unlock | clean
```

`cmd` appends a line to `gbhook.cmd` and prints the `CMD`, `RES` and `OK` or `ERR` lines it
produced. `wait` blocks until a regex matches a new log line. `start` refuses to launch the game
unless `GB_ALLOW_LAUNCH=1` is set. The paths at the top of the script (`REPO_WSL`, `REPO_WIN`,
`TOOLS_WIN`, `GAME_WSL`, `MSBUILD`) describe one machine and need editing for yours.

`examples/SelfTest` is the table's in-game self-test. It exercises each group of the table
against the mod's own memory, and each check is a `PASS` or `FAIL` line under `TEST`. When a
call does not behave as this guide says, run that mod first. Its `modinfo.ini` has two switches
for its own behaviour: one frame subscriber faults on purpose to prove the guard, and a flag
makes `GbhPluginInit` fail to show the `FAIL` path.

### Offline

The pure parts of gbhook are tested without the game. The suites are in `tests/`:

```
cd tests
make test      # lint, then build and run every suite under g++
make cross     # link the Windows half into a dinput8.dll with mingw, as a check only
make podcheck  # cross-check the POD writer against gbtvgr-py when it is present
make clean
```

`make test` needs `g++` and `x86_64-w64-mingw32-gcc`. The mingw compiler builds
`tests/format/fixture_mod.c` into a real mod DLL for the PE reader suite, and nothing loads it.
Each suite prints `<suite>: N passed, M failed` and exits 1 on a failed check.

The pure packages are `format registry modset pod bus cmd input menu svc actors attr view`.
A package may not include `windows.h`, and `make lint` fails when one does. A suite links its
own package and the packages it lists as dependencies in the Makefile. The only stub is
`tests/stub/MinHookStub.cpp`: no-op MinHook entry points that let `make cross` link. Nothing
offline runs the hooks themselves.

A suite is one file, `tests/<package>/test_<name>.cpp`, with a `main`:

```cpp
#include "check.h"
#include "cmd/Line.h"

int main()
{
    CHECK_EQ(Line::Tokenize("level haunt1").size(), (size_t)2);
    CHECK(Line::IsSkippable("# a comment"));
    return check::Done("line");
}
```

For a mod of your own, keep logic that needs no engine in plain headers without `windows.h`,
and test it with `check.h` under g++. The engine-facing code is tested in the game.

## 15. Distributing a mod

The upload is one zip holding one folder, named for the mod, with nothing above it. The zip
unpacks into `<gamedir>/mods/`.

```
My_Mod.zip
└── My_Mod/
    ├── LICENSE
    ├── README.txt
    ├── previews/
    │   ├── modinfo.ini         the Mod Manager's keys, then gbhook's
    │   └── preview_01.png
    ├── art/  data/  world/     loose assets, mirroring the game's archive tree
    │   sets/  models/ ...
    └── gbhook/
        └── MyMod.dll           the Release x64 build, and nothing else
```

- A content-only mod has no `gbhook/` folder. A code-only mod has no asset folders.
- The version is `version` in `modinfo.ini`, and it is not part of the zip name. The log prints it on the `loaded` line and View Mods shows it.
- `previews/` holds the manager's metadata and screenshots. gbhook reads `modinfo.ini` from it and writes only `gbhook.ini`.
- The content cache is `<gamedir>/gbhook/cache/<id>/`, one archive per mod, built by the player's copy of gbhook. Nothing of it belongs in a mod.
- Build the DLL in Release with the static CRT, which `GbHookPlugin.props` sets.
- The DLL's manifest carries the target `ghost.exe` md5. A DLL built for another build is refused.
- A mod already deployed through the manager into the `PATCH.POD` chain is left to it. gbhook stands down for that mod and logs `content <id>: off`.

[MOD_FORMAT.md](MOD_FORMAT.md) holds the folder rules, the id rules, the load orders and what the Mod Manager does with the folder.

A ship check that costs a minute: install the zip into a clean `mods/`, start the game, and
read the log for the `loaded` line, no `REFUSED` or `FAIL` line, and no `warning` line for
your folder.

## 16. Common failures

Each row is a line from `gbhook.log`, its cause, and the fix. The time and the `[id]` prefix
are left out, and `<x>` is a value from your mod.

### Discovery: the mod is refused

| log text | cause and fix |
|---|---|
| `REFUSED <folder>: gbhook/ exists but there is no previews/modinfo.ini` | The folder has a `gbhook/` half and no ini. Add `previews/modinfo.ini` with `id` and `abi`. |
| `REFUSED <folder>: previews/modinfo.ini has no id` | A `gbhook/` folder exists, so a DLL is meant, but the ini has no gbhook key. Add the keys. |
| `REFUSED <folder>: modinfo.ini has no id` | The ini has gbhook keys and no `id`. Add `id = <name>`. |
| `REFUSED <folder>: id '<x>' may only use a-z, 0-9 and _` | The id has capitals, spaces or dots. Rename it. |
| `REFUSED <folder>: modinfo.ini has no abi` | Add `abi = 1`. |
| `REFUSED <folder>: built for ABI <n>, this gbhook speaks ABI 1 -- rebuild the mod` | The ini or the DLL names another ABI version. Rebuild against this SDK and set `abi = 1`. |
| `REFUSED <folder>: stage '<x>' is not one of preboot, early, boot, ready` | Use a stage name. |
| `REFUSED <folder>: priority '<x>' is not a number` | Write an integer. |
| `REFUSED <folder>: modinfo.ini line <n> is not key = value` | A line has no `=`. Fix or comment it out. |
| `REFUSED <folder>: plugin '<x>.dll' is not in gbhook/` | The DLL is missing or `plugin` misspells it. Build, then check the post-build copy ran. |
| `REFUSED <folder>: plugin '<x>.dll': no 'GbhPluginManifest' export -- add GBHOOK_PLUGIN(...)` | The manifest macro is missing, or the symbol was dropped. |
| `REFUSED <folder>: plugin '<x>.dll': no export directory -- did you forget GBHOOK_PLUGIN()?` | The DLL exports nothing. Add `GBHOOK_PLUGIN` and `GBHOOK_EXPORT`. |
| `REFUSED <folder>: plugin '<x>.dll': not x64 -- gbhook is 64-bit only` | Build for `x64`. |
| `REFUSED <folder>: plugin '<x>.dll': built for ghost.exe <md5>, this framework targets 0b89556c07e5b737efe444351227e747 -- an offset table applied to the wrong build crashes unreadably` | The SDK headers are from another build. Rebuild with this repo's SDK. |
| `REFUSED <folder>: plugin '<x>.dll': manifest magic mismatch (stale SDK?)` | The DLL was built against an old header. Rebuild. |
| `REFUSED <folder>: modinfo.ini says id '<a>' but <x>.dll says '<b>' -- one was edited after the build` | `id` in the ini and in `GBHOOK_PLUGIN` differ. Make them equal. |
| `REFUSED <folder>: duplicate id '<x>', already claimed by folder '<y>'` | Two folders share an id. Rename one. |
| `REFUSED <folder>: requires '<x>', which is not present` | The needed mod is absent. The text says `was refused` or `is disabled in gbhook.ini` for those cases. |
| `REFUSED <folder>: folder name '<x>' has no letter or digit: set an id in previews/modinfo.ini` | A content mod with a folder name like `!!!`. Set an `id`. |

### Load: the mod is inert

| log text | cause and fix |
|---|---|
| `MODS FAIL <id>: LoadLibrary failed (error <n>; a missing dependency?) -- the mod is inert` | The DLL imports something Windows cannot find. Link the CRT statically and avoid extra DLLs. |
| `MODS FAIL <id>: the DLL has no GbhPluginInit export -- the mod is inert` | Export `GbhPluginInit` with `extern "C"` and `GBHOOK_EXPORT`. |
| `MODS FAIL <id>: GbhPluginInit returned <n> -- the mod is inert` | Init returned an error. Check the ABI version test and any call whose result you propagate. |
| `MODS FAIL <id>: faulted inside GbhPluginInit -- the mod is inert` | A crash in init. Read the `FAULT` lines just above. |
| `MODS warning <folder>: requires '<x>', which initialises after it -- give this mod a higher priority or a later stage` | `requires` orders nothing. Raise `priority` or move to a later `stage`. |
| `MODS warning <folder>: gbhook/mod.ini is no longer read: its keys go in previews/modinfo.ini` | Move the keys and delete the file. |

### Running

| log text | cause and fix |
|---|---|
| `EVT FAULT in '<id>' <event> callback -- that subscription is off for this session; the game continues` | The callback crashed once and is dropped for the run. Find the `FAULT` line above it. |
| `EVT BUDGET '<id>' averages <n>us per frame over 600 calls (soft limit 250us) -- this is on the engine's critical path` | A frame callback is too slow. Do less per frame. |
| `EVT '<id>' not subscribed: <event> bus full (64)` | The event has 64 subscribers. |
| `HOOK CONFLICT ghost+0x<a>: '<id>' wants it, 'gbhook' already owns it -- refusing (declare it in your manifest)` | The address is one gbhook hooks itself. Subscribe to the event it feeds. |
| `HOOK ghost+0x<a> already hooked by '<x>'; '<id>' refused (one detour per address -- subscribe to an event instead)` | Another mod holds the address. Pick another site or share through a service. |
| `HOOK CONFLICT patch ghost+0x<a>: '<id>' wants it inside the prologue '<x>' detoured at <p> -- refusing` | A patch overlaps a hook's first 20 bytes. Move the patch or the hook. |
| `HOOK patch ghost+0x<a> refused for '<id>': overlaps the patch at 0x<b> held by '<x>'` | Two patches overlap. |
| `HOOK TAMPER ghost+0x<a> (owner '<id>') changed since install -- checked after <when>. Something else is patching this address; expect undefined behaviour.` | Another module rewrote the hooked bytes. A second hooking library in the process is the usual cause. |
| `CMD '<name>' from '<id>' refused: '<id>.<name>' is already registered by <x>` | The command exists. Rename or register once. |
| `` ERR <line>: unknown command (try `help`) `` | Commands are `<id>.<name>`. Run `help` for the list. |
| `ERR <line>: queue full (64), dropped` | More than 64 queued command lines. Space them out. |
| `BIND <action> wants <chord>, held by <x> <action>; unbound` | Another action holds the chord. Bind a different one in `gbhook.ini`. |
| `BIND <action>: '<x>' is not a key or chord; unbound` | The `bind.<action>` value does not parse. Use `CTRL+SHIFT+F5` form. |
| `NMENU row <n> already claimed by '<x>'; '<id>' refused` | One mod per row. Use `mods_page_add` instead. |
| `NMENU row <n> is the game's own <key> row; '<id>' refused` | Only `GBH_ROW_ONLINE` and `GBH_ROW_FREE` can be claimed. |
| `MODS a second Mods page refused: one per mod` | Call `mods_page_add` once. |
| `SVC <id> could not publish: '<id>.<name>' is already published by <x>` | Publish a name once. |
| `FILE read: '..' refused: <path>` | Paths are archive-relative. Remove the `..`. |
| `LEVEL '<x>' is not a checkpoint the live level registered; it has: <list>` | Use a name from the list. |
| `LEVEL prepare '<stem>' FAILED`, followed by `LEVEL prepare error <i>/<n>: <text>` | The engine rejected the level. The error lines name the missing asset or bad data. |
| `GTFO script fault (<code>): <message>` | A Dante script error. The message names the call. |
| `HUD no level is live; "<text>" dropped` | `hud` needs a level. Call it from a level callback. |
| `INI warning <id>: modinfo.ini key '<id>.<x>' repeats the mod id; it is read as '<id>.<id>.<x>'` | Drop the id from the key. |

### Content

| log text | cause and fix |
|---|---|
| `MODS content <id>: left out <names> -- not engine content` | A top-level folder or file is not one of the twelve asset roots. Move it under one. |
| `MODS content <id>: left out <path> -- not a file type the engine reads` | The extension is not on the list in section 2. |
| `MODS CONFLICT content <path> shipped by both '<a>' and '<b>' -- code order decides, silently` | Two mods ship one path. Rename yours or declare an order with stage and priority. |
| `MODS content <id>: build FAILED -- <why>` | The archive could not be written. The text after `FAILED` names the step. |
| `MODS content <id>: mount FAILED -- <why>` | The engine refused the archive. The `POD` lines above it carry the engine's reason. |
| `MODS content <id>: cached <name> is unusable (<why>), rebuilding` | A cache file was cut short. gbhook rebuilds it. |
| `MODS content <id>: off -- already in a chained POD -- the Mod Manager deployed it, gbhook stands down` | The manager already deployed every file. Nothing to fix. |
