# The ABI

`sdk/include/gbhook/gbhook.h` is the ABI contract. It is pure C: a manifest a mod exports as
data, one entry point, and a table of function pointers handed to that entry point.
`gbhook.hpp` wraps the table for C++ and `seh.h` holds the fault guards. This document
describes every public name in them.

```
GbhPluginManifest   exported data, read out of the file before the DLL runs
GbhPluginInit       exported code, called once at the mod's stage with the table
GbhApi              80 entries after the two header fields, appended to and never reordered
```

Sections 1 to 3 are the rules that apply to every entry. Section 4 is the table in header
order. Section 5 is the C++ wrapper and section 6 the SEH helpers. Section 7 lists every
constant and limit in one place.

---

## 1. Versioning

`GBHOOK_ABI_VERSION` is 1. It changes only for a breaking change, which is one an existing
binary cannot survive: removing, reordering or retyping a table member, changing the layout
of `GbhManifest` or of a struct that has no `struct_size`, or changing a status code or
constant a binary already compiled in. A mod built against another version is refused at
discovery, before its DLL is loaded. The check reads `abi` from `modinfo.ini` and
`abi_version` from the manifest. Both must equal the running framework's value.

Within a version the table only grows. Every entry keeps its slot, new entries are appended,
and `GbhApi.struct_size` says how far the running framework's table reaches. A mod built
against a newer header than the framework carries can hold a pointer to a slot the framework
never filled, so it tests before it calls:

```c
if (gbh_api_has(api, file_read)) { /* the table reaches file_read */ }
```

`gbh_api_has(api, member)` is true when `api` is non-null and `struct_size` covers the whole
member. Calling an entry the table does not reach reads past the end of the framework's
table. The C++ wrappers in section 5 perform this test for every entry added after
`file_read`.

The appended entries come in blocks, and the header marks the start of two of them:

| block | first entry | wrapper test |
|---|---|---|
| services, memory, levels, actors, attributes, keys | `service_publish` | `gbh::has_services()` (tests `on_char`) |
| actions and world | `action_register` | `gbh::has_actions()` (tests `world_to_screen`) |
| mod pages and written settings | `mods_page_add` | `gbh::has_mod_pages()` (tests `mods_page_add`) |

`framework_version()` is `(major << 16) | (minor << 8) | patch`. It is for logging.
`struct_size` is the compatibility signal.

Structs that cross the table carry their own `struct_size`: `GbhNativeMenuDesc`,
`GbhActorInfo` and `GbhAttrInfo`. The caller states how much of the struct it has, and the
framework fills no more than that. `GbhManifest.struct_size` is the build-time size of the
manifest. A manifest larger than the framework's own is refused as newer. A smaller one is
accepted.

`GBHOOK_TARGET_MD5` is the md5 of the one `ghost.exe` build every offset was verified
against. `GBHOOK_TARGET_NAME` is its display name. A manifest carries the md5, and a mod
naming another build is refused.

## 2. Entry points and the manifest

A mod DLL exports two symbols, named by `GBH_EXPORT_MANIFEST` (`"GbhPluginManifest"`, data)
and `GBH_EXPORT_INIT` (`"GbhPluginInit"`, code). The manifest is read from the file by
walking the export directory, and no code runs to read it. The names are looked up as plain
C names, so `GBHOOK_LINKAGE` expands to `extern "C"` under C++, and `GBHOOK_EXPORT` expands
to `__declspec(dllexport)` on Windows.

### The manifest

```c
GBHOOK_PLUGIN("mymod");
```

```c
GBHOOK_PLUGIN_EXCLUSIVE("mymod") { "ghost+0x248190", "ghost+0x2487E0", "" } GBHOOK_PLUGIN_END;
```

Use exactly one of the two, once, at file scope in exactly one translation unit of the DLL.
`GBHOOK_PLUGIN(id)` declares a manifest with no exclusive claims.
`GBHOOK_PLUGIN_EXCLUSIVE(id)` opens the initialiser and `GBHOOK_PLUGIN_END` closes it, with
the claim list between them as one braced list of strings.

```c
typedef struct GbhManifest {
    char     magic[9];                                 /* GBH_MANIFEST_MAGIC, "GBHOOKMF" */
    uint32_t struct_size;                              /* sizeof(GbhManifest) at build time */
    uint32_t abi_version;                              /* GBHOOK_ABI_VERSION */
    char     id[64];
    char     target_md5[40];                           /* GBHOOK_TARGET_MD5 */
    char     exclusive_hooks[GBH_MAX_EXCLUSIVE][64];   /* "ghost+0xHEX", ended by an empty entry */
} GbhManifest;
```

What discovery checks, in the order it refuses:

- The magic is `GBH_MANIFEST_MAGIC`.
- `abi_version` equals the framework's.
- `struct_size` does not exceed the framework's `sizeof(GbhManifest)`.
- `id` is not empty.
- `target_md5` equals `GBHOOK_TARGET_MD5`.
- `id` equals the `id` in `modinfo.ini`, and `abi_version` equals its `abi`. A mismatch means
  one of the two was edited after the build, and the mod is refused.

An id may use `a-z`, `0-9` and `_` and may be at most 63 characters. The id is the namespace
of the mod's commands, settings, services and log lines. `docs/MOD_FORMAT.md` covers the
rest of `modinfo.ini`, including `plugin`, `stage`, `priority` and `requires`.

### Exclusive claims

`exclusive_hooks` holds up to `GBH_MAX_EXCLUSIVE` (16) claims, each at most 63 characters. A
claim is the text `ghost+0xHEX` for an address in `ghost.exe`. The list ends at the first
empty entry, and unset entries are empty. A list of 16 needs no terminator.

Claims are compared as exact text across the accepted mods, so `ghost+0x46A110` and
`ghost+0x46a110` are different claims. When two accepted mods claim the same text, discovery
logs a `CONFLICT` line naming both and keeps both. The claim does not itself block or grant
anything. Arbitration happens when the mod calls `hook_create`, as section 4 describes. The
first mod to install a detour at an address holds it, and the other's `hook_create` returns
`NULL`. Load order is stage, then `priority` (lower first), then id.

### `GbhPluginInit`

```c
typedef int (*GbhPluginInitFn)(const GbhApi* api);
GBHOOK_EXPORT int GbhPluginInit(const GbhApi* api);   /* extern "C" in C++ */
```

The framework loads the DLL with `LoadLibraryEx` using an altered search path, so a helper
DLL beside the mod resolves first. It then calls `GbhPluginInit` once, at the stage the
mod names in `modinfo.ini`, with the table. The table lives for the process, so the pointer
may be kept.

- The call runs on gbhook's own startup thread. That thread is not the game thread, so
  `is_game_thread()` answers 0 and the file, level-list and `attr_set` calls refuse it.
  Entries marked "any thread" work there.
- Return `GBH_OK` to come up. Any other value leaves the mod marked failed, with the value
  in its status note. A fault inside the call also marks it failed. The hooks, patches and
  subscriptions the mod made before it failed stay in place, because nothing is unloaded.
- After a successful init the framework verifies every hook, patch and vtable copy and logs
  any drift (see Patches).
- `mods_page_add` is valid only inside this call.

A mod that does not know the table's version checks it first:

```c
int GbhPluginInit(const GbhApi* api)
{
    if (!api || api->abi_version != GBHOOK_ABI_VERSION) return GBH_ERR;
    return GBH_OK;
}
```

### Stages

`GbhStage` orders when mods initialise. Each mod names its stage in `modinfo.ini` as
`preboot`, `early`, `boot` (the default) or `ready`. `GBH_STAGE_COUNT` is 4. Within a stage,
mods run by `priority` (default 100, lower first), then by id. `requires` names a dependency
but orders nothing, so a dependency at a later stage or higher priority initialises after.

| stage | what exists when its mods run |
|---|---|
| `GBH_STAGE_PREBOOT` (0) | settings, discovery, logging and the hook broker. None of the framework's own detours or built-in commands yet |
| `GBH_STAGE_EARLY` (1) | the engine pump, frame, VM registration, level and window detours, the service directory, the framework's commands and the content build thread |
| `GBH_STAGE_BOOT` (2) | the same, with EARLY mods already initialised |
| `GBH_STAGE_READY` (3) | the native menu broker and gbhook's Mods page row |

