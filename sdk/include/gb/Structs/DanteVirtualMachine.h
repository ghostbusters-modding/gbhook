#pragma once
#include <cstdint>
#include "Types.h"

namespace Dante
{
	struct DanteProgram
	{
		char name[64];
		char parentProgramName[64];
		int programIndex;
		int vmCodeAddress;
		int vmCodeSize;
		int vmDataAddress;
		int vmDataSize;
		int stringLiteralCount;
		//CDialogDatabase* dialogDatabase;
		//DanteDestructorStackEntry destructorStackList[256];
		int destructorStackSize;
	};

	void displayMessage(int EHudMessage, const char* text, float duration);
	const CDialogDatabaseEntry* findDialogEntryByTag(const char* tag);
}

extern bool DEBUG_MODE;

using ExportGlobalVariableType = void(*)(char*, __int64, __int64, __int64);
extern ExportGlobalVariableType exportGlobalVariable;
void __stdcall HookedFunction(char* buffer, __int64 adr1, __int64 adr2, __int64 adr3);

namespace Logger {
	void logHookCall(char* buffer, __int64 adr1, __int64 adr2, __int64 adr3);
}

namespace HookManager {
	bool initialize();
	bool createAndEnableHook(void* target);
	void cleanup(void* target);
}