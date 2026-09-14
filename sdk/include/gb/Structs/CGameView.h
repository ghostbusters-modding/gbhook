// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
#include "Types.h"
#include "CActor.h"

namespace CGameView
{
	struct CGameView
	{

	};

	// gMainView + 0x20: the field of view in degrees, 23 in play and 18 while aiming.
	static constexpr uintptr_t fov = 0x20;

	// gMainView + 0x234: the script camera mode, 0 normal, 0xA path, 0xB fixed, 0xD orbit.
	static constexpr uintptr_t mode = 0x234;

	// gMainView + 0x2CC: the follow arm length, 4.5 by default.
	static constexpr uintptr_t dist = 0x2CC;

	void impactCamera(Vector3 dirMag, float hitDuration, float recoveryDuration);
	void shakeCamera(float strength, float duration, float rampUpTime, float rampDownTime, float speed);
	void setCameraModeOrbit(CActor::CActor* followTarget, float radius, float rps, float targetHeightPct, float offsetHeight, float transitionTime, float orbitDuration);
	void resetCamera();
}