Subscriptions, command and action registrations and service publications made at PREBOOT
are kept. They start to fire once the framework installs the detours behind them. The
framework installs its own detours after PREBOOT, so a PREBOOT mod must not hook the sites
that the buses use. The native menu detours are installed after BOOT.

## 3. Rules

**Threads.** Each group below names its thread. "Game thread" is the thread that ticks the
level. `game_thread_id()` is 0 and `is_game_thread()` is false until the first frame. "Engine
main thread" is the game thread or the pump's thread, and the file and level-list calls
accept either. "Any thread"
means the entry takes its own lock. Callbacks run where their group says. Command handlers
run on the game thread.

**Memory.** Every mod is built with the static CRT, so each module owns its own heap, and
nothing may be allocated on one side of the table and freed on the other. The shipped
`sdk/GbHookPlugin.props` sets this. Every buffer a call fills is the caller's, and nothing
transfers ownership. `alloc`, `realloc` and `free` are the framework's heap, for the rare
thing handed across.

**Strings.** A `const char*` a call returns is the framework's. It stays valid until the
calling thread's next call into the table. Copy it if it must live longer. A pointer a
callback receives is valid for the callback only. The exception is a service table, which is
the publisher's pointer and lives as long as the publisher keeps it (see Services).

**The caller.** No entry takes a mod handle. The framework derives the calling mod from the
return address, by finding which loaded mod DLL contains it. That attributes a log line, a
hook, a claim, a setting, a command or a subscription to its owner. A call whose return
address is in no mod DLL, such as one made from a helper DLL the mod loaded, is attributed
to the framework. Such a call logs with no id and registers commands and services without the
id prefix. `setting_set` and `mods_page_add` answer `GBH_ERR_STATE` to it. Keep every call in
the mod's own DLL. The inline `gbh::` wrappers do.

**Faults.** Every callback runs under a guard, and the first fault is handled at the
callback:

| callback | on a fault |
|---|---|
| event subscriber (frame, pump, level, actor, key, char) | logged with the mod and event, and that one subscription is dropped for the process |
| action | logged, and that action is turned off for the process |
| command handler | logged, and the call returns `GBH_ERR`. The command stays registered |
| native row callback | the claim on that row is dropped |
| native `build` | logged, and the page keeps the rows added so far |
| native `activate` | logged, and the page closes |
| `GbhPluginInit` | the mod is marked failed |

The game continues in every case. A detour installed with `hook_create` is the mod's own
code and is not guarded by the framework. A mod guards engine calls inside a detour with the
helpers in section 6. The framework never unloads a DLL: a detour executing on another
thread while its module is torn out is not recoverable.

**Status.** Every `int` entry that reports success answers with `GbhStatus`, and `GBH_OK`
is zero. A negative return from a call that otherwise returns a count or a size is a status.

| status | value | meaning |
|---|---|---|
| `GBH_OK` | 0 | |
| `GBH_ERR` | -1 | unspecified, or a framework limit reached |
| `GBH_ERR_ARG` | -2 | a null or out-of-range argument |
| `GBH_ERR_STATE` | -3 | not valid now: no level, too early, not inside the callback the call needs |
| `GBH_ERR_UNSUPPORTED` | -4 | the framework is older than this call, or the key is read-only |
| `GBH_ERR_CONFLICT` | -5 | another mod owns it, or it is already taken |
| `GBH_ERR_NOT_FOUND` | -6 | |
| `GBH_ERR_WRONG_THREAD` | -7 | must be called on the engine's thread |
| `GBH_ERR_TRUNCATED` | -8 | the output buffer was too small, and what fits is valid |

## 4. The table

Entries are in header order. A signature is shown only where the prose needs it, and
`gbhook.h` has all of them.

### Environment, any thread

| entry | answers |
|---|---|
| `void* game_base(void)` | the `ghost.exe` module base. Every offset in `sdk/include/gb` is relative to it |
| `const char* game_dir(void)` | the folder holding `ghost.exe`, no trailing separator |
| `const char* mod_dir(void)` | the calling mod's `gbhook/` folder, `<root>\<folder>\gbhook`. For a caller that is not a mod, the game folder |
| `uint32_t framework_version(void)` | packed major, minor, patch, for logging |

### Memory, any thread

The framework heap, for memory that crosses the table. It is a private Win32 heap, separate
from the CRT heap of any module.

- `void* alloc(size_t n)` returns at least one byte even for `n` of 0. It returns `NULL`
  when the allocation fails.
- `void* realloc(void* p, size_t n)` behaves as `alloc` when `p` is `NULL`. It frees `p` and
  returns `NULL` when `n` is 0.
- `void free(void* p)` ignores `NULL`.

### Log, any thread

`void log(const char* tag, const char* line)` and `void logf(const char* tag, const char* fmt, ...)`.

Lines go to `<gamedir>\gbhook.log` as `[HH:MM:SS] TAG [id] text`, where the tag names the
subsystem and the framework adds the id. The file is flushed per line. It is truncated at
launch, and the previous run is kept as `gbhook.log.prev`. A `logf` result is cut at 2047
characters. A `NULL` tag or line logs as an empty one.

### Settings, any thread

```c
const char* setting(const char* key, const char* dflt);
int         setting_int(const char* key, int dflt);
float       setting_float(const char* key, float dflt);
int         setting_bool(const char* key, int dflt);
```

A key resolves in this order, and the first hit wins:

1. `<id>.<key>` in `gbhook.ini`.
2. The same key in the mod's own `modinfo.ini`. Every key in that file doubles as a default,
   gbhook's own keys included.
3. `dflt`.

Keys and ids are compared in lower case. A `NULL` or empty key answers `dflt`.

- `setting` returns the text. The returned pointer is valid until the calling thread's next
  `setting*` call.
- `setting_int` reads a C integer in any base `strtol` accepts with base 0, so `0x10` works.
  Text with no number answers `dflt`.
- `setting_float` reads with `strtof`. Text with no number answers `dflt`.
- `setting_bool` answers 1 or 0. An empty value answers `dflt`. `off` is false, and so is any
  value starting with `0`, `f` or `n` in any case. Anything else is true.

See `setting_set` for writes.

### Hooks, any thread

```c
GbhHook hook_create(void* target, void* detour, void** original, uint32_t flags);
int     hook_enable(GbhHook h);
int     hook_disable(GbhHook h);
int     hook_enable_batch(const GbhHook* hooks, int count);
```

`GbhHook` is an opaque handle. Zero is never valid. gbhook owns the one MinHook in the
process. `target` is an absolute address (`game_base()` plus an offset). `original`
receives the trampoline and may be `NULL`.

`hook_create` returns the handle, or `NULL` when:

- `target` or `detour` is null.
- The address already has a detour, held by any mod or by the framework. This includes the
  same mod hooking it twice.
- A live byte patch overlaps the first 20 bytes at `target`.
- MinHook cannot build the hook.
- The hook is not deferred and MinHook cannot enable it.

Each refusal is logged. Unknown bits in `flags` are ignored.

**Flags.** `GbhHookFlags` values combine with `|`.

| flag | value | effect |
|---|---|---|
| `GBH_HOOK_NONE` | 0 | create and enable at once |
| `GBH_HOOK_DEFERRED` | 1 | create the hook and its trampoline, but leave it disabled |
| `GBH_HOOK_EXCLUSIVE` | 2 | mark the hook exclusive |

Without `GBH_HOOK_DEFERRED`, `hook_create` enables the hook before it returns. A later
`hook_enable` on that handle returns `GBH_ERR`, because it is already enabled. `hook_enable`
returns `GBH_OK` only for a valid handle that is currently disabled. `hook_disable` is the
mirror and returns `GBH_ERR` for a hook that is already disabled. A disabled hook keeps its
address, so no other mod can take the address while it exists. There is no call that removes
a hook.

`GBH_HOOK_EXCLUSIVE` does not change what `hook_create` allows, since an address takes one
detour in any case. It marks the hook so that a later attempt from another mod is logged as a
`CONFLICT` that tells the second mod to declare the address in its manifest. Use it together
with a manifest claim (section 2) to state the intent.

