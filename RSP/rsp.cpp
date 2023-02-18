#define _CRT_SECURE_NO_WARNINGS

#include "rsp.h"

#include "pj64_globals.h"
#include "minidump.h"
#include "hooks.h"
#include "resource1.h"

#include <io.h>
#include <fcntl.h>
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>

extern HINSTANCE hInstance;
namespace RSP
{
	HMODULE gLib = nullptr;

#define FN(x) x##Fn g##x = nullptr;
#include "xmacro.h"
#undef FN

	static const char* getRSPPath()
	{
		static char path[MAX_PATH]{ 0 };
		if ('\0' == *path)
		{
			SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
			PathAppend(path, "PJ64Fixups");
			CreateDirectory(path, nullptr);
			PathAppend(path, "RSP.dll");
		}
		return path;
	}

	static const char* unpack()
	{
		auto path = getRSPPath();
		int fd = _open(path, _O_BINARY | _O_WRONLY | _O_CREAT | _O_EXCL, 0777);
		if (-1 != fd)
		{
			auto rc = FindResource(hInstance, MAKEINTRESOURCE(IDR_RSP), RT_RCDATA);
			auto res = LoadResource(hInstance, rc);
			void* data = (wchar_t*)LockResource(res);
			size_t size = SizeofResource(hInstance, rc);
			_write(fd, data, size);
			_close(fd);
		}

		return path;
	}

	static inline BOOL ValidPluginVersion(PLUGIN_INFO* PluginInfo)
	{
		switch (PluginInfo->Type) {
		case PLUGIN_TYPE_RSP:
			if (PluginInfo->Version == 0x0001) { return TRUE; }
			if (PluginInfo->Version == 0x0100) { return TRUE; }
			if (PluginInfo->Version == 0x0101) { return TRUE; }
			break;
		case PLUGIN_TYPE_GFX:
			if (PluginInfo->Version == 0x0102) { return TRUE; }
			if (PluginInfo->Version == 0x0103) { return TRUE; }
			break;
		case PLUGIN_TYPE_AUDIO:
			if (PluginInfo->Version == 0x0101) { return TRUE; }
			break;
		case PLUGIN_TYPE_CONTROLLER:
			if (PluginInfo->Version == 0x0100) { return TRUE; }
			break;
		}
		return FALSE;
	}

	static BOOL setupRSP()
	{
		PLUGIN_INFO PluginInfo;
		gGetDllInfo(&PluginInfo);

		if (!ValidPluginVersion(&PluginInfo) || PluginInfo.MemoryBswaped == FALSE) { return FALSE; }
		PJ64::Globals::RSPVersion() = PluginInfo.Version;
		if (PJ64::Globals::RSPVersion() == 1) { PJ64::Globals::RSPVersion() = 0x0100; }

		PJ64::Globals::DoRspCycles() = gDoRspCycles;
		if (PJ64::Globals::DoRspCycles() == NULL) { return FALSE; }
		PJ64::Globals::InitiateRSP_1_0() = NULL;
		PJ64::Globals::InitiateRSP_1_1() = NULL;
		if (PJ64::Globals::RSPVersion() == 0x100) {
			PJ64::Globals::InitiateRSP_1_0() = gInitiateRSP;
			if (PJ64::Globals::InitiateRSP_1_0() == NULL) { return FALSE; }
		}
		if (PJ64::Globals::RSPVersion() == 0x101) {
			PJ64::Globals::InitiateRSP_1_1() = gInitiateRSP;
			if (PJ64::Globals::InitiateRSP_1_1() == NULL) { return FALSE; }
		}
		PJ64::Globals::RSPRomClosed() = gRomClosed;
		if (PJ64::Globals::RSPRomClosed() == NULL) { return FALSE; }
		PJ64::Globals::RSPCloseDLL() = gCloseDLL;
		if (PJ64::Globals::RSPCloseDLL() == NULL) { return FALSE; }
		PJ64::Globals::GetRspDebugInfo() = gGetRspDebugInfo;
		PJ64::Globals::InitiateRSPDebugger() = gInitiateRSPDebugger;
		PJ64::Globals::RSPDllConfig() = DllConfig;

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

		// Load original RSP
		gLib = LoadLibrary(RSP::unpack());
#define FN(x) g##x = (x##Fn)GetProcAddress(gLib, #x);
#include "xmacro.h"
#undef FN

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
