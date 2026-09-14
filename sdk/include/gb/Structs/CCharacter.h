// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
#include "Types.h"

namespace CCharacter
{
	struct CCharacter
	{ };

	// this + 0xB870: the u32 setInvulnerableFlag stores, which god mode reads back.
	static constexpr uintptr_t invulnerable = 0xB870;

	void setAnimation(CCharacter* actor, const char* animationName, bool useSkelFileExit);
	float startTalking(CCharacter* actor, const char* dbEntryTag);
	//int beginWalkTo(CCharacter* actor, SScriptWalkInfo* info, bool flushQueue);
}