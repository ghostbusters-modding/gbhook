# The ABI

`sdk/include/gbhook/gbhook.h` is the ABI contract: pure C, one header, a manifest a mod
exports as data, one entry point, and a table of function pointers handed to it. This document
is meant to explain that contract.

```
GbhPluginManifest   exported data, read out of the file before the DLL runs
GbhPluginInit       exported code, called once at the mod's stage with the table
GbhApi              78 entries in 24 groups, appended to and never reordered
```

---

## 1. Versioning

`GBHOOK_ABI_VERSION` is bumped only by a breaking change. A mod built against another
version is refused at discovery, from its manifest, before it is loaded.

Within a version the table only grows. Every entry keeps its slot, new ones are appended,
and `struct_size` says how far the running framework's table reaches:

```c
if (gbh_api_has(api, file_read)) ...    /* true when the framework carries the entry */
```

`framework_version()` is `(major << 16) | (minor << 8) | patch` for logging. It is not a
compatibility signal; `struct_size` is.

`GBHOOK_TARGET_MD5` names the one `ghost.exe` build every offset was verified against. A
mod's manifest carries it, and a mod naming another build is refused.

## 2. The manifest

```c
GBHOOK_PLUGIN("gb.mymod");
GBHOOK_PLUGIN_EXCLUSIVE("gb.fastboot") { "ghost+0x248190", "ghost+0x2487E0", "" } GBHOOK_PLUGIN_END;
```

Exactly one translation unit per DLL. The manifest is exported as `GbhPluginManifest`, a
`GbhManifest` with inline arrays so it can be read from the file by walking the export
directory, with no code executed. It states what only the binary can assert: `id` and
`abi_version`, both cross-checked against `modinfo.ini`; the target md5; and up to sixteen
exclusive hook claims as `"ghost+0xHEX"` strings, terminated by an empty one.

## 3. Rules

**Threads.** Each group below names its thread. "Game thread" is the engine's main thread,
which ticks the level and pumps the front end; `is_game_thread()` answers for the caller.
Command handlers are routed there by the framework. Callbacks run there. The file calls
refuse any other thread with `GBH_ERR_WRONG_THREAD`. The rest are safe from anywhere and
say so.

**Memory.** Every mod is built with the static CRT, so each module owns its own heap and
nothing may be allocated on one side of the table and freed on the other. Every buffer a
call fills is the caller's, and nothing transfers ownership. `alloc`, `realloc` and `free`
are the framework's heap, for the rare thing handed across.

**Strings.** A `const char*` a call returns is the framework's, valid until this thread's
next call into the table. Copy it if it must outlive that.

**The caller.** No entry takes a mod handle. The framework derives the calling mod from the
return address, which is what attributes a log line, a hook or a claim to its owner.

**Faults.** Every callback runs under a guard. The first fault names the mod and the event
in the log, drops that one subscription for the process, and the game continues. A fault
inside `GbhPluginInit` leaves the mod inert. The framework never unloads a DLL: a detour
executing on another thread while its module is torn out is not recoverable.

**Status.** Every `int` entry answers with `GbhStatus`. `GBH_OK` is zero.

| status | meaning |
|---|---|
| `GBH_ERR` | unspecified |
| `GBH_ERR_ARG` | a null or out-of-range argument |
| `GBH_ERR_STATE` | not valid now: no level, too early, a page not open |
| `GBH_ERR_UNSUPPORTED` | the framework is older than this call, or the key is read-only |
| `GBH_ERR_CONFLICT` | another mod owns it |
| `GBH_ERR_NOT_FOUND` | |
| `GBH_ERR_WRONG_THREAD` | must be called on the game thread |
| `GBH_ERR_TRUNCATED` | the output buffer was too small; what fits is valid |

## 4. The table

### Environment, any thread

