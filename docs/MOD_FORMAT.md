# The mod format

A mod is one folder carrying assets, code and metadata together. It installs by being
copied into `<gamedir>/mods/` and uninstalls by being deleted.

The format is the **Ghostbusters Mod Manager's**. Any folder the manager would deploy loads
under gbhook as it is, with no edit. gbhook's keys go in the manager's own
`previews/modinfo.ini`, needed only for a DLL, or to choose the id, stage and settings.
The manager deploys the assets as before either way.

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

## 2. Two identifiers

| tool | key | form |
|---|---|---|
| the Mod Manager | the folder name | `my_mod`, lowercase, fixed once published |
| gbhook | `id` in `modinfo.ini`, or derived from the folder name when there is none | `mymod`; also the command, settings and log namespace |

### Mod ids

- `id` is required once `modinfo.ini` has any gbhook key: `id`, `abi`, `plugin`, `scripts`, `content`, `stage`, `priority` or `requires`.
- It uses only `a-z`, `0-9` and `_`, at most 63 characters. Anything else refuses the mod.
- It is unique across every root. A second folder with the same id is refused, and the first folder found keeps it.
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

## 3. `previews/modinfo.ini`

One file, two owners. The Mod Manager's keys come first, written the way it writes them, in
double quotes. gbhook's keys follow: required for a DLL, and otherwise the folder loads as
content under its own name with the defaults below. A `#` or `;` starts a comment, mid-line
too, except inside a double-quoted value. Lists are comma-separated.

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
;requires   = othermod         ; mod ids that must be present and loaded

spawn_rate  = 4                   ; a setting: any other key is one
```

| key | meaning |
|---|---|
| `version`, `compatibility`, `description`, `link` | the manager's. gbhook reads `version` and `description` and never judges the rest. |
| `id` | unique across every root. The namespace for commands, settings and log lines. |
| `abi` | `GBHOOK_ABI_VERSION` from `gbhook.h`. Cross-checked against the DLL's manifest. |
| `plugin` | a DLL under `gbhook/`, or absent. |
| `stage` | when the DLL's init runs. Names, never numbers: a number baked into a shipped mod is what forced an ABI break the last time a stage was inserted. |
| `priority` | within a stage, low runs first. Default 100. |
| `requires` | mod ids that must be present and accepted, else this mod is refused. It does not order anything: see section 6. |
| `bind.<action>` | the default chord for an action the mod registers, `F1` or `CTRL+SHIFT+F5`. `NONE` or empty leaves it unbound. `gbhook.ini` overrides it as `<id>.bind.<action>`. |
| any other key | a setting default in the mod's namespace. `gbhook.ini` overrides it as `<id>.<key>`. |

Two keys are parsed and reserved: `content`, a list of prebuilt archives under `gbhook/`,
is reported when two mods name the same archive but is not mounted; `scripts` is not read.
Neither belongs in a shipped `modinfo.ini` yet.

## 4. Which file owns what

| file | owns | required |
|---|---|---|
| `previews/modinfo.ini`, the manager's keys | human metadata: `version`, `compatibility`, `description`, `link` | every mod |
| `previews/modinfo.ini`, gbhook's keys | loading facts: what exists, what to load, in what order, with what defaults | for a DLL, or to set the id, stage, priority, `requires` or settings |
| the DLL's manifest | what only the binary can assert: `id`, `abi`, the target `ghost.exe` md5, exclusive hook claims | when a DLL exists |

A folder under a root is a mod when it has any of the three: the manager's `modinfo.ini`, a
`gbhook/` half, or one of the engine's asset roots. Anything else is counted in the log and
left alone. Every published mod needs `modinfo.ini` because the Mod Manager requires it,
since its v7.0.0. A code-only mod carries one too. The manager lists it, and its deploy refuses it as "not
valid" because no asset folder exists, then deploys the rest. That line in its log is the
expected outcome, not a fault.

gbhook reads `modinfo.ini` and never writes it. The manager reads its own keys and leaves
the file as authored, so gbhook's keys survive a deploy byte for byte. Its one
write is the modder-only Force Compatibility action, through `QSettings`: keys reordered,
comments dropped, the keys moved under `[General]`, and gbhook reads that too.

The manager packs everything but `previews/` into `MODS.POD`, `gbhook/` included. That
costs the DLL's size in the archive and nothing else: the engine never asks for that path.

`id` and `abi` are stated twice on purpose, in `modinfo.ini` and in the manifest. The ini is
hand-editable and the DLL is not, so a mismatch means one was edited after the build, and
the mod is refused with both values in the log.

A `gbhook/mod.ini` from before the keys moved is not read. gbhook names it in the log and
otherwise ignores it.

## 5. Content

The loose tree is what the Mod Manager deploys, and it is what gbhook loads too, but not
loosely. The engine searches loose files after every mounted archive, so a loose file can
add an asset path but never override one an archive already holds. gbhook therefore builds
the tree into an archive:

```
<gamedir>/gbhook/cache/<id>/<hash>.POD
```

Only engine content is packed. A file goes in when its top folder is one of the engine's
asset roots and its extension is one the engine reads. Both lists are the retail archives'
inventory:

```
animations  art  cinemats  data  fx  materials  models  physics  sets  skeletal  sound  world

