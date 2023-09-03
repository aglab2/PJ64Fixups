#pragma once

#include <windows.h>
#include "Rsp #1.1.h"

namespace RSP
{
	extern HMODULE gLib;

	typedef void (CALL* CloseDLLFn)(void);
	typedef void (CALL* DllAboutFn)(HWND hParent);
	typedef void (CALL* DllConfigFn)(HWND hParent);
	typedef void (CALL* DllTestFn)(HWND hParent);
	typedef DWORD (CALL* DoRspCyclesFn)(DWORD Cycles);
	typedef void (CALL* GetDllInfoFn)(PLUGIN_INFO* PluginInfo);
	typedef void (CALL* GetRspDebugInfoFn)(RSPDEBUG_INFO* RSPDebugInfo);
	typedef void (CALL* InitiateRSPFn)(RSP_INFO Rsp_Info, DWORD* CycleCount);
	typedef void (CALL* InitiateRSPDebuggerFn)(DEBUG_INFO DebugInfo);
	typedef void (CALL* RomClosedFn)(void);

#define FN(x) extern x##Fn g##x;
#include "xmacro.h"
#undef FN

	void load();
}
