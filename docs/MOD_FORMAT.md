# The mod format

A mod is one folder carrying assets, code and metadata together. It installs by being
copied into `<gamedir>/mods/` and uninstalls by being deleted.

The format is the **Ghostbusters Mod Manager's**. Any folder the manager would deploy loads
under gbhook as it is, with no edit. gbhook's keys go in the manager's own
`previews/modinfo.ini`, needed only for a DLL, or to choose the id, stage and settings.
The manager deploys the assets as before either way.

This page is the reference for the folder and for what gbhook does with it at boot. The
per-function API is in [ABI.md](ABI.md) and the task-by-task guide is in
[MODDING_GUIDE.md](MODDING_GUIDE.md).

## 1. The folder

```
My_Mod.zip                        the upload: one folder, named for the mod, nothing above it
└── My_Mod/                       the mod folder: this is what gets installed
    ├── LICENSE
    ├── README.txt
    ├── previews/
    │   ├── modinfo.ini           the Mod Manager's keys, then gbhook's
    │   └── preview_01.png
    ├── art/  data/  world/       loose assets, mirroring the game's archive tree
    │   sets/  models/ ...
    └── gbhook/                   the DLL, if there is one, and nothing else
        └── MyMod.dll
```

The zip unpacks into `<gamedir>/mods/` and the mod is installed. There is no wrapper
folder around it and no version in the zip name: the version is `modinfo.ini`'s.
A content-only mod has no `gbhook/` folder at all. A code-only mod is the same
format with everything optional removed:

```
my_tool/
├── previews/modinfo.ini
└── gbhook/MyTool.dll
```

### The minimum

| kind | what must exist |
|---|---|
| content-only | a folder under a mods root with at least one engine asset folder (section 5) or a `previews/modinfo.ini`, and no `gbhook/` folder. No gbhook key is needed. |
| code | `previews/modinfo.ini` with `id`, `abi` and `plugin`, and `gbhook/<plugin>`, a x64 DLL exporting `GbhPluginManifest` and `GbhPluginInit` (the `GBHOOK_PLUGIN` macro in `gbhook.h` writes the manifest). |
| settings only | `previews/modinfo.ini` with `id` and `abi` and no `plugin`. It loads with no code. |

A folder under a mods root with none of `previews/modinfo.ini`, a `gbhook/` folder or an
engine asset folder is not a mod. It is counted in the last `MODS` line and otherwise ignored.

## 2. Two identifiers

| tool | key | form |
|---|---|---|
| the Mod Manager | the folder name | `my_mod`, lowercase, fixed once published |
| gbhook | `id` in `modinfo.ini`, or derived from the folder name when there is none | `mymod`; also the command, settings and log namespace |

### Mod ids

- `id` is required once `modinfo.ini` has any gbhook key: `id`, `abi`, `plugin`, `scripts`, `content`, `stage`, `priority` or `requires`.
- It uses only `a-z`, `0-9` and `_`, at most 63 characters. Anything else refuses the mod. Upper case is refused, not folded.
- It is unique across every root. A second folder with the same id is refused, and the first folder found keeps it. Folders are visited in each root in case-insensitive name order, roots in `mods_root` order.
- A DLL's `GBHOOK_PLUGIN` id must match it exactly, or the mod is refused.

gbhook builds these names from it:

| name | form |
|---|---|
| setting | `<id>.<key>` in `gbhook.ini` |
| key binding | `<id>.bind.<action>` in `gbhook.ini` |
| command | `<id>.<name>` |
| service | `<id>.<name>`, from the name the mod publishes |
| log line | `TAG  [<id>] text` |
| content cache | `gbhook/cache/<id>/` |

### Content-only ids

A folder with no gbhook key in `modinfo.ini`, or no `modinfo.ini` at all, loads as content.
Its id comes from the folder name, the way the Mod Manager names it:

- ASCII letters and digits are kept and lowercased.
- Every run of anything else becomes one `_`.
- A `_` left at either end is trimmed.
- A name with no ASCII letter or digit at all, such as `!!!`, is refused. Set an `id` instead.

| folder | id |
|---|---|
| `DuelArena` | `duelarena` |
| `Duel Arena` | `duel_arena` |
| `My Mod (PC)` | `my_mod_pc` |
| `mp_maps` | `mp_maps` |
| `v1.2 Pack` | `v1_2_pack` |

A content-only mod gets stage `boot` and priority 100, has no code, and its `modinfo.ini` is
read for `version` and `description` only. Other keys in that file are not settings.
A folder name of more than 63 characters after this rewrite is refused by the same id rule.