`hook_enable_batch` arms a set of hooks together, so no moment exists with one detour live
and its sibling not. It queues the enable of every valid, currently disabled handle in the
list and applies the whole queue at once. Handles that are null, invalid or already enabled
are skipped. It returns `GBH_ERR` when `hooks` is null, `count` is not positive, or MinHook
cannot apply the queue, and `GBH_OK` otherwise.

```cpp
typedef void (__fastcall* tFn)(void*);
static tFn oA, oB;
static void __fastcall HkA(void* self) { oA(self); }
static void __fastcall HkB(void* self) { oB(self); }

// Placeholder offsets: use real ghost+ offsets.
GbhHook h[2] = {
    gbh::hook_at(0x1000, (void*)HkA, (void**)&oA, GBH_HOOK_DEFERRED),
    gbh::hook_at(0x2000, (void*)HkB, (void**)&oB, GBH_HOOK_DEFERRED),
};
if (h[0] && h[1]) gbh::enable_batch(h, 2);
```

**Contended sites.** The framework hooks the per-level tick, the pump, VM global
registration, level prepare, the level-flow action poll and begin-level sync, the window
procedure and, after BOOT, the native menu rows. It fans these out as events (below). A mod
subscribes instead of hooking. From EARLY on, a mod that asks for one of these addresses
gets `NULL`, because the framework already holds it.

### Patches, any thread

```c
GbhPatch patch_write(void* at, const void* bytes, size_t n);
int      patch_revert(GbhPatch p);
```

`patch_write` records the site, writes `n` bytes, reads them back and keeps the record. `n`
is at most `GBH_MAX_PATCH_BYTES` (64). It returns the handle, or `NULL` when:

- `at` or `bytes` is null, or `n` is 0 or over the cap.
- The write overlaps the first 20 bytes at any hooked address, enabled or not.
- The write overlaps another live patch.
- The site is not readable or writable, or the bytes read back differ.

The write changes the page protection for the copy and restores it, so it works on read-only
data such as a vtable slot and on code. It flushes the instruction cache.

`patch_revert` restores the bytes that were there before the patch. It returns `GBH_ERR`
when the handle is invalid, the patch is already reverted, the site is no longer readable,
or the bytes at the site are not the ones the patch wrote. In the last case another writer
changed them, and reverting would clobber that write.

**Arbitration.** A detour and a byte patch cannot share bytes. `hook_create` is refused when
a live patch overlaps the first 20 bytes at its target, and `patch_write` is refused when
it overlaps the first 20 bytes at any hooked target. The 20 bytes cover the longest prologue
MinHook relocates. Two live patches cannot overlap either, and the second is refused with
the first owner named in the log.

**Verification.** The framework keeps a baseline of the first 20 bytes of every enabled
hook, the bytes of every live patch, every vtable slot it wrote and every swapped vptr. It
re-reads them all after each mod's `GbhPluginInit`, at the end of boot, and when the `hooks`
command runs. A difference is logged as `TAMPER` with the owner and the address. The sweep
reports and does not repair. It is not periodic.

### Vtables, any thread

```c
void* vtable_clone(void* original, int slots);
int   vtable_slot(void* clone, int slot, void* fn, void** original_fn);
int   vtable_apply(void* object, void* clone);
void* vtable_original(void* clone, int slot);
```

A vtable override replaces some virtual methods of a class without editing the shipped
table. `vtable_clone` copies a shipped table into framework memory, a mod claims slots in
the copy, and `vtable_apply` points an object at the copy.

- `vtable_clone(original, slots)` copies `slots` entries and the RTTI locator that sits
  before slot 0, so the copy still identifies as its class. It returns a pointer to slot 0
  of the copy, or `NULL` when `original` is null, `slots` is not in 1 to 4096, the table is
  unreadable, or the table was already cloned with fewer slots than asked. Each shipped table
  has one copy, shared by every mod. A second call for the same table with the same or a
  smaller `slots` returns the same copy. The copy lives for the process.
- `vtable_slot(clone, slot, fn, original_fn)` writes `fn` into the copy and stores the
  shipped entry in `*original_fn` (which may be `NULL`). It returns `GBH_ERR_ARG` for a null
  `clone` or `fn` or a negative `slot`. It returns `GBH_ERR_CONFLICT` when `clone` is not a
  copy the framework made, `slot` is outside the copy, or another mod already holds the slot.
  The same mod may set its own slot again, and `*original_fn` still reports the shipped entry.
- `vtable_apply(object, clone)` points the object's first pointer at the copy. The object's
  current vptr must be the shipped table or the copy, otherwise it returns `GBH_ERR`. It
  returns `GBH_ERR_ARG` for a null argument, and `GBH_ERR` when the clone is unknown or the
  object cannot be read or written. Applying twice is `GBH_OK`.
- `vtable_original(clone, slot)` returns the shipped entry for a slot, or `NULL` for an
  unknown clone, a slot out of range or an unreadable table.

Apply only to objects that live for the process. The sweep re-reads every applied vptr, so
an object that is freed and its memory reused reads as tamper.

### Introspection, any thread

- `int mod_count(void)` is the number of mods discovery saw. Accepted mods come first, in
  code order, then the refused and disabled ones.
- `const char* mod_id_at(int i)` is the id at index `i`, or `NULL` out of range.
- `int mod_is_loaded(const char* id)` is 1 when the mod has initialised, or is accepted with
  no code. It is 0 for a mod that is refused, disabled, failed, or not yet reached by its
  stage. The id match ignores case.

### Events, callbacks on the engine's threads

```c
GbhSub on_frame(GbhFrameFn fn, void* user);
GbhSub on_pump(GbhFrameFn fn, void* user);
GbhSub on_level(GbhLevelFn fn, void* user);
GbhSub on_actor_registered(GbhActorFn fn, void* user);
void   unsubscribe(GbhSub s);
```

Each `on_` call returns a `GbhSub` handle, or `NULL` when `fn` is null or the bus is full.
Each bus holds at most 64 subscribers. The framework's detours fire the buses.

| bus | fires | callback |
|---|---|---|
| frame | once per frame while a level is live, on the game thread, just before the engine's tick body runs | `void (*GbhFrameFn)(void* user)` |
| pump | once per frame on the pump's thread after the engine's pump function, front end included | `GbhFrameFn` |
| level | at the start and end of every level prepare | `void (*GbhLevelFn)(int phase, const char* level, int ok, void* user)` |
| actor | once per Dante VM global registered during a level load | `void (*GbhActorFn)(const char* cls, const char* name, void* ptr, void* user)` |

**Frame and pump.** These two are on the engine's critical path. The framework times every
subscriber on both. For the frame bus it logs a `BUDGET` line naming any subscriber that
averages over 250 microseconds per call, once per 600 frames. The pump bus is timed but not
reported. Keep both short.

**Level.** `GbhLevelPhase` is `GBH_LEVEL_PREPARE_BEGIN` (0) or `GBH_LEVEL_PREPARE_END` (1). The
framework clears the VM registry before BEGIN, so the registry and the actor list are empty
at BEGIN. At BEGIN, `ok` is 1 and carries no information, and `level` is the stem CGame
holds before the load starts. At END, `level` is the stem after the load and `ok` says
whether the engine accepted the level. A failed prepare logs the engine's error lines.

**Actor.** `cls` and `name` point into the framework's buffer and are valid only inside the
callback. The event fires for every registration. `registry_snapshot` keeps only entries
whose class name starts with `C`.

**Ordering and re-entrancy.** Subscribers run in slot order, which is registration order
except that a freed slot is reused first. The table is read under a lock and dispatched
outside it, so a callback may subscribe, unsubscribe or log. A subscriber added during a
dispatch first runs on the next one.

**Unsubscribing.** `unsubscribe(s)` ignores a null handle and one already unsubscribed or
faulted. A handle encodes its bus and slot only, and a freed slot is reused. Unsubscribe once
and drop the handle.

### The game model, guarded reads, any thread

| entry | answers |
|---|---|
| `void* game_singleton(void)` | `CGame*`, null very early |
| `void* local_player(void)` | the local player's actor, null when none exists |
| `uint32_t game_thread_id(void)` | zero until the first frame |
| `int is_game_thread(void)` | 1 on the game thread, 0 on any other, and 0 before the first frame |
| `const char* level_name(void)` | the stem CGame holds, `""` when CGame is unreadable. Valid until the thread's next call, and per thread |
| `int registry_snapshot(GbhRegistryEntry* buf, int cap)` | see below |
| `int registry_find(const char* name, GbhRegistryEntry* out)` | `GBH_OK`, `GBH_ERR_ARG` for a null argument, `GBH_ERR_NOT_FOUND` |

