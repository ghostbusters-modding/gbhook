// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// SEH guards that compile to nothing off MSVC, so mingw can compile-check the Windows half.
// A guarded function must stay a leaf with no C++ objects to unwind, or MSVC refuses it (C2712).

#if defined(_MSC_VER)
#  define GBH_SEH_TRY    __try
#  define GBH_SEH_EXCEPT __except (EXCEPTION_EXECUTE_HANDLER)
#else
#  define GBH_SEH_TRY    if (true)
#  define GBH_SEH_EXCEPT else
#endif
