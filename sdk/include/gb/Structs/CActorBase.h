// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
#include "Types.h"

namespace CActorBase
{
	struct CActorBase
	{ };

	// this + 0x50: the enabled cookie. isEnabled compares it with enabled; enable only moves it between the two.
	static constexpr uintptr_t cookie = 0x50;
	// Disabled is the spawn pool: a fully built actor switched off, and the one spawnCharacter takes first.
	static constexpr uint32_t cookieEnabled  = 0x78A123;
	static constexpr uint32_t cookieDisabled = 0x6F3874E;
	// this + 0x54 and + 0xA4: the Vector3 pair warpTo writes.
	static constexpr uintptr_t pos    = 0x54;
	static constexpr uintptr_t orient = 0xA4;
	// this + 0x298: the frame that last updated this actor. enable stamps + 0x29C on the way up.
	static constexpr uintptr_t lastFrame = 0x298;

	CActorBase* enable(CActorBase* actor, bool flag);
	CActorBase* warpTo(CActorBase* actor, Vector3 pos, Vector3 orient);
	bool isDead(CActorBase* actor);
}
