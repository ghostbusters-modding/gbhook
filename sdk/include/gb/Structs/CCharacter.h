#pragma once
#include <cstdint>
#include "Types.h"

namespace CCharacter
{
	struct CCharacter
	{ };

	void setAnimation(CCharacter* actor, const char* animationName, bool useSkelFileExit);
	float startTalking(CCharacter* actor, const char* dbEntryTag);
	//int beginWalkTo(CCharacter* actor, SScriptWalkInfo* info, bool flushQueue);
}