#define _CRT_SECURE_NO_WARNINGS

#include "rsp.h"

#include "pj64_globals.h"
#include "hooks.h"
#include "minidump.h"
#include "parallel.h"
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

	static RSPType sRSPType = RSPType::HLE;
	static bool sOk = false;

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
		Parallel::GetDllInfo(&PluginInfo);

		if (!ValidPluginVersion(&PluginInfo) || PluginInfo.MemoryBswaped == FALSE) { return FALSE; }
		PJ64::Globals::RSPVersion() = PluginInfo.Version;
		if (PJ64::Globals::RSPVersion() == 1) { PJ64::Globals::RSPVersion() = 0x0100; }

		PJ64::Globals::DoRspCycles() = (void*)Parallel::DoRspCycles;
		if (PJ64::Globals::DoRspCycles() == NULL) { return FALSE; }
		PJ64::Globals::InitiateRSP_1_0() = NULL;
		PJ64::Globals::InitiateRSP_1_1() = NULL;
		if (PJ64::Globals::RSPVersion() == 0x100) {
			PJ64::Globals::InitiateRSP_1_0() = (void*)Parallel::InitiateRSP;
			if (PJ64::Globals::InitiateRSP_1_0() == NULL) { return FALSE; }
		}
		if (PJ64::Globals::RSPVersion() == 0x101) {
			PJ64::Globals::InitiateRSP_1_1() = (void*)Parallel::InitiateRSP;
			if (PJ64::Globals::InitiateRSP_1_1() == NULL) { return FALSE; }
		}
		PJ64::Globals::RSPRomClosed() = Parallel::RomClosed;
		if (PJ64::Globals::RSPRomClosed() == NULL) { return FALSE; }
		PJ64::Globals::RSPCloseDLL() = (void*)Parallel::CloseDLL;
		if (PJ64::Globals::RSPCloseDLL() == NULL) { return FALSE; }
		PJ64::Globals::GetRspDebugInfo() = (void*)NULL;
		PJ64::Globals::InitiateRSPDebugger() = (void*)NULL;
		PJ64::Globals::RSPDllConfig() = (void*)Parallel::DllConfig;
		return TRUE;
	}

	static void loadDll(RSPType type)
	{
		sRSPType = type;
	}

	enum ZilmarCfg
	{
		HLE_GFX = 1,
	};

	static unsigned int CALL GetSetting(void* handle, int ID)
	{
		return 1;
	}

	Zilmar::PLUGIN_SETTINGS sPluginSettings
	{
		.GetSetting = &GetSetting,
	};

	static unsigned int CALL FindSystemSettingId(void* handle, const char* Name)
	{
		if (0 == strcmp(Name, "HLE GFX"))
			return ZilmarCfg::HLE_GFX;

		return 0;
	}

	Zilmar::PLUGIN_SETTINGS2 sPluginSettings2
	{
		.FindSystemSettingId = &FindSystemSettingId,
	};

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

		Parallel::SetSettingInfo(&sPluginSettings);
		Parallel::SetSettingInfo2(&sPluginSettings2);
		Parallel::PluginLoaded();

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
