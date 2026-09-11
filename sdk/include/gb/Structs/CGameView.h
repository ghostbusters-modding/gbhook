#pragma once
#include <cstdint>
#include "Types.h"
#include "CActor.h"

namespace CGameView
{
	struct CGameView
	{

	};

	void impactCamera(Vector3 dirMag, float hitDuration, float recoveryDuration);
	void shakeCamera(float strength, float duration, float rampUpTime, float rampDownTime, float speed);
	void setCameraModeOrbit(CActor::CActor* followTarget, float radius, float rps, float targetHeightPct, float offsetHeight, float transitionTime, float orbitDuration);
	void resetCamera();
}