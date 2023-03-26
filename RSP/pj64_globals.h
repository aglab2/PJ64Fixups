#pragma once

#include <Windows.h>

namespace PJ64
{
	struct Timer;

	namespace Globals
	{
		[[maybe_unused]] static inline HWND MainWindow()
		{
			return *(HWND*)0x4D7F50;
		}

		[[maybe_unused]] static inline HWND HiddenWin()
		{
			return *(HWND*)0x4D81F8;
		}

		[[maybe_unused]] static inline HWND StatusWin()
		{
			return *(HWND*)0x4D81e0;
		}

		[[maybe_unused]] static inline char* CurrentSave()
		{
			return (char*)0x4D8080;
		}

		[[maybe_unused]] static inline char* RomName()
		{
			return (char*)0x4af1f8;
		}

		typedef void(__cdecl* RomClosedFn) (void);

		[[maybe_unused]] static inline RomClosedFn GfxRomClosed()
		{
			return *(RomClosedFn*)0x004D803C;
		}

		[[maybe_unused]] static inline RomClosedFn GfxRomOpen()
		{
			return *(RomClosedFn*)0x004D81AC;
		}

		[[maybe_unused]] static inline RomClosedFn AiRomClosed()
		{
			return *(RomClosedFn*)0x004D7F84;
		}

		[[maybe_unused]] static inline RomClosedFn ContRomOpen()
		{
			return *(RomClosedFn*)0x004D7FE4;
		}

		[[maybe_unused]] static inline RomClosedFn ContRomClosed()
		{
			return *(RomClosedFn*)0x004D819C;
		}

		[[maybe_unused]] static inline RomClosedFn& RSPRomClosed()
		{
			return *(RomClosedFn*)0x004D7FB8;
		}

		[[maybe_unused]] static inline Timer* FPSTimer()
		{
			return (Timer*)0x0046BD00;
		}

		[[maybe_unused]] static inline HANDLE hCPU()
		{
			return *(HANDLE*) 0x004D75EC;
		}

		[[maybe_unused]] static inline DWORD CPU_Type()
		{
			return *(DWORD*) 0x4D7F70;
		}

		[[maybe_unused]] static inline WORD& RSPVersion()
		{
			return *(WORD*) 0x004D1544;
		}

		using GenericFn = void*;
		[[maybe_unused]] static inline GenericFn& DoRspCycles()
		{
			return *(GenericFn*)0x4D7F90;
		}

		[[maybe_unused]] static inline GenericFn& InitiateRSP_1_0()
		{
			return *(GenericFn*)0x4D7FF0;
		}

		[[maybe_unused]] static inline GenericFn& InitiateRSP_1_1()
		{
			return *(GenericFn*)0x4D7FEC;
		}

		[[maybe_unused]] static inline GenericFn& RSPCloseDLL()
		{
			return *(GenericFn*)0x4D804C;
		}

		[[maybe_unused]] static inline GenericFn& GetRspDebugInfo()
		{
			return *(GenericFn*)0x4D81F4;
		}

		[[maybe_unused]] static inline GenericFn& InitiateRSPDebugger()
		{
			return *(GenericFn*)0x4D7F9C;
		}

		[[maybe_unused]] static inline GenericFn& RSPDllConfig()
		{
			return *(GenericFn*)0x4D81CC;
		}

		[[maybe_unused]] static inline BOOL CPURunning()
		{
			return *(BOOL*)0x004D7610;
		}

		[[maybe_unused]] static inline BOOL InFullScreen()
		{
			return *(BOOL*)0x004D7600;
		}

		[[maybe_unused]] static inline HWND ManageWindow()
		{
			return *(HWND*)0x48E048;
		}

#if 0
		typedef BOOL(__fastcall* R4300i_LW_VAddrFn)(DWORD VAddr, DWORD* Value);
		static inline R4300i_LW_VAddrFn R4300i_LW_VAddr()
		{
			return (R4300i_LW_VAddrFn)0x0042a870;
		}
#endif

		typedef BOOL(__fastcall* MachineFn)(void);
		[[maybe_unused]] static inline MachineFn Machine_LoadState()
		{
			return (MachineFn)0x0041e680;
		}

		[[maybe_unused]] static inline MachineFn Machine_SaveState()
		{
			return (MachineFn)0x0041f510;
		}

		[[maybe_unused]] static inline DWORD* TLB_ReadMap()
		{
			return *(DWORD**) 0x4D6A14;
		}

		typedef void(__cdecl* DacreateChanged)(int);
		[[maybe_unused]] static inline DacreateChanged AiDacreateChanged()
		{
			return *(DacreateChanged*)(0x4D8040);
		}
	}
}
