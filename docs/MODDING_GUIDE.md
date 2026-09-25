# Modding guide

Two kinds of mod, one folder format. A content mod is loose assets and a `modinfo.ini`. A
code mod adds a DLL on gbhook's C ABI. Both live at `<gamedir>/mods/<name>/`, and
[MOD_FORMAT.md](MOD_FORMAT.md) is the reference for the folder and the ini.

```
1. a content mod      a level in a folder, and nothing to build
2. a code mod         a DLL: manifest, init, the stages
3. the ABI in use     log, settings, commands, events, hooks, the game model
4. the native menu    a row on the game's main menu and pages under it
5. testing            the log, the command file, the harness
```

---

## 1. A content mod

```
mods/DuelArena/
├── previews/modinfo.ini          the Mod Manager's metadata, and gbhook's keys
├── world/duel_arena.lvl          the level
├── world/duel_arena.dante        its script
├── world/en/duel_arena.txt       its text
├── sets/duel.bst
├── data/biped1/fiend_wb2.cib
└── art/lightmap/duel_arena/...
```

```ini
# previews/modinfo.ini
version="1.0.1"
compatibility="PC"
description="Duel Arena -- a bare 200x200ft test range for 1-on-1 play."
link=""

id    = duelarena
abi   = 1
stage = boot
```

That is the whole mod. At boot gbhook hashes the tree, builds it into
`gbhook/cache/duelarena/<hash>.POD` the first time and after any change, and mounts the
archive above the game's own. The level appears under **Mods ▸ Load Level ▸ Custom**, and
the checkpoints its script registers with `defineCheckpoint` appear when it is chosen.

What to know:

- Paths mirror the game's archives, backslash form inside the engine: `world\duel_arena.lvl`.
- The front end's strings are read before the mount, so `world\en\ui.txt` cannot be changed
  this way. Per-level text can.
- Two mods shipping the same path are named in the log. Mount order decides silently.
- `gbhook/` and `previews/` are never packed.
- The log's `MODS` lines say built, cached or off, per mod.

## 2. A code mod

### The minimum

```cpp
// src/MyMod.cpp
#include "gbhook/gbhook.h"
#include "gbhook/gbhook.hpp"

GBHOOK_PLUGIN("mymod");

extern "C" GBHOOK_EXPORT int GbhPluginInit(const GbhApi* api)
{
    if (!api || api->abi_version != GBHOOK_ABI_VERSION) return GBH_ERR;
    gbh::bind(api);
    gbh::log("MOD", "hello");
    return GBH_OK;
}
```

```ini
# previews/modinfo.ini
version="0.1.0"
compatibility="PC"
description="A small example mod."
link=""

id     = mymod
abi    = 1
plugin = MyMod.dll
stage  = boot
```

`GBHOOK_PLUGIN` places the manifest in the DLL as exported data. gbhook reads it out of the
file without running the DLL and checks the id against `modinfo.ini`, the ABI version, and the
target `ghost.exe` md5 before anything is loaded. Do no work in `DllMain`: it runs under the
loader lock, before the framework can talk to you. `GbhPluginInit` is the entry point, and
anything but `GBH_OK` leaves the mod inert.

### Building

A project file imports the shared settings and names the sources. `examples/MyMod/MyMod.vcxproj`
is the template:

```xml
<Import Project="..\..\sdk\GbHookPlugin.props" />
<ItemGroup>
  <ClCompile Include="src\MyMod.cpp" />
</ItemGroup>
```

```
MSBuild.exe MyMod.vcxproj -p:Configuration=Release -p:Platform=x64
```

The props file sets C++20, the include paths, and the static CRT. The static CRT is not a
style choice: every module owns its own heap, so nothing may be allocated on one side of
the ABI and freed on the other. The table's `alloc`, `realloc` and `free` exist for the
few buffers that do cross. A post-build step copies the DLL into `gbhook/`, so the mod
folder installs as it is.

### Stages

`stage` in `modinfo.ini` says when `GbhPluginInit` runs. Within a stage, `priority` orders,
low first.

| stage | when | for |
|---|---|---|
| `preboot` | before any framework service, about a second before the engine's boot screens | a detour that must win a race with the engine, such as a boot-screen skip |
| `early` | core services up, before any level can load | a guard around the level loader |
| `boot` | the default | everything else |
| `ready` | after the native menu broker | a mod that reads other mods' state |

Menu row claims made at `boot` are recorded and go live when the broker installs, so
`ready` is rarely needed.

### Exclusive hooks

A mod that detours an engine function nobody else may touch declares it in the manifest,
and the claim is checked against every other mod at discovery:

