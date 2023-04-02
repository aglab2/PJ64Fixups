#define _CRT_SECURE_NO_WARNINGS

#include "fixups.h"

#include "pj64_globals.h"
#include "hooks.h"
#include "minidump.h"
#include "Rsp #1.1.h"
#include "tool_ui.h"

namespace Fixups
{
	static BOOL setupRSP()
	{
		PJ64::Globals::RSPVersion() = 0x101;
		PJ64::Globals::DoRspCycles() = (void*) DoRspCycles;
		PJ64::Globals::InitiateRSP_1_0() = NULL;
		PJ64::Globals::InitiateRSP_1_1() = (void*) InitiateRSP;
		PJ64::Globals::RSPRomClosed() = RomClosed;
		PJ64::Globals::RSPCloseDLL() = (void*) CloseDLL;
		PJ64::Globals::GetRspDebugInfo() = (void*) GetRspDebugInfo;
		PJ64::Globals::InitiateRSPDebugger() = (void*) InitiateRSPDebugger;
		PJ64::Globals::RSPDllConfig() = (void*) UI::show;

		return TRUE;
	}

	void load()
	{
		char path[MAX_PATH];
		HMODULE hm = NULL;

		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&load, &hm) == 0)
		{
			return;
		}
		if (GetModuleFileName(hm, path, sizeof(path)) == 0)
		{
			return;
		}

		bool ok = plantHooks();
		if (ok)
		{
			// Retain ourselves, forever and ever
			LoadLibrary(path);
			ok = setupRSP();
		}

		if (ok)
		{
			setupExceptionFilters();
			zapRSPInit();
		}
	}
}
