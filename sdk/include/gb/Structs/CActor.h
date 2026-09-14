// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
#include "Types.h"

namespace CActor
{
	struct CActor
	{ };

	// this + 0x28: the name, NUL-terminated in place; the cookie at + 0x50 bounds it.
	static constexpr uintptr_t name = 0x28;
	// this + 0x288: the next actor in gGame's chain, null at the tail.
	static constexpr uintptr_t next = 0x288;
	// this + 0x2E4: the team.
	static constexpr uintptr_t team = 0x2E4;

	CActor* createActor(const char* actor, Vector3 wPos);
}
