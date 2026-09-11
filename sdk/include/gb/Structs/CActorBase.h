#pragma once
#include <cstdint>
#include "Types.h"

namespace CActorBase
{
	struct CActorBase
	{ };
	 
	CActorBase* enable(CActorBase* actor, bool flag);
	CActorBase* warpTo(CActorBase* actor, Vector3 pos, Vector3 orient);
	bool isDead(CActorBase* actor);
}
