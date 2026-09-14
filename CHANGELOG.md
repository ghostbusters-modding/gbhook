# Changelog

Framework releases. The ABI version stays at 1 while the table only grows; `docs/ABI.md`
section 1 says when it would not.

## 0.2.0 (2026-09-13)

- `actor_snapshot`, `actor_find`, `actor_is_a`: the engine's own actor list, the spawn pool
  included, with the class read out of the RTTI. `actors [filter]` and `actor <name>` in
  `gbhook.cmd`.
- `attr_count`, `attr_at`, `attr_get`, `attr_get_float`, `attr_set`: objective engine state by
  key (`god`, `giant`, `torpedo`, `hunt`, `gravity`, `time`, `fov`, `camdist`, `cammode`), read
  out of memory and set through the engine's natives. `attr` in `gbhook.cmd`.
- `level_list`, `level_checkpoints`: the lists behind the Mods page, now a service the menu and
  the ABI share.
- Field offsets in `sdk/include/gb/Structs` and the physics and time globals in `Globals.h`.
- Pure packages `actors` and `attr` with suites.
- `service_publish`, `service_find`, `service_count`, `service_name_at`, `service_owner`: a
  C table one mod publishes and others find by name, the way `gb.menu.ui` reaches every mod.
  `services` in `gbhook.cmd`.
- `on_key`, `on_char`: the game window subclassed once by gbhook, keys fanned out on the
  message thread; a subscriber answering 1 keeps the key from the engine.
- `mem_read`: the guarded copy every mod used to carry itself.
- `sdk/include/gbhook/seh.h`: SEH guards for a mod's own engine calls, inert under mingw.
- Pure package `svc` with its suite; SelfTest covers the new entries.

## 0.1.0 (2026-09-11)

The rewrite: the mod format, content build, native menu, commands, events, hooks.
