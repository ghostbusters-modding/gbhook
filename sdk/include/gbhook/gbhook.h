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

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* GBHOOK_H */