```cpp
GBHOOK_PLUGIN_EXCLUSIVE("fastboot") { "ghost+0x248190", "ghost+0x2487E0", "" } GBHOOK_PLUGIN_END;
```

Two mods claiming one address: whichever loads first wins, and the other's claim is refused
by name in the log.

## 3. The ABI in use

Everything below is `gbh::` from `gbhook.hpp`, thin inline wrappers over the table. The
table itself is `gbhook.h`, and [ABI.md](ABI.md) lists every entry with its thread rule.

### Log and settings

```cpp
gbh::log("MOD", "one line");
gbh::logf("MOD", "%d things", n);
```

Lines land in `gbhook.log` as `TAG  [mymod] text`. The tag names the subsystem; the id
is added by gbhook.

```cpp
const bool  on   = gbh::setting_bool("bootskip", true);
const int   rate = gbh::setting_int("spawn_rate", 4);
const auto  font = gbh::setting("overlay.font", "");
```

Keys are read as `mymod.<key>` from `gbhook.ini`, then from the same key in the mod's own
`modinfo.ini`, then the default given. A mod ships its defaults in `modinfo.ini` and the user
overrides them in `gbhook.ini`.

### Commands

```cpp
int CmdHello(int argc, const char* const* argv, const char** err, void*)
{
    if (argc < 1) { *err = "usage: hello <name>"; return GBH_ERR_ARG; }
    gbh::logf("RES", "hello %s", argv[0]);
    return GBH_OK;
}

gbh::command_register("hello", CmdHello, nullptr, "<name> -- say hello");
```

The command is `mymod.hello`, reachable from `gbhook.cmd`, from other mods and from
gbhook. `argv` holds the arguments only. Every handler runs on the game thread, so engine
calls are safe inside it. `gbh::run(line)` runs a command now, or queues it when the caller
is not on the game thread; `gbh::queue(line)` always waits for the next frame, front end
included.

### Events

```cpp
void OnFrame(void*)                                        { /* every frame in a level */ }
void OnLevel(int phase, const char* level, int ok, void*)  { /* prepare begin and end */ }
void OnActor(const char* cls, const char* name, void* ptr, void*) { /* each VM global */ }

gbh::on_frame(OnFrame).release();
gbh::on_level(OnLevel).release();
gbh::on_actor_registered(OnActor).release();
```

`on_pump` is the main thread every frame, the front end included; `on_frame` is the
per-level tick. All run on the game thread. Every callback runs under a guard: the first
fault names the mod and the event in the log, drops that one subscription for the process,
and the game continues. The frame bus is on the engine's critical path and is budgeted; a
subscriber averaging over the budget is named in the log every 600 frames.

`release()` keeps a subscription for the life of the process, which is the usual case. A
`gbh::Sub` that goes out of scope unsubscribes.

### Hooks, patches, vtables

gbhook owns the one MinHook in the process. A mod never links its own.

```cpp
typedef __int64 (__fastcall* tFn)(void*, void*, void*, void*);
tFn oLegal = nullptr;
__int64 __fastcall LegalDetour(void*, void*, void*, void*) { return 0; }

GbhHook h = gbh::hook_at(HookTargets::legalScreen, (void*)&LegalDetour, (void**)&oLegal,
                         GBH_HOOK_DEFERRED | GBH_HOOK_EXCLUSIVE);
gbh::enable_batch(&h, 1);
```

`GBH_HOOK_DEFERRED` creates without enabling, and `enable_batch` arms a set together, so a
pair of hooks never runs half-armed. `GBH_HOOK_EXCLUSIVE` refuses the target if anyone
already has it, and matches a manifest claim. `sdk/include/gb/HookTargets.h` names the
known detour sites; the contended ones, the tick, VM registration, level prepare, are
gbhook's and reach mods as events. Never hook those yourself.

A byte patch is recorded and re-verified:

```cpp
static const uint8_t nop2[] = { 0x90, 0x90 };
gbh::Patch p(gbh::at(0x1234), nop2, sizeof nop2);
p.release();                       // else it reverts when p dies
```

A vtable override is a copy of the shipped table with slots claimed per mod:

```cpp
gbh::VtableOverride vt(gbh::at(0x7CA868), 116);
vt.set(83, &MyActivate, &oActivate);
vt.apply(object);                  // the object's vptr now points at the copy
```

Apply only to objects that live for the process. The integrity sweep re-verifies every
hook, patch and applied vptr, and `hooks` in `gbhook.cmd` runs it on demand; drift is
logged as tamper, with the mod named where it can be.

### The game model

