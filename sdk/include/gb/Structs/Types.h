#pragma once
#include <cstdint>

namespace CActor {
	struct CActor;
}

//----------------GB----------------

struct Vector3
{
	float x;
	float y;
	float z;
};

//---------------PDB----------------

struct String
{
	char* ptr;
};


struct FastString : String
{
};

struct HSound
{
	int handle;
};

struct CDialogDatabaseEntry
{
	FastString tag;
	FastString text;
	HSound hSound;
};

struct SScriptWalkInfo
{
	Vector3 wDest;
	Vector3 orient;
	CActor::CActor* destActor;
	float atDestTol;
	float runDistTol;
	float facingTolInRadians;
	int eventAddress;
	int cancelScriptControl;
};

//---------------PDB-END------------