```c
typedef struct GbhRegistryEntry {
    void*    ptr;
    char     cls[48];
    char     name[64];
    uint32_t generation;   /* bumped at every level prepare */
} GbhRegistryEntry;
```

The registry lists this level's Dante VM globals, which the script exported. It covers
objects whose class name starts with `C`. `registry_snapshot` copies up to `cap` entries
into `buf` and returns how many it wrote. A null `buf` with `cap` 0 returns the total
instead. Any other null or non-positive combination returns `GBH_ERR_ARG`. The total can
change between the two calls, so leave headroom. `registry_find` matches the name exactly
first, then as a substring, ignoring case. Strings are cut to the field sizes.

A `ptr` is live game memory. Use it on the game thread only. It is stale after the next
level prepare, and `generation` tells which prepare it belongs to.

### Commands

```c
typedef int (*GbhCommandFn)(int argc, const char* const* argv, const char** err, void* user);

int command_register(const char* name, GbhCommandFn fn, void* user, const char* help);
int command_run(const char* line);
int command_queue(const char* line);
```

A command is a named handler the player or another mod can call. It is reached from
`gbhook.cmd` in the game folder, from other mods and from the framework.

- `command_register` registers `<id>.<name>`, lower-cased. It returns `GBH_ERR_ARG` for a
  null `fn`, an empty name, or a name containing a blank, tab or double quote. It returns
  `GBH_ERR_CONFLICT` when that qualified name already exists. `help` is shown by `help` and
  may be `NULL`. The handler always runs on the game thread.
- A handler receives its arguments only. `argv` does not include the command name, and the
  line is split at blanks with double quotes grouping. It returns `GBH_OK`, or points `*err`
  at a static string and returns a `GbhStatus`. The framework logs `OK` or `ERR` with the
  line.
- `command_run(line)` runs the line where it is safe. Called on the game thread it runs in
  place and returns the handler's result. Called from any other thread, a command that needs
  the game thread (every mod command does) is queued, and the call returns `GBH_OK`. If
  nothing drains the queue yet, it runs in place on the caller's thread. An unknown command
  returns `GBH_ERR_NOT_FOUND`, and an empty line `GBH_ERR_ARG`.
- `command_queue(line)` queues the line to run on the game thread at the next frame, the
  front end included. Called from the game thread it runs in place. If no tick or pump is
  hooked yet it returns `GBH_ERR_STATE`.

The queue holds 64 lines of up to 511 characters each. A full queue drops the line and
returns `GBH_ERR_STATE`. The game thread drains eight lines per frame. The framework's own
commands include `help`, `mods`, `hooks`, `ping`, `hud`, `key`, `sleep`, `level`,
`checkpoint`, `files`, `filesize`, `services`, `actors`, `actor`, `attr` and `binds`.

### Input, any thread

- `void input_set_key(int dik, int down)` writes the engine's own scan-code table, the one
  the game reads. `SendInput` never reaches it. The key stays down until a call with `down`
  of 0 releases it, like a real key. `dik` is a DirectInput scan code. An out-of-range code
  is ignored.
- `int input_dik_from_name(const char* name)` maps a name to a DIK code, ignoring case, and
  returns -1 when unknown or empty. Names include letters, digits, `F1` to `F12`, `ENTER`,
  `SPACE`, `TAB`, `ESC`, the arrows, `HOME`, `END`, `PGUP`, `PGDN`, `INS`, `DEL`, numpad
  keys, punctuation names such as `COMMA`, and the sided modifiers `LSHIFT`, `RSHIFT`,
  `LCTRL`, `RCTRL`, `LALT` and `RALT`. This is a different name set from the action chord
  keys, which map to virtual-key codes (see Actions).

### HUD, any thread, in a level

`void hud_message(const char* text, float seconds)` shows the engine's message line at the
top of the screen. It returns nothing.

- On the game thread it draws at once. From any other thread it is queued through the `hud`
  command, with `"` in the text changed to `'` and the whole line cut at 511 characters.
- `seconds` of 0 or below shows for 3 seconds.
- It needs a live level. With no local player the text is dropped and the log says so.

### Level flow, any thread

`int level_chain(const char* level, const char* checkpoint)` loads a level, from the front
end or in play. It runs the `level` command, so the thread rules of `command_run` apply: on
the game thread it returns the handler's result, and elsewhere it queues and returns
`GBH_OK`. It returns `GBH_ERR_ARG` for a null or empty `level`, and `GBH_ERR` when the load
could not be armed.

- At the front end `level` is a stem or a `.lvl` name. In play it must be the stem alone.
  The front end is the state with no local player and no recent tick.
- `checkpoint` may be `NULL`. It is a script name such as `checkpoint_OldStacks`, or the
  display name the level registers it under, matched ignoring case. It is armed at the
  level's begin, after its script has registered its checkpoints. A name the level does not
  register is logged with the ones it does, and the level starts from the top.
- The call is formatted as one command line of 255 characters. Neither name may contain a
  blank.

### The native menu, game thread

The game's own main menu is a column of seven rows: Career (0), Online (1), Mods (2), a free
row (3), Options (4), Extras (5) and Exit (6). A mod can claim the Online and free rows,
and from a claimed row's callback it can push a page of its own. Pages nest like the game's
own screens, and ESC closes one page.

```c
typedef void (*GbhRowFn)(int row, void* user);

#define GBH_ROW_ONLINE        1
#define GBH_ROW_FREE          3
#define GBH_NATIVE_LABEL_CAP  40
#define GBH_NATIVE_MAX_ROWS   40
#define GBH_NATIVE_STAY       0    /* rebuild and redraw this page */
#define GBH_NATIVE_CLOSE      1    /* pop this page */
#define GBH_NATIVE_CLOSE_ALL  2    /* pop every page, back to the main menu */
#define GBH_NATIVE_INERT      -1   /* a row action for a header or text line */

typedef struct GbhNativeMenuDesc {
    uint32_t    struct_size;
    void      (*build)(void* user);
    int       (*activate)(int action, void* user);
    void*       user;
    const char* title;
} GbhNativeMenuDesc;
```

| entry | notes |
|---|---|
| `int native_row_claim(int row, GbhRowFn fn, void* user)` | `GBH_OK` on success. `GBH_ERR_ARG` for a row outside 0 to 6 or a null `fn`. `GBH_ERR_CONFLICT` for any row other than `GBH_ROW_ONLINE` and `GBH_ROW_FREE`, and for a row another mod already holds. `GBH_ERR_STATE` if the broker could not install. A claim made before the broker is up goes live when it installs |
| `int native_row_label(int row, const char* label)` | sets your row's text verbatim, at most 63 characters, and re-applies it after every refill of the menu. `GBH_ERR_ARG` for a bad row or null label, `GBH_ERR_STATE` for a row nobody claimed, `GBH_ERR_CONFLICT` for another mod's row |
| `int native_submenu_open(const GbhNativeMenuDesc* desc)` | pushes a page. Valid only inside a row callback you own, or inside the `activate` of an open page, where it nests under that page. See below |
| `int native_submenu_add_row(const char* label, int action)` | adds a row to the page being built. Valid only inside `build`. `GBH_ERR_STATE` outside it or past `GBH_NATIVE_MAX_ROWS` |
| `void native_submenu_refresh(void)` | rebuilds and re-labels the page on top. Cheap when nothing changed. Does nothing with no page open |

Rows 1 and 3 stay hidden until a mod claims them. A claimed row never falls through to the game's own action. A row callback runs on the game
thread when the player activates the row, with the row number and your `user`.

**Pages.** `native_submenu_open` copies `desc` and calls `build` at once, then again after
every activation that answers `GBH_NATIVE_STAY` and on every `native_submenu_refresh`.
`build` publishes rows with `native_submenu_add_row`. The label is cut to 39 characters and
`NULL` is an empty label. The `action` is the integer you will get back in `activate`.
`GBH_NATIVE_INERT` marks a row that never activates. The rows reach the screen a few frames
after the page opens.