## 3. `previews/modinfo.ini`

gbhook reads exactly `<mod folder>/previews/modinfo.ini`. There is no other lookup. gbhook
never writes it.

One file, two owners. The Mod Manager's keys come first, written the way it writes them, in
double quotes. gbhook's keys follow: required for a DLL, and otherwise the folder loads as
content under its own name with the defaults below.

Grammar:

- `key = value`, one per line. Keys are case-insensitive and stored lowercase.
- A `#` or `;` starts a comment, mid-line too, except inside a double-quoted value. A quoted value loses its quotes and keeps any `#` or `;`.
- A value is trimmed. An empty value reads as absent, so `id =` is the same as no `id` line.
- If a key repeats, the last line wins for the keys gbhook reads. Every line is kept as a setting default.
- `[section]` lines are skipped and the keys under them are read as if there were none. A UTF-8 byte order mark is ignored.
- A line that is not `key = value`, or a `[` without `]`, refuses the mod, but only once a gbhook key is present.
- Lists are comma-separated, with blank items dropped.

```ini
version="0.1.0"
compatibility="PC"
description="What the mod is, for the manager's listing."
link=""

id          = mymod
abi         = 1
plugin      = MyMod.dll           ; a DLL under gbhook/, or leave the key out
stage       = boot                ; preboot | early | boot | ready
priority    = 100                 ; within a stage, low runs first
;requires   = othermod            ; mod ids that must be present and accepted
bind.fire   = F5                  ; default chord for the action "fire"

spawn_rate  = 4                   ; a setting: any other key is one
```

| key | type, default | effect |
|---|---|---|
| `version` | text, empty | shown in the log and the Mods page. |
| `description` | text, empty | read for the listing. |
| `compatibility`, `link` | text | the manager's. gbhook does not judge them. |
| `id` | `a-z 0-9 _`, at most 63; required | the namespace for commands, settings and log lines. |
| `abi` | whole number; required | must equal the `GBHOOK_ABI_VERSION` of this gbhook (currently 1) and the DLL's manifest. Non-numeric text refuses the mod. |
| `plugin` | file name under `gbhook/`; none | the DLL to load. A value that is absolute, has a drive, or contains `..` refuses the mod. |
| `stage` | `preboot`, `early`, `boot`, `ready`; `boot` | when the DLL's init runs. Names only, any case. |
| `priority` | whole number; 100 | within a stage, low runs first. Negative is allowed. |
| `requires` | list of ids; none | each named mod must be present and accepted, else this mod is refused. It does not order anything: see section 6. |
| `content` | list of file names under `gbhook/`; none | parsed, never mounted. Used only to report two mods naming the same file. Each item is checked like `plugin`. |
| `scripts` | folder under `gbhook/`; none | parsed and checked like `plugin`, then unused. |
| `bind.<action>` | `F1`, `CTRL+SHIFT+F5`, `NONE` or empty; unbound | the default chord for an action the mod registers. `gbhook.ini` overrides it as `<id>.bind.<action>`. |
| any other key | text | a setting default in the mod's namespace. `gbhook.ini` overrides it as `<id>.<key>`. |

`stage` values:

| stage | init runs |
|---|---|
| `preboot` | right after the scan, before any framework service. For beating the engine's boot screens. |
| `early` | core services are up, before any level can load. |
| `boot` | the default. |
| `ready` | after the native menu broker. |

Every key in the file except `disabled` is also a setting default, the gbhook keys and the
manager's keys included. A key that already starts with `<id>.` logs an `INI` warning, because
the id is prefixed again.

`disabled` is no longer read. A line carrying it logs a warning and does nothing.
A `gbhook/mod.ini` is not read either, and logs a warning.

## 4. Which file owns what

| file | owns | required |
|---|---|---|
| `previews/modinfo.ini`, the manager's keys | human metadata: `version`, `compatibility`, `description`, `link` | every published mod |
| `previews/modinfo.ini`, gbhook's keys | loading facts: what exists, what to load, in what order, with what defaults | for a DLL, or to set the id, stage, priority, `requires` or settings |
| the DLL's manifest | what only the binary can assert: `id`, `abi`, the target `ghost.exe` md5, exclusive hook claims | when a DLL exists |

The manifest is read out of the DLL file without running it. It holds the magic `GBHOOKMF`,
the struct size, the ABI version, the id, the `ghost.exe` md5 this gbhook targets
(`0b89556c07e5b737efe444351227e747`) and up to 16 exclusive hook claims of the form
`ghost+0xHEX`. Claims are made with `GBHOOK_PLUGIN_EXCLUSIVE` in `gbhook.h`.

