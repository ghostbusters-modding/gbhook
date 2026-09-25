/* gbhook: a mod loader for Ghostbusters: The Video Game Remastered
 * Copyright (C) 2026 Colin Sullivan and contributors
 * SPDX-License-Identifier: GPL-2.0-only */
#ifndef GBHOOK_H
#define GBHOOK_H
/* The mod ABI. Pure C, and the only header a mod needs. docs/ABI.md is the contract in prose. */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bumped only by a breaking change. struct_size for appending to GbH. */
#define GBHOOK_ABI_VERSION      1

/* The one ghost.exe build every offset was verified against. A mod naming another build is refused. */
#define GBHOOK_TARGET_MD5       "0b89556c07e5b737efe444351227e747"
#define GBHOOK_TARGET_NAME      "Ghostbusters: TVG Remastered (Steam, x64)"

/* Load stages. A mod names its stage in modinfo.ini, so these numbers never reach a shipped binary. */
typedef enum GbhStage {
    GBH_STAGE_PREBOOT = 0,   /* before any framework service: the cold-boot screen race */
    GBH_STAGE_EARLY   = 1,   /* core services up, before any level can load */
    GBH_STAGE_BOOT    = 2,   /* the default */
    GBH_STAGE_READY   = 3    /* after the native menu broker */
} GbhStage;
#define GBH_STAGE_COUNT         4

/* Every int-returning entry point answers with one of these. */
typedef enum GbhStatus {
    GBH_OK                  =  0,
    GBH_ERR                 = -1,   /* unspecified */
    GBH_ERR_ARG             = -2,   /* a NULL or out-of-range argument */
    GBH_ERR_STATE           = -3,   /* not valid right now: no level, too early */
    GBH_ERR_UNSUPPORTED     = -4,   /* the framework is older than this call */
    GBH_ERR_CONFLICT        = -5,   /* another mod owns it */
    GBH_ERR_NOT_FOUND       = -6,
    GBH_ERR_WRONG_THREAD    = -7,   /* must be called on the game thread */
    GBH_ERR_TRUNCATED       = -8    /* output buffer too small; out is valid */
} GbhStatus;

/* Opaque handles. Zero is never valid. */
typedef struct GbhHookT*   GbhHook;    /* one installed detour */
typedef struct GbhPatchT*  GbhPatch;   /* one recorded byte patch */
typedef struct GbhSubT*    GbhSub;     /* one event subscription */

typedef enum GbhHookFlags {
    GBH_HOOK_NONE      = 0,
    GBH_HOOK_DEFERRED  = 1 << 0,   /* create but do not enable; hook_enable_batch arms a set together */
    GBH_HOOK_EXCLUSIVE = 1 << 1    /* refuse if anyone already hooked the target; matches a manifest claim */
} GbhHookFlags;

/* The most bytes one patch_write may cover. */
#define GBH_MAX_PATCH_BYTES     64

/* ---------------------------------------------------------------------------
 *  The manifest: what only the binary can assert. Everything else lives in previews/modinfo.ini.
 *  Read out of the DLL file without running it, so every field is an inline array.
 * ------------------------------------------------------------------------- */
#define GBH_MANIFEST_MAGIC      "GBHOOKMF"
#define GBH_MAX_EXCLUSIVE       16

typedef struct GbhManifest {
    char     magic[9];                            /* GBH_MANIFEST_MAGIC, NUL included */
    uint32_t struct_size;                         /* sizeof(GbhManifest) at build time */
    uint32_t abi_version;                         /* GBHOOK_ABI_VERSION; cross-checked against modinfo.ini */
    char     id[64];                              /* cross-checked against modinfo.ini */
    char     target_md5[40];                      /* GBHOOK_TARGET_MD5 */
    char     exclusive_hooks[GBH_MAX_EXCLUSIVE][64];  /* "ghost+0xHEX", terminated by an empty entry */
} GbhManifest;

#if defined(_WIN32)
#  define GBHOOK_EXPORT __declspec(dllexport)
#else
#  define GBHOOK_EXPORT __attribute__((visibility("default")))
#endif

/* Looked up by exported name in a file that is never executed, so it must not be C++-mangled. */
#ifdef __cplusplus
#  define GBHOOK_LINKAGE extern "C"
#else
#  define GBHOOK_LINKAGE
#endif

#define GBH_EXPORT_MANIFEST     "GbhPluginManifest"   /* data, required */
#define GBH_EXPORT_INIT         "GbhPluginInit"       /* code, required */