| entry | answers |
|---|---|
| `game_base()` | the `ghost.exe` module base; every offset in `sdk/include/gb` is relative to it |
| `game_dir()` | the folder holding `ghost.exe`, no trailing separator |
| `mod_dir()` | this mod's `gbhook/` folder |
| `framework_version()` | packed major, minor, patch |

### Memory, any thread

`alloc(n)`, `realloc(p, n)`, `free(p)`: the framework heap. Use it only for memory that
crosses the table.

### Log, any thread

`log(tag, line)`, `logf(tag, fmt, ...)`. Lines go to `<gamedir>\gbhook.log` as
`TAG  [id] text`; the tag names the subsystem and the id is added by the framework.

### Settings, any thread

`setting(key, dflt)`, `setting_int`, `setting_float`, `setting_bool`. A key is resolved as
`<id>.<key>` in `gbhook.ini`, then the same key in the mod's own `modinfo.ini`, then `dflt`.

### Hooks, any thread, install at init

| entry | notes |
|---|---|
| `hook_create(target, detour, original, flags)` | `target` is absolute; `original` receives the trampoline. One detour per address across every mod. |
| `hook_enable(h)`, `hook_disable(h)` | |
| `hook_enable_batch(hooks, n)` | arms a set together, so no window exists in which half of them are live |

`GBH_HOOK_DEFERRED` creates without enabling. `GBH_HOOK_EXCLUSIVE` refuses if anyone
already holds the target, and matches a manifest claim. The contended sites, the per-level
tick, VM registration, level prepare and the pump, are the framework's and reach mods as
events; a mod asking for one of them is refused.

### Patches, any thread

`patch_write(at, bytes, n)` records a byte patch of at most `GBH_MAX_PATCH_BYTES`,
arbitrated against every detour so a patch never lands inside a hooked prologue, and
re-verified by the integrity sweep. `patch_revert(p)` restores the bytes.

### Vtables, any thread

| entry | notes |
|---|---|
| `vtable_clone(original, slots)` | one copy per shipped table, shared by every mod that claims a slot in it |
| `vtable_slot(clone, slot, fn, original_fn)` | claims a slot; a second claimant is refused |
| `vtable_apply(object, clone)` | points the object at the copy; the swap is recorded and re-verified |
| `vtable_original(clone, slot)` | the shipped entry |

Apply only to objects that live for the process. The sweep re-reads every applied vptr, so
an object freed and reused reads as tamper.

### Introspection, any thread

`mod_count()`, `mod_id_at(i)`, `mod_is_loaded(id)`: every mod discovery saw, accepted ones
first in code order.

### Events, callbacks on the game thread

| entry | fires |
|---|---|
| `on_frame(fn, user)` | once per frame while a level is live. On the engine's critical path and budgeted: a subscriber averaging over 250 µs is named every 600 frames |
| `on_pump(fn, user)` | once per frame on the main thread, the front end included |
| `on_level(fn, user)` | `GBH_LEVEL_PREPARE_BEGIN` and `_END` with the stem and, at the end, whether the engine accepted the level |
| `on_actor_registered(fn, user)` | during a level load, once per Dante VM global: class, name, pointer |
| `unsubscribe(s)` | idempotent; a stale or null handle is ignored |

Order is registration order. A callback may subscribe, unsubscribe or log; the table is
read under the lock and dispatched outside it.

### The game model, guarded reads, any thread

| entry | answers |
|---|---|
| `game_singleton()` | `CGame*`, null very early |
| `local_player()` | null in menus and during loads |
| `game_thread_id()` | zero until the first frame |
| `is_game_thread()` | |
| `level_name()` | the stem, `""` at the front end |
| `registry_snapshot(buf, cap)` | this level's VM globals; a null `buf` with `cap` zero answers the total |
| `registry_find(name, out)` | exact, then substring, case-insensitive |

A `GbhRegistryEntry` pointer is live game memory: game thread only, stale after the next
level prepare, and `generation` says which prepare it belongs to.

### Commands

