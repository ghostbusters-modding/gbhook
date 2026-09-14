/* gbhook: a mod loader for Ghostbusters: The Video Game Remastered
 * Copyright (C) 2026 Colin Sullivan and contributors
 * SPDX-License-Identifier: GPL-2.0-only */
#ifndef GBHOOK_SEH_H
#define GBHOOK_SEH_H
/* SEH guards for a mod's own engine calls. They compile to nothing off MSVC, so mingw can compile-check a mod.
 * A guarded function must stay a leaf with no C++ objects to unwind, or MSVC refuses it (C2712). */

#if defined(_MSC_VER)
#  include <excpt.h>   /* EXCEPTION_EXECUTE_HANDLER, so a file without windows.h still compiles */
#  define GBH_SEH_TRY    __try
#  define GBH_SEH_EXCEPT __except (EXCEPTION_EXECUTE_HANDLER)
#else
#  define GBH_SEH_TRY    if (1)
#  define GBH_SEH_EXCEPT else
#endif

#endif /* GBHOOK_SEH_H */