- `struct_size` may be smaller than the current struct. The framework copies only what it
  states, with `title` optional. `build` and `activate` are required.
- `title` is shown verbatim, up to 95 characters. A leading `@` names a localisation key,
  and a key the string table lacks leaves the donor's title in place. `NULL` also keeps it.
- `activate(action, user)` answers `GBH_NATIVE_STAY`, `GBH_NATIVE_CLOSE` or
  `GBH_NATIVE_CLOSE_ALL`. Any other value acts as stay. If `activate` itself opens a child
  page, the parent page stays as it is, whatever the return value. Row actions come from the
  rows the last `build` published.
- At most 16 pages nest. `native_submenu_open` returns `GBH_ERR_ARG` for a null `desc`,
  `build` or `activate`. It returns `GBH_ERR_STATE` when called outside a row callback or an
  `activate`, while the pages are closing, past 16 deep, or when the page cannot be created.

```c
static const GbhApi* g_api;
static void Build(void* user)
{
    g_api->native_submenu_add_row("Heal", 1);
    g_api->native_submenu_add_row("Back", 2);
}
static int Activate(int action, void* user)
{
    if (action == 1) g_api->hud_message("healed", 2.0f);
    return action == 2 ? GBH_NATIVE_CLOSE : GBH_NATIVE_STAY;
}
static void OnRow(int row, void* user)
{
    GbhNativeMenuDesc d = { sizeof d, Build, Activate, 0, "My page" };
    g_api->native_submenu_open(&d);
}
/* in GbhPluginInit: */
/*   g_api->native_row_claim(GBH_ROW_FREE, OnRow, 0);  g_api->native_row_label(GBH_ROW_FREE, "My mod"); */
```

### Files, engine main thread only

```c
int file_list(const char* dir, const char* pattern, void (*cb)(const char* name, void* user), void* user);
int file_read(const char* path, void* buf, int cap);
```

The engine's file tables take no lock, so both calls return `GBH_ERR_WRONG_THREAD` from any
other thread, `GbhPluginInit` included. They work from a command handler, `on_frame`,
`on_pump` and an action. Argument checks come first, so a bad argument answers `GBH_ERR_ARG`
on any thread.

**`file_list`** walks every mounted archive through the engine's own enumerator. `dir` may
be `NULL`. `pattern` is required (`GBH_ERR_ARG` if null or empty, and the same for a null
`cb`), and `dir` plus `pattern` together must stay under 200 characters. The engine splits
`pattern` on `;` and `,`. The names are bare, de-duplicated ignoring case and sorted
ignoring case. `cb` runs once per name before the call returns, and the string is valid for
that call only. The return value is the count. A negative return is a status. `GBH_ERR` means the engine's
enumerator faulted or returned a count over 4096.

**`file_read`** reads `path` through the engine's own resolution, which is memory, then the
mounted archives, then loose files, and returns the decompressed bytes. The return value is
the number of bytes. A null `buf` with `cap` 0 answers the size without copying, and the
file is still read in full. A `cap` smaller than the file returns `GBH_ERR_TRUNCATED` and
copies nothing. A missing file is `GBH_ERR_NOT_FOUND`, and an engine fault is `GBH_ERR`.
`path` must be non-empty, shorter than 240 characters and free of `..`, otherwise
`GBH_ERR_ARG`. A negative `cap`, or a positive `cap` with a null `buf`, is also
`GBH_ERR_ARG`. Reading stops past 64 MiB.

### Services, any thread

Services are published tables of function pointers that one mod owns and others call.

```c
int         service_publish(const char* name, const void* table, uint32_t size);
const void* service_find(const char* name, uint32_t* size);
int         service_count(void);
const char* service_name_at(int i);
const char* service_owner(const char* name);
```

A service table is a C struct of function pointers. Its first field is a `uint32_t
struct_size`, so a consumer gates on what the table carries, the same way `gbh_api_has`
does for this table.

- `service_publish(name, table, size)` lists the table under `<id>.<name>`, lower-cased, with
  the id taken from the calling mod. It returns `GBH_ERR_ARG` for an empty name, a name with
  a blank, tab or double quote, a null table, a `size` below 4, or a qualified name of 64
  characters or more. It returns `GBH_ERR_CONFLICT` when the name is taken and `GBH_ERR`
  when all 64 slots are used. The framework stores `table` as given and never copies it, so
  the table must stay valid and unchanged for the process.
- `service_find(name, &size)` takes the full `<id>.<name>` in any case. It returns the
  published pointer, or `NULL` when nothing is published under that name or `name` is null.
  `size` (which may be `NULL`) receives what the publisher stated, or 0 on a miss. Gate each
  field on `size` before reading it.
- `service_count()`, `service_name_at(i)` and `service_owner(name)` list the directory.
  `service_name_at` returns the qualified lower-case name, or `NULL` out of range.
  `service_owner` returns the publisher's id, or `NULL` for an unknown name.

A mod that needs another's service loads after it: a later stage, or a higher `priority`
number in the same stage. `service_find` answers `NULL` until the publisher's init has run.

```c
typedef struct MyUi { uint32_t struct_size; void (*toast)(const char* text); } MyUi;
/* the publisher, id "mymod": found by others as "mymod.ui" */
static const MyUi g_ui = { sizeof(MyUi), Toast };
api->service_publish("ui", &g_ui, sizeof g_ui);

/* a consumer */
uint32_t n = 0;
const MyUi* ui = (const MyUi*)api->service_find("mymod.ui", &n);
if (ui && n >= offsetof(MyUi, toast) + sizeof ui->toast) ui->toast("hi");
```

### Memory reads, any thread

`int mem_read(const void* src, void* dst, size_t n)` copies `n` bytes out of process memory
under a guard. It returns 1 when copied. It returns 0 when `src` or `dst` is null, `n` is 0,
the range is unmapped, or the read faulted. After a 0, the contents of `dst` are undefined.

### Levels, engine main thread only

```c
#define GBH_LEVELS_CAREER  0
#define GBH_LEVELS_CUSTOM  1
int level_list(int kind, void (*cb)(const char* stem, void* user), void* user);
int level_checkpoints(const char* stem, void (*cb)(const char* name, void* user), void* user);
```

These are the lists the Mods page shows, with the same thread rule and the same
`GBH_ERR_WRONG_THREAD` as the file calls. Both return the count, and `cb` runs once per item
before the call returns, with a string valid for that call only.

- `level_list` returns `GBH_ERR_ARG` for a null `cb` or a `kind` other than the two
  constants. `GBH_LEVELS_CAREER` is the engine's own table, in its order, and
  `GBH_ERR_STATE` means it could not be read. `GBH_LEVELS_CUSTOM` is every other
  `world\*.lvl` a mounted archive holds, minus the career levels, de-duplicated and sorted
  ignoring case. Stems have no `.lvl`.
- `level_checkpoints(stem, ...)` reads `world\<stem>.dante` and lists the checkpoint
  functions it declares, in script order, as their script names (`checkpoint_OldStacks`).
  A level with no readable script returns 0, which is not an error. `GBH_ERR_ARG` for a null
  or empty `stem` or a null `cb`.

`level_chain` loads one of them.

### Actors, guarded reads, any thread

```c
int actor_snapshot(GbhActorInfo* buf, int cap);
int actor_find(const char* name, GbhActorInfo* out);
int actor_is_a(void* actor, const char* cls);
```

These read the engine's own list of every `CActor` in the level, out of game memory under a
guard. The list is a superset of the VM registry. The registry sees what the script exported,
and the list sees everything, including the spawn pool.

```c
typedef struct GbhActorInfo {
    uint32_t struct_size;
    void*    ptr;            /* live game memory: game thread only, stale after the next level prepare */
    char     name[64];       /* "?" when the node held no printable name */
    char     cls[48];        /* the RTTI class name, "?" when the vtable was unreadable */
    float    pos[3];
    float    orient[3];
    int32_t  team;
    uint32_t flags;          /* GbhActorFlags */
    int32_t  last_frame;     /* the engine frame that last updated it */
    uint32_t generation;     /* the level prepare it was read in */
} GbhActorInfo;
```

