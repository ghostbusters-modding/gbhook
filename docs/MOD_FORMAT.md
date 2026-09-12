# The mod format

A mod is one folder carrying assets, code and metadata together. It installs by being
copied into `<gamedir>/mods/` and uninstalls by being deleted.

The format is a superset of the **Ghostbusters Mod Manager's**. An asset-only mod is the
same folder under both tools, and a mod with a `gbhook/` half still deploys its assets
through the manager, which never looks inside `gbhook/`.

## 1. The folder

```
My_Mod_v0.1.0/                    release wrapper, "<mod name>_v<version>", for the upload
├── my_mod/                       the mod folder: this is what gets installed
│   ├── previews/                 the Mod Manager's, untouched
│   │   ├── modinfo.ini
│   │   └── preview_01.png
│   ├── art/  data/  world/       loose assets, mirroring the game's archive tree
│   │   sets/  models/ ...
│   └── gbhook/                   gbhook's half
│       ├── mod.ini               required if this folder exists
│       └── MyMod.dll             optional code
└── README.txt
```

Installed, the mod lives at `<gamedir>/mods/my_mod/`. A code-only mod is the same format
with everything optional removed:

```
my_tool/
└── gbhook/
    ├── mod.ini
    └── MyTool.dll
```

## 2. Two identifiers

| tool | key | form |
|---|---|---|
| the Mod Manager | the folder name | `my_mod`, lowercase, fixed once published |
| gbhook | `id` in `mod.ini` | `gb.mymod`; also the command, settings and log namespace |

## 3. `gbhook/mod.ini`

Required whenever `gbhook/` exists. Flat `key = value` inside sections. A `#` or `;`
starts a comment, mid-line too, so a value can never contain either. Lists are
comma-separated. The same parser reads `gbhook.ini`.

```ini
[mod]
format      = 1
id          = gb.mymod

[gbhook]
abi         = 1
plugin      = MyMod.dll     ; a DLL beside this file, or leave the key out
stage       = boot                ; preboot | early | boot | ready
priority    = 100                 ; within a stage, low runs first
;requires   = gb.othermod         ; mod ids that must be present and loaded
;disabled   = 1                   ; the manual off switch: the whole mod, assets included

[settings]
spawn_rate  = 4                   ; this mod's defaults, in its own namespace
```

| key | meaning |
|---|---|
| `format` | the version of this document the mod was written against. An unknown format refuses the mod whole. |
| `id` | unique across every root. The namespace for commands, settings and log lines. |
| `version`, `description`, `author` | code-only mods only: a mod with `previews/modinfo.ini` states them there, and a `version` here is then ignored with a log line saying so. |
| `abi` | `GBHOOK_ABI_VERSION` from `gbhook.h`. Cross-checked against the DLL's manifest. |
| `plugin` | a DLL under `gbhook/`, or absent. |
| `stage` | when the DLL's init runs. Names, never numbers: a number baked into a shipped mod is what forced an ABI break the last time a stage was inserted. |
| `priority` | within a stage, low runs first. Default 100. |
| `requires` | mod ids that must be present and accepted, else this mod is refused. It does not order anything: see section 6. |
| `disabled` | `1` switches the whole mod off, content included. gbhook reads it and never writes it. |
| `[settings]` | defaults in the mod's namespace. `gbhook.ini` overrides them as `<id>.<key>`. |

Two keys are parsed and reserved: `content`, a list of prebuilt archives under `gbhook/`,
is reported when two mods name the same archive but is not mounted; `scripts` is not read.
Neither belongs in a shipped `mod.ini` yet.

## 4. Which file owns what

| file | owns | required |
|---|---|---|
| `previews/modinfo.ini` | human metadata: `version`, `compatibility`, `description`, `link` | any mod shipping assets |
| `gbhook/mod.ini` | loading facts: what exists, what to load, in what order, with what defaults | when `gbhook/` exists |
| the DLL's manifest | what only the binary can assert: `id`, `abi`, the target `ghost.exe` md5, exclusive hook claims | when a DLL exists |

A mod shipping assets needs `modinfo.ini` because the Mod Manager requires it, since its
v7.0.0. A code-only mod has no `previews/` folder at all and states its version in `[mod]`.

gbhook reads `modinfo.ini` and never writes it, and no gbhook key goes in it. The manager
rewrites that file on a user action, through Qt's `QSettings`: comments stripped, keys
reordered, values re-quoted. Thus we cannot use that to store information.

`id` and `abi` are stated twice on purpose, in `mod.ini` and in the manifest. The ini is
hand-editable and the DLL is not, so a mismatch means one was edited after the build, and
the mod is refused with both values in the log.

## 5. Content

The loose tree is what the Mod Manager deploys, and it is what gbhook loads too, but not
loosely. The engine searches loose files after every mounted archive, so a loose file can
add an asset path but never override one an archive already holds. gbhook therefore builds
the tree into an archive:

```
<gamedir>/gbhook/cache/<id>/<hash>.POD
```

The hash is of the tree's file names, sizes and times, so an unchanged mod costs nothing at
boot, a changed one is rebuilt, and a cache with no mod behind it is removed. The archive is
mounted with the engine's own mount call at revision 1: above the game's bulk archives,
below a Mod Manager `PATCH.POD` chain, so a mod already deployed through the manager is left
to it. `gbhook/` and `previews/` are never packed.

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
mods.root = mods        ; comma-separated; <gamedir>-relative or absolute
```

Default `<gamedir>/mods`. Extra roots are opt-in because the Mod Manager's own folder can
be anywhere. Every configured root that does not exist is logged, and so is any mod folder
gbhook can see that carries a `gbhook/` half it was not asked to load. Detection, not
correction.

## 8. Settings

A mod's `[settings]` block supplies defaults, keyed as the mod reads them. `gbhook.ini`
overrides them under the mod's id, so a mod works out of the box and the user's edit wins:

```ini
# mods/harbor_docks/gbhook/mod.ini
[settings]
spawn_rate = 4

# <gamedir>/gbhook.ini
gb.mymod.spawn_rate = 12
```

## 9. What the log says

At boot, under `MODS`: each root, then one line per mod in code order, then the content
build. A refused mod is one line, `REFUSED <id>: <reason>`, and the same reason is on the
mod's page under View Mods. The reasons:

| reason | fix |
|---|---|
| `gbhook/ has no mod.ini` | add one |
| `mod.ini says abi N` | rebuild against this gbhook, or set `abi` to match |
| `mod.ini says id 'x'` | the DLL's manifest says another; make them agree |
| `duplicate id` | two folders claim one id |
| `requires 'x'` | the named mod is absent or refused |
| `plugin 'x'` | the DLL is missing, or its manifest is bad or for another game build |

A DLL that loads but fails its init is `FAIL <id>: <reason>` and inert. A callback that
faults later loses that one subscription, not the mod, and the log names both.