.ani .bfm .bst .cib .cinemat .dante .fnt .fxa .fxe .hbb .jug .lvl .mtb .phys2b
.sbs .sec .skb .smb .smp .snb .subb .tex .tfb .txt .ui
```

Everything else stays where it is: `gbhook/`, `previews/`, a README, a generator tree, build
output, a zip. The log names each root entry left out, and each file under an asset root
whose type the engine has no reader for.

The hash is of the packed files' names, sizes and times. An unchanged mod costs nothing at
boot and a changed one is rebuilt. An edited note or a rebuilt DLL is not packed, so it
changes nothing. A cache with no mod behind it is removed.

The archive is written under a `.tmp` name and renamed once complete. A cached archive's
header is checked against its size before it is trusted. A build cut short therefore leaves
nothing behind but the log line saying it is being rebuilt.

The build runs on its own thread, and each archive is mounted from the front end the moment
it is ready. A mod that needs rebuilding is late for that mod alone. The framework, the Mods
row and the INSERT menu never wait on it, and until its archive lands a level of that mod is
absent from Load Level ▸ Custom. The archive is mounted with the engine's own mount call at
revision 1: above the game's bulk archives, below a Mod Manager `PATCH.POD` chain, so a mod
already deployed through the manager is left to it.

What a mod ships this way: levels (`world\<stem>.lvl`, its `.dante` script, its text), sets,
models, textures, character definitions. A `world\*.lvl` appears under Load Level ▸ Custom
with no further declaration, and its script's registered checkpoints appear under it.

One timing fact: the front end's own strings, `world\en\ui.txt`, are read before the first
mount, so a content mod cannot rename main-menu rows. Per-level text is read at level load
and does override.

Two mods shipping the same loose path are named in the log at boot. The engine's mount
order decides between them silently otherwise.

## 6. Two load orders

They answer different questions and are not merged.

- **Asset order** is the manager's list, and decides who wins a file collision.
- **Code order** is `(stage, priority, id)`, and decides who initialises first. `requires`
  states a dependency and does not order anything: a mod that must init after another
  needs a higher `priority` in the same stage, or a later stage.

gbhook prints the resolved code order at boot.

## 7. Where gbhook looks

```ini
mods_root = mods        ; comma-separated; <gamedir>-relative or absolute
```

Default `<gamedir>/mods`. Extra roots are opt-in because the Mod Manager's own folder can
be anywhere. Every configured root that does not exist is logged, and so is any mod folder
gbhook can see that carries a `gbhook/` half it was not asked to load. Detection, not
correction.

## 8. Settings

Every key in a mod's `modinfo.ini` is a default, keyed as the mod reads it. `gbhook.ini`
overrides it under the mod's id, so a mod works out of the box and the user's edit wins.
A mod can read the manager's keys the same way: `version` is one setting for both tools.
A setting cannot reuse a name gbhook reads for itself, such as `id` or `stage`.

```ini
# mods/harbor_docks/previews/modinfo.ini
spawn_rate = 4

# <gamedir>/gbhook.ini
mymod.spawn_rate = 12
```

`mods_disabled` in `gbhook.ini` turns mods off without touching them: ids or folder names,
comma-separated. It works on a folder with no gbhook keys too. The Mods page's Disable
and Enable rows write it, and the change takes effect at the next start. A mod cannot switch
itself off: `disabled` in `modinfo.ini` is no longer read, and the log names the line.

## 9. What the log says

At boot, under `MODS`: each root, then one line per mod in code order, then the content
build. A refused mod is one line, `REFUSED <id>: <reason>`, and the same reason is on the
mod's page under View Mods. The reasons:

| reason | fix |
|---|---|
| `gbhook/ exists but there is no previews/modinfo.ini` | add one, with an `id` and `abi` |
| `previews/modinfo.ini has no id` | a `gbhook/` folder exists, so a DLL is meant: add gbhook's keys. Without `gbhook/` the folder loads as content and nothing is said |
| `modinfo.ini says abi N` | rebuild against this gbhook, or set `abi` to match |
| `modinfo.ini says id 'x'` | the DLL's manifest says another; make them agree |
| `duplicate id` | two folders claim one id |
| `requires 'x'` | the named mod is absent or refused |
| `plugin 'x'` | the DLL is missing, or its manifest is bad or for another game build |

A DLL that loads but fails its init is `FAIL <id>: <reason>` and inert. A callback that
faults later loses that one subscription, not the mod, and the log names both.