`struct_size` is set by the caller, in `buf[0]` or in `out`, to `sizeof(GbhActorInfo)` as
the caller knows it. It is the stride between entries and how many bytes of each entry are
filled, so a caller built against an older struct reads a shorter one. A value too small to
reach `ptr` answers `GBH_ERR_ARG`. The framework writes the filled size back into each entry.

`flags` is a `GbhActorFlags` mask:

| flag | value | meaning |
|---|---|---|
| `GBH_ACTOR_ENABLED` | 1 | the engine's cookie says live. Clear is the spawn pool: fully built and switched off |
| `GBH_ACTOR_CHARACTER` | 2 | the RTTI chain carries `CCharacter` |
| `GBH_ACTOR_GHOSTBUSTER` | 4 | `CGhostbuster` |
| `GBH_ACTOR_NPC` | 8 | `CNPC` or `CHuman` |
| `GBH_ACTOR_GHOST` | 16 | `CGhost` |
| `GBH_ACTOR_BREAKER` | 32 | `CBreaker`: destructible, native hit points |
| `GBH_ACTOR_ANIMODEL` | 64 | `CAniModel`: an animated set piece |
| `GBH_ACTOR_PHYSOBJ` | 128 | `CPhysicsObjectBase`: a movable prop |

- `actor_snapshot` copies up to `cap` entries and returns how many it wrote. A null `buf`
  with `cap` 0 returns the total. Other combinations with a null `buf` or a `cap` of 0 or
  below return `GBH_ERR_ARG`. `GBH_ERR_STATE` means CGame or its actor chain is unreadable,
  as it is outside a level. The walk follows the chain until its end, an unreadable node or
  16384 nodes.
- `actor_find(name, out)` matches the engine-list name exactly first, then as a substring,
  ignoring case. It returns `GBH_ERR_NOT_FOUND` on a miss.
- `actor_is_a(actor, cls)` returns 1 or 0 by the actor's RTTI chain, comparing ignoring case
  and counting the exact class. It returns `GBH_ERR_ARG` for a null `actor` or a null or
  empty `cls`, and `GBH_ERR_STATE` when the object's vtable or RTTI is unreadable.

A `ptr` is live game memory, for the game thread only, and stale after the next level
prepare. Every snapshot walks the whole chain, so call it as a command costs, not every
frame.

### Attributes, reads any thread, writes on the game thread

Attributes are objective engine state by key, read out of the engine's own memory and
written through the engine's own natives. Nothing here is a mod's remembered toggle. A
toggle the engine exposes no getter for (fly mode, letterbox, the HUD) is a mod's own state
and does not belong here.

```c
typedef enum GbhAttrType { GBH_ATTR_BOOL = 0, GBH_ATTR_FLOAT = 1, GBH_ATTR_INT = 2 } GbhAttrType;

typedef struct GbhAttrInfo {
    uint32_t struct_size;    /* the caller's, sizeof(GbhAttrInfo) */
    char     key[32];
    uint32_t type;           /* GbhAttrType */
    uint32_t writable;       /* 0: no engine native sets it */
    float    min, max;       /* what a setter accepts, both zero when unbounded */
    char     unit[16];       /* "", "x", "deg" */
    char     help[96];
} GbhAttrInfo;
```

| entry | notes |
|---|---|
| `int attr_count(void)` | the number of keys in the catalogue |
| `int attr_at(int i, GbhAttrInfo* out)` | fills one catalogue row. `out->struct_size` must be set by the caller (at least 4), and the framework fills no more than that and writes the filled size back. `GBH_ERR_ARG` for a null `out` or an `i` out of range |
| `int attr_get(const char* key, char* out, int cap)` | the display form: `ON`, `-32.00`, `1.00x`. `GBH_ERR_NOT_FOUND` for an unknown key, `GBH_ERR_STATE` when the engine state cannot be read now, `GBH_ERR_TRUNCATED` when `cap` is too small, `GBH_ERR_ARG` for a null argument or `cap` of 0 or below |
| `int attr_get_float(const char* key, float* out)` | the number. A bool is 0 or 1. Same errors as `attr_get` |
| `int attr_set(const char* key, const char* value)` | see below |

Keys match ignoring case. Each is read live out of the engine, so it survives a level load
or a script changing it behind a mod's back.

| key | type | unit | set through | range and notes |
|---|---|---|---|---|
| `god` | bool | | `CCharacter::setInvulnerableFlag` | |
| `giant` | bool | | `CGhostbuster::enableGiantBossMode` | the Stay Puft camera framing |
| `torpedo` | bool | | `CGhostbuster::enableProtonTorpedo` | |
| `hunt` | bool | | `CGhostbuster::toggleHuntMode` | the pack's hunt flag |
| `gravity` | float | | `Global::setGravity`, `reset` calls `resetGravity` | -100 to 50, the y component. -32 is the engine's normal |
| `time` | float | `x` | `Global::setTimeFactor`, `reset` | 0.05 to 4, ramped over 0.25 s. 1.00x is the engine's own curve |
| `fov` | float | `deg` | read-only | the main view's field of view |
| `camdist` | float | | read-only | the main view's follow distance |
| `cammode` | int | | read-only | the script camera mode: 0 normal, 10 path, 11 fixed, 13 orbit |

`attr_set` runs on the game thread only, and any other thread gets `GBH_ERR_WRONG_THREAD`.
The value is `on`, `off`, `1`, `0`, `true`, `false` or `toggle` for a bool, a number inside
the range for a float, and `reset` where the engine has one (`gravity` and `time`). Its
errors:

- `GBH_ERR_ARG`: a null argument, or a value the key refuses (out of range, not a number).
- `GBH_ERR_NOT_FOUND`: an unknown key.
- `GBH_ERR_UNSUPPORTED`: a read-only key.
- `GBH_ERR_STATE`: the key cannot be read now, for example the player keys with no local
  player.
- `GBH_ERR`: the engine's setter faulted.

A successful set logs the old and new value.

### Keys, message thread

```c
typedef int (*GbhKeyFn)(int vk, int down, void* user);
typedef int (*GbhCharFn)(unsigned int ch, void* user);
GbhSub on_key(GbhKeyFn fn, void* user);
GbhSub on_char(GbhCharFn fn, void* user);
```

The framework subclasses the game window once, when the pump first runs. These callbacks run
on the window's thread, inside its window procedure.

- `on_key` fires for every `WM_KEYDOWN`, `WM_SYSKEYDOWN`, `WM_KEYUP` and `WM_SYSKEYUP`, with
  the virtual-key code and `down` of 1 or 0. Auto-repeat sends repeated downs.
- `on_char` fires for every `WM_CHAR` and `WM_SYSCHAR`, with the character code.
- A subscriber answers 1 to keep the message from the engine's own handler, which is what
  fills its scan-code table. Answering 0 passes it on. The fan-out stops at the first 1.
- Order is registration order, so a menu that loads first sees a key before a mod that
  flies. The mouse is never routed. `input_set_key` is the other direction.
- Raw subscribers run before actions. A text line that keeps typed keys also keeps them from
  firing actions.

Do engine work in an action, `on_frame` or `on_pump`, which run from the tick and the pump.
Record the key in the callback and act later.

### Actions, main thread

An action is a named press the mod registers and the player binds.

```c
typedef void (*GbhActionFn)(const char* name, void* user);

int action_register(const char* name, GbhActionFn fn, void* user, const char* help);
int action_binding(const char* name, char* out, int cap);
int action_held(const char* name);
int action_enable(const char* name, int on);
int action_capture(int on);
```

Actions are per mod: the table key is the calling mod's id plus the name.

| entry | notes |
|---|---|
| `action_register` | `GBH_OK` whether the action got a key or not. `GBH_ERR_CONFLICT` when this mod already has the name. `GBH_ERR_ARG` for a null `fn`, an empty name, a name containing whitespace, or one over 31 characters. `GBH_ERR` when all 128 slots (across every mod) are used. Names are case-sensitive. `help` is shown by `binds` |
| `action_binding(name, out, cap)` | writes the chord as text, such as `CTRL+SHIFT+F5`, or `""` when unbound. `GBH_ERR_NOT_FOUND` for a name never registered, `GBH_ERR_TRUNCATED` when `cap` is too small (the cut text is written), `GBH_ERR_ARG` for a null argument or `cap` of 0 or below |
| `action_held(name)` | 1 while every key of the chord is down, 0 otherwise, `GBH_ERR_NOT_FOUND` for a name never registered. A switched-off or captured-away action reads 0. Any thread |
| `action_enable(name, on)` | `on` of 0 keeps the claim and makes it inert. The key reaches the engine and nothing fires. `GBH_ERR_NOT_FOUND` for an unknown name. An action turned off by a fault stays off |
| `action_capture(on)` | `on` of 1 means only the caller's actions fire. `GBH_ERR_CONFLICT` while another mod holds the capture, for on and off alike |