/* GBHOOK_PLUGIN("mymod"); at file scope in exactly one translation unit. */
#define GBHOOK_PLUGIN(id_) \
    GBHOOK_LINKAGE GBHOOK_EXPORT const GbhManifest GbhPluginManifest = { \
        GBH_MANIFEST_MAGIC, sizeof(GbhManifest), GBHOOK_ABI_VERSION, id_, GBHOOK_TARGET_MD5, {{0}} }

/* Long form, for exclusive hook claims:
 *   GBHOOK_PLUGIN_EXCLUSIVE("gbcoop") { "ghost+0x46A110", "" } GBHOOK_PLUGIN_END; */
#define GBHOOK_PLUGIN_EXCLUSIVE(id_) \
    GBHOOK_LINKAGE GBHOOK_EXPORT const GbhManifest GbhPluginManifest = { \
        GBH_MANIFEST_MAGIC, sizeof(GbhManifest), GBHOOK_ABI_VERSION, id_, GBHOOK_TARGET_MD5,
#define GBHOOK_PLUGIN_END }

/* ---------------------------------------------------------------------------
 *  Events. Every callback runs under the framework's guard: the first fault names the mod and the event,
 *  drops that one subscription for the process, and the game continues. Order is registration order.
 * ------------------------------------------------------------------------- */
/* Game thread, once per frame while a level is live. A shared budget on the engine's critical path: keep it short. */
typedef void (*GbhFrameFn)(void* user);

/* Game thread, during a level load, once per Dante VM global registration: cls "CGhostbuster", name "Egon". */
typedef void (*GbhActorFn)(const char* cls, const char* name, void* ptr, void* user);

/* Game thread. `level` is the stem, "firehouse". At PREPARE_END, `ok` says whether the engine accepted the level. */
typedef enum GbhLevelPhase {
    GBH_LEVEL_PREPARE_BEGIN = 0,
    GBH_LEVEL_PREPARE_END   = 1
} GbhLevelPhase;
typedef void (*GbhLevelFn)(int phase, const char* level, int ok, void* user);

/* A command handler. The framework routes it to the game thread, so a GB:: native is safe inside. argv holds the
 * arguments only. Return GBH_OK, or point *err at a static string and return a GbhStatus. */
typedef int (*GbhCommandFn)(int argc, const char* const* argv, const char** err, void* user);

/* ---------------------------------------------------------------------------
 *  The game's own main menu. A claimed row is un-hidden and its activation routed to you, and from inside that
 *  callback a real page can be pushed, built from the rows you publish. A page may push another from inside its
 *  activate, nesting the way the game's own screens do; ESC pops one page. Game thread throughout.
 *  Rows are slots on screen, top to bottom: Career, Online, Mods, a free slot, Options, Extras, Exit. Online and
 *  the free slot are claimable and stay hidden until claimed. The others are refused with GBH_ERR_CONFLICT.
 * ------------------------------------------------------------------------- */
typedef void (*GbhRowFn)(int row, void* user);

#define GBH_ROW_ONLINE          1
#define GBH_ROW_FREE            3

#define GBH_NATIVE_LABEL_CAP    40
#define GBH_NATIVE_MAX_ROWS     40
#define GBH_NATIVE_STAY         0     /* rebuild and redraw this page; also the answer after pushing a child page */
#define GBH_NATIVE_CLOSE        1     /* pop this page, back to the one under it */
#define GBH_NATIVE_CLOSE_ALL    2     /* pop every page, back to the main menu, one close animation each */
#define GBH_NATIVE_INERT        -1    /* a row action for a header or a text line: never activates */

typedef struct GbhNativeMenuDesc {
    uint32_t    struct_size;
    void      (*build)(void* user);                 /* publish rows with native_submenu_add_row; on open and after every activation */
    int       (*activate)(int action, void* user);  /* the chosen row's action; GBH_NATIVE_STAY or GBH_NATIVE_CLOSE */
    void*       user;
    const char* title;                              /* verbatim; a leading '@' names a localisation key; NULL keeps the donor's */
} GbhNativeMenuDesc;

/* One VM global registration. `ptr` is live game memory: game thread only, stale after the next level prepare. */
typedef struct GbhRegistryEntry {
    void*    ptr;
    char     cls[48];
    char     name[64];
    uint32_t generation;   /* bumped at every level prepare */
} GbhRegistryEntry;

/* ---------------------------------------------------------------------------
 *  The engine's own actor list: every CActor in the level, the VM registry's superset, the spawn pool included.
 *  Read out of game memory under a guard. A disabled actor is fully built and switched off, not free.
 * ------------------------------------------------------------------------- */
typedef enum GbhActorFlags {
    GBH_ACTOR_ENABLED     = 1 << 0,   /* the engine's cookie says live; clear is the spawn pool */
    GBH_ACTOR_CHARACTER   = 1 << 1,   /* the RTTI chain carries CCharacter */
    GBH_ACTOR_GHOSTBUSTER = 1 << 2,   /* CGhostbuster */
    GBH_ACTOR_NPC         = 1 << 3,   /* CNPC or CHuman */
    GBH_ACTOR_GHOST       = 1 << 4,   /* CGhost */
    GBH_ACTOR_BREAKER     = 1 << 5,   /* CBreaker: destructible, native hit points */
    GBH_ACTOR_ANIMODEL    = 1 << 6,   /* CAniModel: an animated set piece */
    GBH_ACTOR_PHYSOBJ     = 1 << 7    /* CPhysicsObjectBase: a movable prop */
} GbhActorFlags;

/* struct_size is the caller's: set it in buf[0] (or out) to sizeof(GbhActorInfo); it is the stride and the fill. */
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

/* Level lists, as the Mods page shows them. */
#define GBH_LEVELS_CAREER       0     /* the engine's own table, in its order */
#define GBH_LEVELS_CUSTOM       1     /* every other world\*.lvl a mounted archive holds */

/* ---------------------------------------------------------------------------
 *  Attributes: objective engine state by key, read out of memory and written through the engine's own natives.
 *  Nothing here is a mod's remembered toggle. attr_at enumerates the catalogue with its types and ranges.
 * ------------------------------------------------------------------------- */
typedef enum GbhAttrType {
    GBH_ATTR_BOOL  = 0,
    GBH_ATTR_FLOAT = 1,
    GBH_ATTR_INT   = 2
} GbhAttrType;

typedef struct GbhAttrInfo {
    uint32_t struct_size;    /* the caller's, sizeof(GbhAttrInfo) */
    char     key[32];
    uint32_t type;           /* GbhAttrType */
    uint32_t writable;       /* 0: no engine native sets it; attr_set answers GBH_ERR_UNSUPPORTED */
    float    min, max;       /* what a setter accepts; both zero when unbounded */
    char     unit[16];       /* "", "x", "deg" */
    char     help[96];
} GbhAttrInfo;

/* The game window's keys, on the message thread. Answer 1 to keep the key from the engine; the fan-out stops there. */
typedef int (*GbhKeyFn)(int vk, int down, void* user);
typedef int (*GbhCharFn)(unsigned int ch, void* user);

/* An action's press: main thread, from the pump, once per press. `name` is the action's own, so one fn can serve many. */
typedef void (*GbhActionFn)(const char* name, void* user);

/* What paused() answers. 0 while the world ticks. */
#define GBH_PAUSED_FREEZE       1   /* the engine's controller-disconnect freeze; pause() holds it */
#define GBH_PAUSED_SCREEN       2   /* a front-end screen is up: the pause menu, the title */
#define GBH_PAUSED_FOCUS        4   /* the game window lost focus */

/* ---------------------------------------------------------------------------
 *  The API table. Handed to GbhPluginInit, valid for the life of the process; entries are only appended.
 *  No entry takes a mod handle: the caller is derived from the return address. Returned strings are ours,
 *  valid until this thread's next call. Every buffer is the caller's; nothing transfers ownership.
 * ------------------------------------------------------------------------- */
typedef struct GbhApi {
    uint32_t struct_size;
    uint32_t abi_version;

    /* -- environment ----------------------------------------------------- */
    void*       (*game_base)(void);           /* ghost.exe module base */
    const char* (*game_dir)(void);            /* directory holding ghost.exe, no trailing separator */
    const char* (*mod_dir)(void);             /* this mod's gbhook/ folder */
    uint32_t    (*framework_version)(void);   /* (major << 16) | (minor << 8) | patch */

    /* -- memory: the framework heap, for anything handed across this table -- */
    void* (*alloc)(size_t n);
    void* (*realloc)(void* p, size_t n);
    void  (*free)(void* p);

    /* -- log: <gamedir>\gbhook.log, attributed to the calling mod. `tag` names the subsystem -- */
    void (*log)(const char* tag, const char* line);
    void (*logf)(const char* tag, const char* fmt, ...);

    /* -- settings: gbhook.ini "<id>.<key>", then the mod's own modinfo.ini key, then dflt -- */
    const char* (*setting)(const char* key, const char* dflt);
    int         (*setting_int)(const char* key, int dflt);
    float       (*setting_float)(const char* key, float dflt);
    int         (*setting_bool)(const char* key, int dflt);

    /* -- hooks: the one MinHook in the process. `target` is absolute; `original` receives the trampoline -- */
    GbhHook (*hook_create)(void* target, void* detour, void** original, uint32_t flags);
    int     (*hook_enable)(GbhHook h);
    int     (*hook_disable)(GbhHook h);
    int     (*hook_enable_batch)(const GbhHook* hooks, int count);   /* together: no half-armed window */

    /* -- patches: recorded, arbitrated against detours, re-verified. At most GBH_MAX_PATCH_BYTES -- */
    GbhPatch (*patch_write)(void* at, const void* bytes, size_t n);
    int      (*patch_revert)(GbhPatch p);

    /* -- vtables: one copy per shipped table, slots claimed per mod, every vptr swap recorded and re-verified by the
     *    integrity sweep, so apply only to objects that live for the process -- */
    void* (*vtable_clone)(void* original, int slots);
    int   (*vtable_slot)(void* clone, int slot, void* fn, void** original_fn);
    int   (*vtable_apply)(void* object, void* clone);
    void* (*vtable_original)(void* clone, int slot);

    /* -- introspection: every mod discovery saw, accepted ones first in code order -- */
    int         (*mod_count)(void);
    const char* (*mod_id_at)(int i);
    int         (*mod_is_loaded)(const char* id);

    /* -- events: the contended detours, hooked once and fanned out. Subscribe; never hook these yourself -- */
    GbhSub (*on_frame)(GbhFrameFn fn, void* user);              /* the per-level tick */
    GbhSub (*on_pump)(GbhFrameFn fn, void* user);               /* the main thread every frame, front end included */
    GbhSub (*on_level)(GbhLevelFn fn, void* user);
    GbhSub (*on_actor_registered)(GbhActorFn fn, void* user);
    void   (*unsubscribe)(GbhSub s);                            /* idempotent; a stale or null handle is ignored */

    /* -- game model: guarded reads, any thread. A pointer handed back is live game memory -- */
    void*       (*game_singleton)(void);        /* CGame*, NULL very early */
    void*       (*local_player)(void);          /* NULL in menus and during loads */
    uint32_t    (*game_thread_id)(void);        /* 0 until the first frame */
    int         (*is_game_thread)(void);
    const char* (*level_name)(void);            /* the stem, "" at the front end */
    int         (*registry_snapshot)(GbhRegistryEntry* buf, int cap);      /* this generation; a null buf and cap 0 answers the total */
    int         (*registry_find)(const char* name, GbhRegistryEntry* out); /* exact, then substring, case-insensitive */

    /* -- commands: registered as "<id>.<name>", reached from gbhook.cmd, other mods and the framework -- */
    int (*command_register)(const char* name, GbhCommandFn fn, void* user, const char* help);
    int (*command_run)(const char* line);     /* now, or queued when the handler needs the game thread */
    int (*command_queue)(const char* line);   /* on the game thread at the next frame, the front end included */

    /* -- input: the engine's own scan-code table, the one the game reads. SendInput never reaches it -- */
    void (*input_set_key)(int dik, int down);          /* DIK_* code; held at down until up, like a real key */
    int  (*input_dik_from_name)(const char* name);     /* "W", "ENTER", "LSHIFT", "F5"; -1 when unknown */

    /* -- hud: the engine's own message line at the top of the screen. Any thread; needs a live level -- */
    void (*hud_message)(const char* text, float seconds);

    /* -- level flow: chain to a level from the front end or in play. A checkpoint waits until the level is live -- */
    int  (*level_chain)(const char* level, const char* checkpoint);

    /* -- native menu: the game's own front end. Claims are exclusive, refused by name -- */
    int  (*native_row_claim)(int row, GbhRowFn fn, void* user);
    int  (*native_row_label)(int row, const char* label);           /* your row, verbatim, re-applied after every refill */
    int  (*native_submenu_open)(const GbhNativeMenuDesc* desc);     /* inside your row callback, or a page's activate */
    int  (*native_submenu_add_row)(const char* label, int action);  /* from inside build() only; GBH_NATIVE_MAX_ROWS at most */
    void (*native_submenu_refresh)(void);                           /* rebuild and re-label the open page; cheap when unchanged */

    /* -- files: the engine's own asset enumerator and stream. Engine main thread only, else GBH_ERR_WRONG_THREAD -- */
    int (*file_list)(const char* dir, const char* pattern, void (*cb)(const char* name, void* user), void* user);
    int (*file_read)(const char* path, void* buf, int cap);         /* bytes copied; a null buf and cap 0 answers the size */

    /* -- appended after file_read: gbh_api_has(api, service_publish) gates the block ---------------------- */

    /* -- services: a C table one mod publishes, others find by name. The table lives for the process and starts
     *    with a uint32_t struct_size, like this one, so a consumer gates on what it carries -- */
    int         (*service_publish)(const char* name, const void* table, uint32_t size);   /* GBH_ERR_CONFLICT when taken */
    const void* (*service_find)(const char* name, uint32_t* size);                        /* NULL until published */
    int         (*service_count)(void);
    const char* (*service_name_at)(int i);
    const char* (*service_owner)(const char* name);                                        /* the publisher's id */

    /* -- memory: a guarded copy out of game memory. 1 when copied, 0 when unmapped or faulted. Any thread -- */
    int (*mem_read)(const void* src, void* dst, size_t n);

    /* -- levels: what the Mods page lists. Engine main thread only, like file_list. Returns the count -- */
    int (*level_list)(int kind, void (*cb)(const char* stem, void* user), void* user);   /* GBH_LEVELS_* */
    int (*level_checkpoints)(const char* stem, void (*cb)(const char* name, void* user), void* user);

    /* -- actors: the engine's own list. Guarded reads, any thread; a null buf and cap 0 answers the total -- */
    int (*actor_snapshot)(GbhActorInfo* buf, int cap);
    int (*actor_find)(const char* name, GbhActorInfo* out);   /* exact, then substring, case-insensitive */
    int (*actor_is_a)(void* actor, const char* cls);          /* 1 or 0 by the RTTI chain; GBH_ERR_STATE when unreadable */

    /* -- attributes: reads any thread; attr_set on the game thread, "on|off|toggle" or a number, "reset" where the
     *    engine has one -- */
    int (*attr_count)(void);
    int (*attr_at)(int i, GbhAttrInfo* out);
    int (*attr_get)(const char* key, char* out, int cap);       /* display form: "ON", "-32.00", "1.00x"; GBH_ERR_STATE when unreadable */
    int (*attr_get_float)(const char* key, float* out);         /* bools and ints as their number */
    int (*attr_set)(const char* key, const char* value);

    /* -- keys: the game window subclassed once. Message thread; the first subscriber answering 1 keeps the key -- */
    GbhSub (*on_key)(GbhKeyFn fn, void* user);
    GbhSub (*on_char)(GbhCharFn fn, void* user);

    /* -- actions: named by the mod, bound by the player as `bind.<name>`, one key or a chord such as "CTRL+SHIFT+F5".
     *    A bound key never reaches the engine. The first claim on a chord wins; the loser stays unbound -- */
    int (*action_register)(const char* name, GbhActionFn fn, void* user, const char* help);  /* GBH_OK bound or unbound; GBH_ERR_CONFLICT when this mod has the name */
    int (*action_binding)(const char* name, char* out, int cap);    /* the chord as text, "" when unbound */
    int (*action_held)(const char* name);                           /* 1 while the whole chord is down; GBH_ERR_NOT_FOUND */
    int (*action_enable)(const char* name, int on);                 /* off: the key passes to the engine and nothing fires */
    int (*action_capture)(int on);                                  /* on: only the caller's actions fire. GBH_ERR_CONFLICT when another mod holds it */

    /* -- world: the level loop's freeze byte and the front-end gate, read out of memory. Any thread -- */
    int (*paused)(void);                                            /* GBH_PAUSED_* bits */
    int (*pause)(int on);                                           /* hold or release the freeze; frozen while any mod holds it */
    int (*world_to_screen)(const float pos[3], float out[2]);       /* pixels; 1 on screen, 0 behind or outside; GBH_ERR_STATE when unreadable */
} GbhApi;

/* True when the framework is new enough to carry `member`. */
#define gbh_api_has(api, member) \
    ((api) != 0 && (api)->struct_size >= (offsetof(GbhApi, member) + sizeof((api)->member)))

/* Exported as GbhPluginInit. Called once at the mod's stage; anything but GBH_OK leaves the mod inert. */
typedef int (*GbhPluginInitFn)(const GbhApi* api);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* GBHOOK_H */
