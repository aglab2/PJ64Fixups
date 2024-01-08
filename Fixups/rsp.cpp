#define _CRT_SECURE_NO_WARNINGS

#include "rsp.h"

#include "pj64_globals.h"
#include "hooks.h"
#include "minidump.h"
#include "resource1.h"
#include "zlib-ng.h"

#include <io.h>
#include <fcntl.h>
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>

extern HINSTANCE hInstance;
namespace RSP
{
	enum class RSPType
	{
		HLE,
		LLE,
	};

	HMODULE gLib = nullptr;
	static RSPType sRSPType = RSPType::HLE;
	static bool sOk = false;
	constexpr int sRSPTypesCount = 2;

#define FN(x) x##Fn g##x = nullptr;
#include "xmacro.h"
#undef FN

	static const char* toRSPName(RSPType type)
	{
		switch (type)
		{
		case RSPType::HLE:
			return "RSP.dat";
		case RSPType::LLE:
			return "LLERSP.dat";
		}
	}

	static const char* getRSPPath(RSPType type)
	{
		static char paths[sRSPTypesCount][MAX_PATH]{ };
		char* path = paths[(int) type];
		if ('\0' == *path)
		{
			SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
			PathAppend(path, "PJ64Fixups");
			CreateDirectory(path, nullptr);
			PathAppend(path, toRSPName(type));
		}
		return path;
	}

	static auto getResource(RSPType type)
	{
		switch (type)
		{
		case RSP::RSPType::HLE:
			return MAKEINTRESOURCE(IDR_RSP);
		case RSP::RSPType::LLE:
			return MAKEINTRESOURCE(IDR_LLERSP);
		}
	}

	static const char* unpack(RSPType type)
	{
		auto path = getRSPPath(type);
		int fd = _open(path, _O_BINARY | _O_WRONLY | _O_CREAT | _O_EXCL, 0777);
		if (-1 != fd)
		{
			auto rc = FindResource(hInstance, getResource(type), RT_RCDATA);
			auto res = LoadResource(hInstance, rc);
            uint8_t *dataComp = (uint8_t*)LockResource(res);
			size_t sizeComp = SizeofResource(hInstance, rc);
			size_t dataLen = 1024 * 1024;
            uint8_t *data   = (uint8_t*) malloc(dataLen);
            zng_uncompress(data, &dataLen, dataComp, sizeComp);
            _write(fd, data, dataLen);
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

		PJ64::Globals::DoRspCycles() = (void*) gDoRspCycles;
		if (PJ64::Globals::DoRspCycles() == NULL) { return FALSE; }
		PJ64::Globals::InitiateRSP_1_0() = NULL;
		PJ64::Globals::InitiateRSP_1_1() = NULL;
		if (PJ64::Globals::RSPVersion() == 0x100) {
			PJ64::Globals::InitiateRSP_1_0() = (void*)gInitiateRSP;
			if (PJ64::Globals::InitiateRSP_1_0() == NULL) { return FALSE; }
		}
		if (PJ64::Globals::RSPVersion() == 0x101) {
			PJ64::Globals::InitiateRSP_1_1() = (void*)gInitiateRSP;
			if (PJ64::Globals::InitiateRSP_1_1() == NULL) { return FALSE; }
		}
		PJ64::Globals::RSPRomClosed() = gRomClosed;
		if (PJ64::Globals::RSPRomClosed() == NULL) { return FALSE; }
		PJ64::Globals::RSPCloseDLL() = (void*)gCloseDLL;
		if (PJ64::Globals::RSPCloseDLL() == NULL) { return FALSE; }
		PJ64::Globals::GetRspDebugInfo() = (void*)gGetRspDebugInfo;
		PJ64::Globals::InitiateRSPDebugger() = (void*)gInitiateRSPDebugger;
		PJ64::Globals::RSPDllConfig() = (void*)DllConfig;

		return TRUE;
	}

	static void loadDll(RSPType type)
	{
		if (gLib)
			FreeLibrary(gLib);

		gLib = LoadLibrary(RSP::unpack(type));
		sRSPType = type;
#define FN(x) g##x = (x##Fn)GetProcAddress(gLib, #x);
#include "xmacro.h"
#undef FN
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

		// Load original HLE RSP
		loadDll(RSPType::HLE);

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
			sOk = true;
		}
	}

#define fisupper(a) (((unsigned)(a)-'A') < 26)

	static int ftolower(int c)
	{
		if (fisupper(c)) return c | 32;
		return c;
	}

	void reload()
	{
		if (!sOk)
			return;

		auto dll = PJ64::Globals::GfxDll();
		if (!dll)
			return;

		auto getPluginInfo = (void(__cdecl*)(PLUGIN_INFO*))GetProcAddress(dll, "GetDllInfo");
		if (!getPluginInfo)
			return;

		PLUGIN_INFO info;
		getPluginInfo(&info);
		for (auto& ch : info.Name)
		{
			ch = ftolower(ch);
		}

		RSPType wantType;
		if (strstr(info.Name, "parallel"))
		{
			wantType = RSPType::LLE;
		}
		else
		{
			wantType = RSPType::HLE;
		}

		if (wantType != sRSPType)
		{
			loadDll(wantType);
			setupRSP();
		}

		return;
	}
}
