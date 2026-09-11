#pragma once
#include <cstdint>
#include "Types.h"

namespace CActor
{
	struct CActor
	{ };

	CActor* createActor(const char* actor, Vector3 wPos);
}