A folder under a root is a mod when it has any of the three: the manager's `modinfo.ini`, a
`gbhook/` folder, or one of the engine's asset folders. Folder names that start with `.`
are skipped.

`id` and `abi` are stated twice on purpose, in `modinfo.ini` and in the manifest. The ini is
hand-editable and the DLL is not, so a mismatch means one was edited after the build, and
the mod is refused with both values in the log.

The Mod Manager reads its own keys and leaves the file as authored, so gbhook's keys survive
a deploy. It packs everything but `previews/` into `MODS.POD`, `gbhook/` included. The engine
never asks for that path, so the DLL costs only its size in the archive. A code-only mod is
listed by the manager and its deploy reports "not valid" because no asset folder exists. That
line in its log is the expected outcome.

## 5. Content

The loose tree is what the Mod Manager deploys, and it is what gbhook loads too, but not
loosely. The engine searches loose files after every mounted archive, so a loose file can
add an asset path but never override one an archive already holds. gbhook therefore builds
the tree into an archive:

```
<gamedir>/gbhook/cache/<id>/<hash>.POD
```

### What goes in

A file is packed when its top folder is one of the engine's asset roots and its extension
is one the engine reads. Both comparisons ignore case. The lists are the retail archives'
inventory:

```
animations  art  cinemats  data  fx  materials  models  physics  sets  skeletal  sound  world

.ani .bfm .bst .cib .cinemat .dante .fnt .fxa .fxe .hbb .jug .lvl .mtb .phys2b
.sbs .sec .skb .smb .smp .snb .subb .tex .tfb .txt .ui
```

The file's path inside the folder becomes its path in the archive, with backslashes.
Files are stored uncompressed.

### What stays out

Everything else stays where it is: `gbhook/`, `previews/`, a README, a generator tree, build
output, a zip. A top-level folder that is not an asset root is never entered. The log names
what was left out for each mod:

```
content <id>: left out gen\, previews\, README.md -- not engine content
content <id>: left out art\crate.png -- not a file type the engine reads
content <id>: left out 12 more file(s) under the asset roots
```