**Binding.** The chord comes from the setting `bind.<name>`. `bind.menu = F1` in the mod's
`modinfo.ini` is the default, and `mymod.bind.menu` in `gbhook.ini` overrides it. The
setting is read once, when the action is registered. A chord is one key plus any of `CTRL`,
`SHIFT` and `ALT`, joined by `+`, in any case and any order. The sided spellings `LCTRL`,
`RCTRL`, `LSHIFT`, `RSHIFT`, `LALT`, `RALT` and the long forms `LCONTROL`, `RCONTROL`,
`LMENU`, `RMENU` mean the plain modifier, because the window procedure cannot tell the sides
apart. A modifier alone, two keys, a repeated modifier or an unknown name is invalid. An empty
value or `NONE` leaves the action unbound. An invalid value is logged and also leaves it
unbound.

The key names are virtual-key names, not the DIK names of `input_dik_from_name`:
`A` to `Z`, `0` to `9`, `F1` to `F12`, `ESC` or `ESCAPE`, `ENTER` or `RETURN`, `SPACE`,
`TAB`, `BACKSPACE` or `BACK`, `HOME`, `END`, `PGUP` or `PRIOR`, `PGDN` or `NEXT`, `INSERT` or
`INS`, `DELETE` or `DEL`, `UP`, `DOWN`, `LEFT`, `RIGHT`, `NUMPAD0` to `NUMPAD9`, `MULTIPLY`,
`ADD`, `SUBTRACT`, `DECIMAL`, `DIVIDE`, `PAUSE`, `CAPSLOCK` or `CAPITAL`, `NUMLOCK`, `SCROLL`,
`SEMICOLON`, `EQUALS`, `COMMA`, `MINUS`, `PERIOD`, `SLASH`, `GRAVE`, `LBRACKET`,
`BACKSLASH`, `RBRACKET` and `APOSTROPHE`.

**Clashes.** The first claim on a chord wins. A second claim for the same chord, from any
mod, is logged naming both actions and stays unbound. `F` and `CTRL+F` are different chords.
A claim fires when its key goes down and all its modifiers are held, so when several match,
the one with the most modifiers fires, and ties go to the earlier registration. An `F` claim
also fires with Ctrl held when no `CTRL+F` claim exists.

**Firing.** A bound key never reaches the engine, its key-up included. Holding it fires
once. `fn` runs on the main thread from the pump, once per press, under the same guard as an
event, and receives the action's `name` so one function can serve several actions. Up to 64
presses wait for the pump, and more are dropped. When the window loses focus, every key is
treated as released.

**Capture.** A capture is for a menu that wants the keyboard. While one mod holds it, only
its own actions fire, every other mod's bound key is swallowed, and a key nobody bound still
reaches the engine. The `binds` command lists every action, its chord and its help.

### World, any thread

```c
#define GBH_PAUSED_FREEZE  1
#define GBH_PAUSED_SCREEN  2
#define GBH_PAUSED_FOCUS   4
int paused(void);
int pause(int on);
int world_to_screen(const float pos[3], float out[2]);
```