```cpp
void* game = gbh::game_singleton();       // CGame*, NULL very early
void* hero = gbh::local_player();         // NULL in menus and during loads
std::string stem = gbh::level_name();     // "" at the front end
```

The registry is every Dante VM global the level registered, class and name to pointer:

```cpp
for (const GbhRegistryEntry& e : gbh::registry()) { /* e.cls, e.name, e.ptr */ }
GbhRegistryEntry egon;
if (gbh::registry_find("Egon", egon)) { /* exact match, then substring */ }
```

Pointers are live game memory, game thread only, and stale after the next level prepare;
`generation` says which level they belong to.

The engine's own list is wider: every `CActor` in the level, the spawn pool included, with
the exact class read out of the RTTI:

```cpp
for (const GbhActorInfo& a : gbh::actors())
{
    const bool live = a.flags & GBH_ACTOR_ENABLED;      // clear: pooled, built and switched off
    const bool ghost = a.flags & GBH_ACTOR_GHOST;
    /* a.name, a.cls, a.pos, a.orient, a.team, a.ptr */
}
GbhActorInfo slimer;
if (gbh::actor_find("Slimer", slimer) && gbh::actor_is_a(slimer.ptr, "CCharacter")) { /* ... */ }
```

The snapshot walks the list under a guard and is not per-frame cheap: rebuild a cache on
the game thread when something asks, and read the cache from anywhere else.

Objective engine state has a key, read out of memory and set through the engine's native,
so every mod sees the same value whoever changed it last:

```cpp
bool god = false;
if (gbh::attr_bool("god", god)) { /* the live flag, whatever set it */ }
gbh::attr_set("god", "toggle");           // game thread: a command handler or on_frame
gbh::attr_set("gravity", "-1.6");         // in range, or GBH_ERR_ARG
gbh::attr_set("time", "reset");
gbh::attr("fov");                         // "23.00 deg"; read-only, attr_set answers GBH_ERR_UNSUPPORTED
for (const GbhAttrInfo& a : gbh::attrs()) { /* a.key, a.type, a.writable, a.min, a.max, a.unit, a.help */ }
```

`attr` in `gbhook.cmd` reads and sets the same keys. A toggle the engine has no getter for
stays the mod's own state; it never becomes a key.

`gbh::read<T>(src, out)` is the guarded read behind the rest, for the field a mod reads on
its own: `0` on an unmapped or torn pointer, never a crash.

The engine's script-visible methods are in `sdk/include/gb/Generated/GBApi.generated.h`,
typed C++ wrappers that call each method through the engine's own thunk:

```cpp
#include "gb/Generated/GBApi.generated.h"
char* gameBase = nullptr;                 // the header's base symbol; set it from game_base()

GB::CCharacter::setInvulnerableFlag(hero, true);
```

### Levels, input, HUD, files

```cpp
gbh::level("duel_arena");                            // from the front end or in play
gbh::level("library1b", "checkpoint_OldStacks");     // a checkpoint the level's script registers
gbh::key(gbh::dik("SPACE"), true);                   // the engine's own scan-code table
gbh::hud("Objective updated", 3.0f);                 // the HUD message line, in a level
std::vector<std::string> lvls = gbh::files("world", "*.lvl");
std::vector<unsigned char> bytes = gbh::file_read("world\\duel_arena.dante");
```

The file calls go through the engine's own resolution, so a mod's archive and the game's
answer alike. They run on the engine's main thread only: a command handler, `on_frame` or
`on_pump`. So do the level lists, the ones the Mods page shows:

```cpp
std::vector<std::string> career = gbh::levels(GBH_LEVELS_CAREER);      // the engine's table, in order
std::vector<std::string> custom = gbh::levels(GBH_LEVELS_CUSTOM);      // every other .lvl an archive holds
std::vector<std::string> cps    = gbh::checkpoints("library1b");        // what its script registers
```

### Keys

The game window is subclassed once, by gbhook. A mod that owns a key while some mode is on
subscribes and answers `1`, which keeps the key from the engine's own handler:

```cpp
int OnKey(int vk, int down, void*)
{
    return g_flying && (vk == 'W' || vk == 'A' || vk == 'S' || vk == 'D') ? 1 : 0;
}
gbh::on_key(OnKey).release();     // message thread; the first subscriber answering 1 ends the fan-out
```

Order is registration order, so a menu that loads first sees a key before a mod that flies.
`on_char` carries typed characters the same way. Never subclass the window yourself.

### Actions

A key the player should be able to rebind is an action. The mod names it and the player
binds it:

```cpp
void OnMenu(const char* name, void*) { g_open = !g_open; }
gbh::action_register("menu", OnMenu, nullptr, "open the overlay");
```