| entry | thread | notes |
|---|---|---|
| `command_register(name, fn, user, help)` | any | registered as `<id>.<name>`; the handler always runs on the game thread |
| `command_run(line)` | any | now on the game thread, queued from anywhere else |
| `command_queue(line)` | any | at the next frame, the front end included |

A handler receives its arguments only, returns `GBH_OK`, or points `*err` at a static
string and returns a status.

### Input, any thread

`input_set_key(dik, down)` writes the engine's own scan-code table, the one the game reads;
`SendInput` never reaches it. A key stays down until it is released, like a real key.
`input_dik_from_name("LSHIFT")` maps a name, `-1` when unknown.

### HUD, any thread, in a level

`hud_message(text, seconds)`: the engine's message line at the top of the screen.

### Level flow, any thread

`level_chain(level, checkpoint)`: a stem or a `.lvl` name, from the front end or in play.
The checkpoint is a script name, `checkpoint_OldStacks`, or the display name the level
registers it under, and it is armed at the level's begin, after its script has registered
its checkpoints. A name the level does not register is logged with the ones it does, and
the level starts from the top.

### The native menu, game thread

| entry | notes |
|---|---|
| `native_row_claim(row, fn, user)` | `GBH_ROW_ONLINE` or `GBH_ROW_FREE`; the game's own rows are refused with `GBH_ERR_CONFLICT`, a second claimant by name |
| `native_row_label(row, label)` | your row's text, verbatim, re-applied after every refill of the menu |
| `native_submenu_open(desc)` | pushes a page: from inside your row callback, or a page's `activate` to nest |
| `native_submenu_add_row(label, action)` | from inside `build()` only; at most `GBH_NATIVE_MAX_ROWS`, labels under `GBH_NATIVE_LABEL_CAP` |
| `native_submenu_refresh()` | rebuild and re-label the open page; cheap when unchanged |

`GbhNativeMenuDesc` is `struct_size`, `build`, `activate`, `user` and `title`. A title
with a leading `@` names a localisation key. `activate` answers `GBH_NATIVE_STAY`,
`GBH_NATIVE_CLOSE` or `GBH_NATIVE_CLOSE_ALL`; a row with action `GBH_NATIVE_INERT` never
activates.

### Files, engine main thread only

| entry | notes |
|---|---|
| `file_list(dir, pattern, cb, user)` | every mounted archive walked; bare names, deduplicated, sorted; returns the count |
| `file_read(path, buf, cap)` | through the engine's own resolution chain, decompressed; a null `buf` with `cap` zero answers the size |

A negative return is a status. Any thread but the engine's main thread gets
`GBH_ERR_WRONG_THREAD`: the engine's file tables take no lock.

### Services, any thread

| entry | notes |
|---|---|
| `service_publish(name, table, size)` | one publisher per name; a second answers `GBH_ERR_CONFLICT`. The table lives for the process |
| `service_find(name, &size)` | null until published; `size` is what the publisher stated |
| `service_count()`, `service_name_at(i)`, `service_owner(name)` | the directory; the owner is the publishing mod's id |

A service is a C struct of function pointers one mod owns and others call, the way this
table is: its first field is a `uint32_t struct_size`, so a consumer gates on what it carries
the way `gbh_api_has` does. Name it under the publisher's id, `gb.menu.ui`. A mod that needs
another's table loads after it: a later stage, or a higher `priority` in the same stage.
`gbh::service_find<T>(name)` is the typed wrapper.

### Memory, any thread

`mem_read(src, dst, n)` copies out of game memory under a guard: `1` when copied, `0` when
the range is unmapped or the read faulted. The one primitive behind every "guarded read" a
mod used to carry itself.

### Levels, engine main thread only