| entry | notes |
|---|---|
| `paused()` | a mask of `GBH_PAUSED_FREEZE` (the engine's controller-disconnect freeze, which `pause` holds), `GBH_PAUSED_SCREEN` (a front-end screen is up: the pause menu, the title) and `GBH_PAUSED_FOCUS` (the window lost focus). 0 while the world ticks |
| `pause(on)` | the caller holds (1) or releases (0) the freeze. Idempotent both ways. Up to eight mods may hold it at once, and a ninth gets `GBH_ERR`. Returns `GBH_OK` otherwise |
| `world_to_screen(pos, out)` | backbuffer pixels. 1 on screen, 0 behind the camera or outside, `GBH_ERR_STATE` when the camera cannot be read, `GBH_ERR_ARG` for a null argument |

The freeze is the byte the level loop itself tests. The world update and the game clock
stop, while rendering, the HUD and audio carry on. The engine clears the byte on a pad
hot-plug or the Steam overlay, so gbhook sets it again every pump tick while any mod holds
it. On the last release gbhook clears the byte only if gbhook set it. If the engine's own
disconnect freeze starts while a mod holds the pause, the last release clears it too. Esc
still opens the game's pause menu during a freeze. The front-end gate is only read, because
forcing it blacks out the frame.

The `SCREEN` bit is the engine's own check when the caller is on the engine's thread. From
any other thread it is read from memory instead, which is a close approximation.

`world_to_screen` is the engine's projection with its own camera, so it follows camera shake
and cinematic cameras. `pos` is x, y, z in world units and `out` receives x, y in pixels. A
point in front of the camera but off screen returns 0 and still fills `out`, so an arrow can
be clamped to the edge. A point behind the camera returns 0 and leaves `out` unchanged.

### Mod pages and written settings

```c
int mods_page_add(const char* label, const GbhNativeMenuDesc* desc);
int setting_set(const char* key, const char* value);
```

**`mods_page_add`** puts one row on the Mods page, under Load Level and View Mods, in code
order. Choosing the row opens `desc` as a native page, the way `native_submenu_open` does.

- Call it from `GbhPluginInit` only. Anywhere else it returns `GBH_ERR_STATE`, as it does
  for a caller that is not a mod.
- One page per mod. A second call is `GBH_ERR_CONFLICT`.
- `GBH_ERR_ARG` for a null `label`, a null `desc`, a `desc` without `build` or `activate`,
  a `struct_size` too small to reach `title`, or a label that is empty after trimming or
  holds a line break.
- `label` is trimmed and cut to 39 characters, ending in `~` when cut. The framework copies
  `desc` and `label`, so neither needs to outlive the call. A `NULL` title shows the label.
- The Mods page holds 38 mod rows. A later page returns `GBH_ERR`.
- The row is hidden when its mod is not in the loaded state.

**`setting_set`** writes `<id>.<key>` to `gbhook.ini` and updates what `setting()` answers
next. A `NULL` value removes the key, and `setting()` then falls back to the mod's
`modinfo.ini` default.

- A key may hold letters, digits, `_`, `.` and `-`, and may not start or end with `.`. A
  value may not hold a line break or `"`. Anything else is `GBH_ERR_ARG`.
- A value holding `#` or `;`, or with a leading or trailing blank, is written quoted so it
  reads back whole. A key that already has a line keeps that line's trailing comment.
- `GBH_ERR_STATE` for a caller that is not a mod. `GBH_ERR` means the file could not be
  written and nothing changed. The file is replaced through a temporary file and a rename.

## 5. The C++ wrapper

`gbhook.hpp` is header-only and compiles into the mod, so it adds nothing to the contract.
It needs `gbhook.h` and the standard library. Call `gbh::bind(api)` first, in
`GbhPluginInit`. Until then every wrapper returns its fallback and does nothing.

Most wrappers keep the table's names. Where they differ:

| wrapper | notes |
|---|---|
| `gbh::api()`, `gbh::bind(api)` | the stashed table pointer |
| `gbh::log`, `gbh::logf` | `logf` formats into 1024 bytes |
| `gbh::setting`, `setting_int`, `setting_float`, `setting_bool` | `setting` returns `std::string`. The defaults are `""`, 0, 0 and `true` |
| `gbh::game_base`, `game_dir`, `mod_dir` | `game_dir` and `mod_dir` return `std::string` |
| `gbh::at<T>(rva)` | a `ghost.exe` offset as a `T*` |
| `gbh::hook`, `hook_at(rva, ...)`, `enable_batch` | return `GbhHook` (null on failure) and `bool`. There are no wrappers for `hook_enable` and `hook_disable`: call `gbh::api()->hook_enable(h)` |
| `gbh::Patch`, `gbh::VtableOverride`, `gbh::Sub` | handle classes, below |
| `gbh::on_frame`, `on_pump`, `on_level`, `on_actor_registered` | return a `gbh::Sub` |
| `gbh::game_singleton`, `local_player`, `is_game_thread`, `level_name` | `level_name` returns `std::string` |
| `gbh::registry()`, `registry_find` | `registry()` returns a `std::vector` with 64 entries of headroom trimmed to the count |
| `gbh::command_register`, `run`, `queue` | `help` defaults to `""` |
| `gbh::dik`, `key`, `hud`, `level` | `hud` defaults to 3 seconds, `level` to no checkpoint |
| `gbh::native_row_claim`, `native_row_label`, `native_submenu_open`, `native_submenu_add_row`, `native_submenu_refresh` | `native_submenu_open` takes a `const GbhNativeMenuDesc&` |
| `gbh::files(dir, pattern)`, `file_read(path)` | return vectors. Errors give an empty vector. `file_read` calls the table twice, once to size and once to copy |
| `gbh::service_publish`, `service_find<T>(name)` | `service_find<T>` returns null when the table is shorter than a `uint32_t`. It does not check against `sizeof(T)`: gate each field on the size you read from `gbh::api()->service_find` |
| `gbh::mem_read`, `read<T>(src, out)` | `bool` |
| `gbh::levels(kind)`, `checkpoints(stem)` | vectors of strings |
| `gbh::actors()`, `actor_find`, `actor_is_a` | `actors()` sets `struct_size` and returns a vector with 64 entries of headroom trimmed to the count |
| `gbh::attrs()`, `attr(key)`, `attr_float`, `attr_bool`, `attr_set` | `attr` returns the display string in a 64-byte buffer, empty on failure |
| `gbh::on_key`, `on_char` | return a `gbh::Sub` |
| `gbh::action_register`, `action_binding`, `action_held`, `action_enable`, `action_capture`, `paused`, `pause`, `world_to_screen` | `world_to_screen` takes pointers and answers true only for 1 |
| `gbh::mods_page_add`, `setting_set`, `setting_clear(key)` | `setting_clear` is `setting_set(key, nullptr)` |

There are no wrappers for `alloc`, `realloc`, `free`, `framework_version`, `game_thread_id`,
`mod_count`, `mod_id_at`, `mod_is_loaded`, `service_count`, `service_name_at`,
`service_owner`, `attr_count`, `attr_at` or `unsubscribe`. Reach them through `gbh::api()`.
`gbh::attrs()` covers the catalogue and `gbh::Sub` covers unsubscribing.

**Feature tests.** `gbh::has_services()`, `gbh::has_actions()` and `gbh::has_mod_pages()`
apply the blocks of section 1. Each wrapper in a block answers as unsupported without its
test: `GBH_ERR_UNSUPPORTED` for an `int` entry, null or empty or false otherwise. The
wrappers added after `file_read` all perform this test, and the block starting at
`action_register` is the one that arrived in 0.2.5.

**`gbh::Sub`** holds a subscription and unsubscribes when it is destroyed. It is move-only.
`active()` says whether it holds one, `reset()` unsubscribes now, and `release()` gives up
the handle and keeps the subscription for the process. Discarding a `Sub` returned by
`gbh::on_frame(...)` unsubscribes at once, so a subscription meant to live calls `.release()`:

```cpp
gbh::on_frame(OnFrame).release();    // lives for the process
static gbh::Sub s = gbh::on_frame(OnFrame);   // lives as long as s
```

**`gbh::Patch`** holds a byte patch and reverts it when it is destroyed. It is move-only.
`ok()` says the patch took, `reset()` reverts now, and `release()` returns the `GbhPatch`
handle and keeps the patch for the process.

```cpp
void* fn = (void*)MyFn;
gbh::Patch p(gbh::at<void>(0x7C0000), &fn, sizeof fn);   // a vtable slot, placeholder offset
if (p.ok()) p.release();
```

**`gbh::VtableOverride`** wraps `vtable_clone`. `ok()` says the clone exists and `clone()`
returns it. `set<Fn>(slot, fn, &original)` claims a slot and stores the shipped entry in
`original`. `apply(object)` points an object at the clone, and `original<Fn>(slot)` returns
a shipped entry. It has no destructor action: the copy belongs to the framework for the
process.

## 6. SEH helpers

`sdk/include/gbhook/seh.h` defines two macros that guard a mod's own engine calls.

```cpp
#include "gbhook/seh.h"

static bool ReadHealth(const char* p, float* out)
{
    GBH_SEH_TRY { *out = *(const float*)p; return true; }
    GBH_SEH_EXCEPT { return false; }
}
```

- Under MSVC, `GBH_SEH_TRY` is `__try` and `GBH_SEH_EXCEPT` is
  `__except (EXCEPTION_EXECUTE_HANDLER)`. An access violation inside the block runs the
  handler block. `seh.h` includes `<excpt.h>`, so a file without `windows.h` compiles.
- Under any other compiler they become `if (1)` and `else`. The code compiles, so mingw can
  compile-check a mod, and nothing is caught.
- A guarded function must be a leaf with no C++ object that needs unwinding. MSVC refuses
  the function otherwise (C2712). Keep the guarded call in a small plain-C-style function.
- Build the mod with `/EHa`. `sdk/GbHookPlugin.props` sets `ExceptionHandling` to `Async`.

Use `mem_read` for a plain guarded copy. Use these macros for a call into the engine.

## 7. Constants and limits

| name | value | where |
|---|---|---|
| `GBHOOK_ABI_VERSION` | 1 | section 1 |
| `GBHOOK_TARGET_MD5`, `GBHOOK_TARGET_NAME` | the verified build | section 1 |
| `GBH_STAGE_COUNT` | 4 | section 2 |
| `GBH_MANIFEST_MAGIC` | `"GBHOOKMF"` | section 2 |
| `GBH_MAX_EXCLUSIVE` | 16 claims of 63 characters | section 2 |
| `GBH_EXPORT_MANIFEST`, `GBH_EXPORT_INIT` | `"GbhPluginManifest"`, `"GbhPluginInit"` | section 2 |
| `GBH_MAX_PATCH_BYTES` | 64 | Patches |
| hook and patch overlap window | 20 bytes at a hooked address | Patches |
| vtable clone slots | 1 to 4096 | Vtables |
| subscribers per bus | 64 | Events |
| frame budget | 250 microseconds average, reported per 600 frames | Events |
| command queue | 64 lines of 511 characters, 8 run per frame | Commands |
| `GBH_ROW_ONLINE`, `GBH_ROW_FREE` | 1 and 3 | The native menu |
| `GBH_NATIVE_LABEL_CAP` | 40, so a label holds 39 characters | The native menu |
| `GBH_NATIVE_MAX_ROWS` | 40 per page | The native menu |
| `GBH_NATIVE_STAY`, `CLOSE`, `CLOSE_ALL`, `INERT` | 0, 1, 2, -1 | The native menu |
| page nesting | 16 | The native menu |
| row label set by `native_row_label` | 63 characters | The native menu |
| mod rows on the Mods page | 38 | Mod pages |
| `file_read` path | under 240 characters, no `..` | Files |
| `file_read` size | stops past 64 MiB | Files |
| `file_list` | `dir` plus `pattern` under 200 characters, 4096 names | Files |
| services | 64, qualified name under 64 characters | Services |
| actions | 128 across all mods, name up to 31 characters, 64 queued presses | Actions |
| pause holders | 8 | World |
| actor list | 16384 nodes | Actors |
| `GBH_LEVELS_CAREER`, `GBH_LEVELS_CUSTOM` | 0, 1 | Levels |
| `GBH_PAUSED_FREEZE`, `SCREEN`, `FOCUS` | 1, 2, 4 | World |
| `GBH_ATTR_BOOL`, `FLOAT`, `INT` | 0, 1, 2 | Attributes |
| `GbhRegistryEntry` | `cls` 48 bytes, `name` 64 bytes | The game model |
| `GbhActorInfo` | `name` 64 bytes, `cls` 48 bytes | Actors |
| `GbhAttrInfo` | `key` 32, `unit` 16, `help` 96 bytes | Attributes |