```ini
# previews/modinfo.ini
bind.menu = F1
```

The player rebinds it in `gbhook.ini` as `mymod.bind.menu = CTRL+SHIFT+M`, or turns it off
with `NONE`. The handler runs on the main thread once per press, and a bound key never
reaches the game. If another mod already has the chord, the log names both and yours stays
unbound. `gbh::action_held("menu")` answers while the chord is down. `gbh::action_capture(true)`
lets a menu take the keyboard: only its own actions fire until it lets go. `binds` lists every
action and its chord.

### Pausing the world

An overlay that should stop the game holds the freeze while it is open:

```cpp
gbh::pause(true);     // the world and the game clock stop; drawing and audio carry on
gbh::pause(false);    // the world ticks again once no mod holds it
```

Esc still opens the game's own pause menu during the freeze. `gbh::paused()` says why the
world is stopped, and `gbh::world_to_screen(pos, out)` turns a world position into pixels
for a marker.

### Services

A mod that offers a table to other mods publishes it by name, and a consumer finds it:

```cpp
struct MyTable { uint32_t struct_size; int (*answer)(void); };
static const MyTable g_table = { sizeof g_table, Answer };
gbh::service_publish("mymod.table", &g_table, sizeof g_table);      // once; a second name is refused

const MyTable* t = gbh::service_find<MyTable>("mymod.table");      // null until the publisher has loaded
```

The table lives for the process and starts with `struct_size`, so a consumer gates on what
it carries the way `gbh_api_has` does. A consumer loads after its publisher: a later stage,
or a higher `priority` in the same stage. `mods/gbmenu` is the first: the ImGui overlay, one
per process, publishing `gb.menu.ui` so every mod's pages share one keyboard and one frame.

## 4. The native menu

The game's own main menu has seven rows. Two are claimable, Online and a free slot, hidden
until a mod claims them. A claimed row is un-hidden and its activation routed to the mod,
and from there a real page can be pushed, built on the engine's own screen class.

```cpp
int g_count = 0;

void Build(void*)
{
    char c[GBH_NATIVE_LABEL_CAP];
    snprintf(c, sizeof c, "Count: %d", g_count);
    gbh::native_submenu_add_row("Log a line", 1);
    gbh::native_submenu_add_row(c, 2);
    gbh::native_submenu_add_row("A header", GBH_NATIVE_INERT);
    gbh::native_submenu_add_row("Close", 3);
}

int Activate(int action, void*)
{
    if (action == 1) { gbh::log("MENU", "row activated"); return GBH_NATIVE_STAY; }
    if (action == 2) { ++g_count; return GBH_NATIVE_STAY; }
    return GBH_NATIVE_CLOSE;
}

void OnRow(int, void*)
{
    GbhNativeMenuDesc d = { sizeof d, Build, Activate, nullptr, "MyMod" };
    gbh::native_submenu_open(d);
}

gbh::native_row_claim(GBH_ROW_ONLINE, OnRow);
gbh::native_row_label(GBH_ROW_ONLINE, "MyMod");
```

`examples/MenuDemo` is this page, complete. A page is data: `build` publishes labels with actions, `activate`
answers with a verdict.
`GBH_NATIVE_STAY` rebuilds the page, and a changed label is relabelled in place.
`GBH_NATIVE_CLOSE` pops it. `GBH_NATIVE_CLOSE_ALL` pops every page back to the main menu,
which is what a row that starts a level load wants. A page may open another from inside
its `activate`; ESC pops one page. Labels are at most 39 characters, a title may be a
localisation key with a leading `@`, and everything runs on the game thread.

## 5. Testing

`gbhook.log` is the record of every run. Tags to look for: `BOOT`, `MODS` and `HOOK` at
startup, `LEVEL` and `REG` around a load, `EVT` for the buses, `NMENU` for the menu,
`FAULT` for any first-chance fault with the module named and the stack walked, `TAMPER`
when a hook's bytes drifted.

`gbhook.cmd` in the game folder is polled while the game runs: one command per line, and
the file is renamed to `.cmd.run` before it is read so a script writing the next batch
cannot race this one. Replies are `OK`, `RES` and `ERR` lines in the log.

`tools/harness/gb.sh` wraps the loop: `build`, `install`, `start`, `stop`, `cmd "<line>"`,
`wait "<regex>"`, `log`, `shot`, `keys`. Its README says how.

`examples/SelfTest` is the ABI's in-game self-test. Every group of the table is exercised against
the mod's own memory, and each check is a `PASS` or `FAIL` line under `TEST`. When a call
does not behave as this guide says, that mod is the first place to look.