| entry | notes |
|---|---|
| `level_list(kind, cb, user)` | `GBH_LEVELS_CAREER`: the engine's own table in its order; `GBH_LEVELS_CUSTOM`: every other `world\*.lvl` a mounted archive holds, sorted. Stems, no `.lvl` |
| `level_checkpoints(stem, cb, user)` | what `world\<stem>.dante` registers, in script order; none when the script is unreadable |

The same lists the Mods page shows, and the same thread rule as the file calls, for the
same reason. `level_chain` loads one.

### Actors, guarded reads, any thread

| entry | notes |
|---|---|
| `actor_snapshot(buf, cap)` | the engine's own list of every `CActor` in the level; a null `buf` with `cap` zero answers the total |
| `actor_find(name, out)` | exact, then substring, case-insensitive, on the engine-list name |
| `actor_is_a(actor, cls)` | `1` or `0` by the RTTI chain; `GBH_ERR_STATE` when the object is unreadable |

`GbhActorInfo` starts with a `struct_size` the caller sets in `buf[0]` (and in `out`): it is
the stride and how much of each entry is filled, so the struct can grow. `flags` says whether
the engine has the actor enabled (clear is the spawn pool, fully built and switched off) and
which of the common bases its class derives from; `cls` is the exact RTTI name. This is the
VM registry's superset: the registry sees what the script exported, the list sees everything.
A `ptr` is live game memory: game thread only, stale after the next level prepare.

### Attributes, reads any thread, writes on the game thread

| entry | notes |
|---|---|
| `attr_count()`, `attr_at(i, out)` | the catalogue: key, type, whether writable, range, unit, help |
| `attr_get(key, out, cap)` | the display form, `ON`, `-32.00`, `1.00x`; `GBH_ERR_STATE` when it cannot be read now |
| `attr_get_float(key, out)` | the number; a bool as `0` or `1` |
| `attr_set(key, value)` | `on`, `off`, `toggle`, a number in range, or `reset` where the engine has one |

| key | type | set through | notes |
|---|---|---|---|
| `god` | bool | `CCharacter::setInvulnerableFlag` | |
| `giant` | bool | `CGhostbuster::enableGiantBossMode` | the Stay Puft camera framing |
| `torpedo` | bool | `CGhostbuster::enableProtonTorpedo` | |
| `hunt` | bool | `CGhostbuster::toggleHuntMode` | |
| `gravity` | float | `Global::setGravity`, `reset` | the y component; `-32` is the engine's normal |
| `time` | float, `x` | `Global::setTimeFactor`, `reset` | `0.05` to `4`; `1.00` on the engine's own curve |
| `fov` | float, `deg` | read-only | `23` normal, `18` aiming |
| `camdist` | float | read-only | the third-person follow distance |
| `cammode` | int | read-only | `0` normal, `0xA` path, `0xB` fixed, `0xD` orbit |

Every key is read out of the engine's own memory, never remembered, so it survives a level
load or a script changing it behind a mod's back. A setter goes through the engine's native.
A toggle the engine exposes no getter for (fly mode, letterbox, the HUD) is a mod's own state
and does not belong here.

### Keys, message thread

| entry | fires |
|---|---|
| `on_key(fn, user)` | every `WM_KEYDOWN` and `WM_KEYUP` on the game window, virtual-key code and direction |
| `on_char(fn, user)` | every `WM_CHAR`, the typed character |

The game window is subclassed once, by the framework. A subscriber answering `1` keeps the
message from the engine's own handler, which is what fills its scan-code table, and the
fan-out stops there; order is registration order, so a menu that loads first sees a key
before a mod that flies. The mouse is never routed. `input_set_key` is the other direction.
Raw subscribers run before actions, so a text line that keeps typed keys also keeps them
from firing actions.

### Actions, main thread