The first line lists top-level folders (with a trailing `\`) and top-level files. The second
is printed for each file under an asset root whose extension the engine has no reader for,
for the first five. The third counts the rest.

### The cache

The folder is `<gamedir>/gbhook/cache/<id>/`. It holds one `<hash>.POD`. The hash is a 64-bit
FNV-1a of every packed file's relative path, size and last-write time, in path order, written
as 16 hex digits.

- Any added, removed, renamed, resized or re-timed packed file changes the hash, and the archive is rebuilt. The old `.POD` is deleted first.
- An edited note or a rebuilt DLL is not packed, so it changes nothing.
- A leftover `.POD.tmp` is deleted at boot.
- A cached archive is checked for a `POD6` header and an index that fits the file before it is trusted. A bad one logs `content <id>: cached <name> is unusable (<why>), rebuilding`.
- The archive is written under a `.tmp` name and renamed once complete, so a build cut short leaves nothing the next boot would trust.
- A cache folder whose id matches no mod folder in any root is removed: `cache for '<id>' removed: no such mod in any root`. A refused or disabled mod keeps its cache.

### The build

The build starts right after the pump is installed and runs on its own thread. Each archive
is mounted from the front end the moment it is ready, through the engine's own mount call.
A mod that needs rebuilding is late for that mod alone. A mod whose cache is current mounts
at once. Until its archive lands, a level of that mod is absent from Load Level, Custom.

A disabled or refused mod gets no build and no mount.

What a mod ships this way: levels (`world\<stem>.lvl`, its `.dante` script, its text), sets,
models, textures, character definitions. A `world\*.lvl` appears under Load Level, Custom
with no further declaration.

Assets the engine has already loaded are not re-read after a mount. The front end's own
strings, `world\en\ui.txt`, are read before the first mount.

### Archive order against the game's archives

The engine looks a file up in mounted archives in slot order and the first match wins. The
slots are sorted by ascending header revision after each mount, and equal revisions keep the
order they were mounted in. A lower revision therefore wins over a higher one.

gbhook writes revision 1 and no chain field. An archive with revision 0, such as the Mod
Manager's `MODS.POD` reached through `PATCH.POD`, sorts ahead of it and wins. The game's own
archives are listed with their revisions by the `pod list` command in the log.

### Mod Manager coexistence

At boot gbhook reads `<gamedir>/PATCH.POD` and follows each archive's next-archive field,
up to 100 links. A missing target ends the chain silently. An archive that is not a readable
POD stops it with `chain: <name> is not a readable POD (<why>)`.

If the chain holds any file, the log says
`PATCH.POD chain holds N file(s) -- a mod already in it is left to the Mod Manager`.
A mod whose every packed path is already in the chain gets no archive of its own:
`content <id>: off -- already in a chained POD -- the Mod Manager deployed it, gbhook stands down`.
If even one packed path is missing from the chain, gbhook builds and mounts the whole tree.

### Duplicate content between mods

When two accepted mods pack the same path (compared ignoring case), the log says
`CONFLICT content <path> shipped by both '<id>' and '<id>' -- code order decides, silently`.
Both archives are mounted at revision 1. Equal revisions keep mount order, and mods are
mounted in code order, so the mod earlier in code order wins the file.

Two mods that list the same file name in `content` are reported at scan time:
`CONFLICT content <name> shipped by both '<id>' and '<id>' -- mount order decides, silently`.

Two mods that claim the same `ghost+0xHEX` in their manifests are reported at scan time:
`CONFLICT hook <addr> claimed by both '<id>' and '<id>' -- whichever loads first wins and the other is refused at install time`.
Both mods stay accepted.

## 6. Two load orders

They answer different questions and are not merged.

- **Asset order** is the archive order above. The Mod Manager's chain wins over gbhook's archives, and among gbhook's archives code order decides.
- **Code order** is `(stage, priority, id)`, and decides who initialises first. `requires`
  states a dependency and does not order anything: a mod that must init after another
  needs a higher `priority` in the same stage, or a later stage. A dependency that inits
  later logs `warning <folder>: requires '<id>', which initialises after it -- give this mod a higher priority or a later stage`.

gbhook prints the resolved code order at boot as numbered `MODS` lines.

## 7. Where gbhook looks

```ini
mods_root = mods        ; comma-separated; <gamedir>-relative or absolute
```

Default `<gamedir>/mods`. A path is taken relative to the game folder unless it starts with
a drive letter or `\\`. Forward slashes are accepted. Only the direct subfolders of a root
are mods. Extra roots are opt-in because the Mod Manager's own folder can be anywhere.
A root that does not exist logs `root <path> does not exist` and shows in the Mods list header.

## 8. Settings and disabling

Every key in a mod's `modinfo.ini` is a default, keyed as the mod reads it. `gbhook.ini`
in `<gamedir>` overrides it under the mod's id, so a mod works out of the box and the user's
edit wins. Only accepted mods' defaults are loaded.

```ini
# mods/harbor_docks/previews/modinfo.ini
spawn_rate = 4

# <gamedir>/gbhook.ini
mymod.spawn_rate = 12
```

`mods_disabled` in `gbhook.ini` turns mods off without touching them: ids or folder names,
comma-separated, any case. It works on a folder with no gbhook keys too and beats any
refusal. A disabled mod is never loaded and gets no content. The Mods page's Disable and
Enable rows write the line and the change takes effect at the next start. A mod cannot switch
itself off.

## 9. What the log says

At boot, under `MODS`: each root, then one line per mod, then the content build.

```
MODS  root <gamedir>\mods
MODS  1. mymod 0.1.0 (boot, priority 100) from My_Mod, plugin MyMod.dll
MODS  2. duel_arena  (boot, priority 100) from Duel Arena, no id in modinfo.ini
MODS  OFF mymod (My_Mod): disabled in gbhook.ini
MODS  REFUSED My_Mod: <reason>
MODS  3 mod folder(s), 2 accepted (1 without an id), 1 off, 0 refused, 0 folder(s) that are not mods
```

`REFUSED` names the folder, since a refused mod may have no id. The same reason is on the
mod's page under View Mods.

### Refusals

| reason in the log | cause and fix |
|---|---|
| `gbhook/ exists but there is no previews/modinfo.ini` | a DLL is meant. Add `previews/modinfo.ini` with gbhook's keys. |
| `previews/modinfo.ini has no id` | a `gbhook/` folder exists and the ini has no gbhook key. Add keys. |
| `modinfo.ini has no id` | a gbhook key is present without `id`. |
| `modinfo.ini has no abi` | add `abi`. |
| `abi 'x' is not a number (the integer GBHOOK_ABI_VERSION)` | fix the value. |
| `built for ABI N, this gbhook speaks ABI 1 -- rebuild the mod` | the ini or the manifest names another ABI. Rebuild. |
| `id is longer than 63 characters` / `id 'x' may only use a-z, 0-9 and _` | fix the id. |
| `folder name 'x' has no letter or digit: set an id in previews/modinfo.ini` | a content-only folder with a name like `!!!`. |
| `modinfo.ini line N is not key = value` | fix the line. Only checked once a gbhook key is present. |
| `plugin 'x' leaves gbhook/`, `scripts 'x' leaves gbhook/`, `content 'x' leaves gbhook/` | absolute, drive or `..` path. |
| `stage 'x' is not one of preboot, early, boot, ready` | fix the value. |
| `priority 'x' is not a number` | fix the value. |
| `plugin 'x' is not in gbhook/` | the DLL is missing. |
| `plugin 'x': <why>` | unreadable DLL: `cannot open file`, `not a PE file (bad DOS header)`, `not x64 -- gbhook is 64-bit only`, `not a DLL`, `no export directory -- did you forget GBHOOK_PLUGIN()?`, `manifest export is a forwarder` and similar. Files over 64 MB are not opened. |
| `plugin 'x': manifest magic mismatch (stale SDK?)` | rebuild against the current SDK. |
| `plugin 'x': built for ABI N, this gbhook speaks ABI 1 -- rebuild the mod` | rebuild. |
| `plugin 'x': manifest is newer than this framework` | the DLL's SDK is newer than gbhook. |
| `plugin 'x': empty id in manifest` | set the id in `GBHOOK_PLUGIN`. |
| `plugin 'x': built for ghost.exe <md5>, this framework targets 0b89556c07e5b737efe444351227e747 -- an offset table applied to the wrong build crashes unreadably` | wrong game build. |
| `modinfo.ini says id 'a' but x says 'b' -- one was edited after the build` | make them agree. |
| `modinfo.ini says abi N but x says M -- one was edited after the build` | make them agree. |
| `duplicate id 'x', already claimed by folder 'y'` | two folders claim one id. |
| `requires 'x', which is not present` / `was refused` / `is disabled in gbhook.ini` | the named mod is absent, refused or off. A refusal can cascade through a chain of `requires`. |

### Other lines a packager can hit

| line | meaning |
|---|---|
| `OFF <id> (<folder>): disabled in gbhook.ini` | listed in `mods_disabled`. |
| `warning <folder>: gbhook/mod.ini is no longer read: its keys go in previews/modinfo.ini` | move the keys. |
| `warning <folder>: line N: 'disabled' is no longer read; list the mod under mods_disabled in gbhook.ini` | remove the line. |
| `warning <folder>: requires '<id>', which initialises after it -- ...` | raise `priority` or use a later `stage`. |
| `INI  warning <id>: modinfo.ini key 'k' repeats the mod id; it is read as '<id>.k'` | drop the prefix from the key. |
| `CONFLICT hook ...`, `CONFLICT content ...` | section 5. |
| `content <id>: left out ...` | section 5. |
| `content <id>: built N file(s) -> gbhook\cache\<id>\<hash>.POD` | archive written. |
| `content <id>: cached, gbhook\cache\<id>\<hash>.POD` | cache current. |
| `content <id>: build FAILED -- <why>` | the archive could not be written, for example `could not read '<file>'` or a rename error. The mod loads without its content. |
| `content <id>: mount FAILED -- <reason>` | the engine refused the archive. |
| `content: X built, Y cached, Z left to the Mod Manager, W without content` | build tally. |
| `content: X of Y archive(s) mounted` | final tally, printed once. |
| `content build thread could not start (error N); building on the boot thread` | build runs inline. |
| `loaded <id> <version> at <stage>` | the DLL's init returned OK. |
| `FAIL <id>: <reason> -- the mod is inert` | see below. |
| `stage <name>: X of Y mod(s) up` | per stage tally. |
| `DISABLED <id>: <reason>` | the framework switched a loaded mod off at runtime. |

A DLL that is accepted but fails to start is `FAIL <id>: <reason>` and inert. The reasons are
`LoadLibrary failed (error N; a missing dependency?)`, `the DLL has no GbhPluginInit export`,
`faulted inside GbhPluginInit` and `GbhPluginInit returned N`. The DLL is loaded with an
altered search path, so a helper DLL beside it resolves first. A callback that faults later
loses that one subscription, not the mod, and the log names both. An exclusive hook
refused at install time logs `HOOK  CONFLICT <addr>: '<id>' wants it, '<id>' already owns it -- refusing`.
