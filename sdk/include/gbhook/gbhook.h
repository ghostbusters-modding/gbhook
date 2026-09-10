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

/* Load stages. A mod names its stage in mod.ini, so these numbers never reach a shipped binary. */
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

typedef enum GbhHookFlags {
    GBH_HOOK_NONE      = 0,
    GBH_HOOK_DEFERRED  = 1 << 0,   /* create but do not enable; hook_enable_batch arms a set together */
    GBH_HOOK_EXCLUSIVE = 1 << 1    /* refuse if anyone already hooked the target; matches a manifest claim */
} GbhHookFlags;

/* The most bytes one patch_write may cover. */
#define GBH_MAX_PATCH_BYTES     64

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* GBHOOK_H */