| entry | notes |
|---|---|
| `action_register(name, fn, user, help)` | `GBH_OK` whether it bound or not; `GBH_ERR_CONFLICT` when this mod already has the name; `GBH_ERR_ARG` for an empty name, whitespace, or over 31 characters |
| `action_binding(name, out, cap)` | the chord as text, `CTRL+SHIFT+F5`, or `""` when unbound |
| `action_held(name)` | `1` while every key of the chord is down; `GBH_ERR_NOT_FOUND` for a name never registered. Any thread |
| `action_enable(name, on)` | off keeps the claim and makes it inert: the key reaches the engine and nothing fires |
| `action_capture(on)` | only the caller's actions fire; `GBH_ERR_CONFLICT` while another mod holds it |

An action is a name the mod registers and the player binds. The chord comes from the
setting `bind.<name>`, so `bind.menu = F1` in the mod's `modinfo.ini` is the default and
`gb.mymod.bind.menu` in `gbhook.ini` overrides it. A chord is one key plus any of `CTRL`,
`SHIFT` and `ALT`, joined by `+`, any case, in any order. `LCTRL`, `RSHIFT` and the other
sided spellings mean the plain modifier, since the window cannot tell the sides apart. Key
names are the ones `input_dik_from_name` knows. An empty value or `NONE` leaves the action
unbound; anything else that does not parse is logged and left unbound.

The first claim on a chord wins. A second, from any mod, is logged naming both actions and
stays unbound. `F` and `CTRL+F` are different chords, and when both could match the one with
more modifiers fires. A bound key never reaches the engine, its key-up included, and holding
it down fires once. `fn` runs on the main thread from the pump, once per press, under the
same guard as an event: a fault logs the mod and the action and turns that action off for
the process. When the window loses focus every key is treated as released.

A capture is for a menu that wants the keyboard. While one mod holds it, only its own
actions fire, every other mod's bound key is swallowed, and a key nobody bound still reaches
the engine. `binds` lists every action, its chord and its help.

### World, any thread

| entry | notes |
|---|---|
| `paused()` | `GBH_PAUSED_FREEZE`, `GBH_PAUSED_SCREEN` and `GBH_PAUSED_FOCUS` bits; `0` while the world ticks |
| `pause(on)` | the caller holds or releases the freeze. Idempotent both ways; up to eight mods may hold it at once |
| `world_to_screen(pos, out)` | backbuffer pixels. `1` on screen, `0` behind the camera or outside; `GBH_ERR_STATE` when the camera cannot be read |

The freeze is the byte the level loop itself tests: the world update and the game clock stop,
while rendering, the HUD and audio carry on and nothing is blacked out. The engine clears the
byte on a pad hot-plug or the Steam overlay, so gbhook sets it again every frame while anyone
holds it. On the last release it clears the byte only if gbhook set it. One gap: if the
engine's own disconnect freeze starts while a mod holds the pause, the last release clears it
too. Esc still opens the game's pause menu during a freeze. The front-end gate is only read,
never forced: forcing it blacks out the frame.

The `SCREEN` bit is the engine's own check on the main thread. From any other thread it is
read from memory, which is close but not the same check.

`world_to_screen` is the engine's own projection, so it follows camera shake and cinematic
cameras. A point in front of the camera but off screen still fills `out`, so an arrow can be
clamped to the edge. The last step, from the projected point to a pixel row, was never
traced in the engine: if a marker comes out mirrored vertically, that one line is flipped.

## 5. The C++ wrapper

`gbhook.hpp` is header-only and compiles into the mod, so it adds nothing to the contract.
`gbh::bind(api)` stashes the table; the rest are one-line wrappers under the same names,
plus three guards that release on scope exit unless told to keep: `gbh::Sub` for a
subscription, `gbh::Patch` for a byte patch, `gbh::VtableOverride` for a cloned table.
`gbh::at<T>(rva)` turns a `ghost.exe`-relative offset into a pointer, and `gbh::hook_at`
hooks one. `gbh::has_services()` says whether the framework carries the block appended after
`file_read`; every wrapper for that block answers as unsupported without it.
`gbh::has_actions()` does the same for the actions and world entries appended after
`on_char` in 0.2.5.
 