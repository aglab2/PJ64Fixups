#pragma once

#include <Windows.h>

namespace PJ64
{
	struct Timer;

	namespace Globals
	{
		static inline HWND MainWindow()
		{
			return *(HWND*)0x4D7F50;
		}

		static inline HWND HiddenWin()
		{
			return *(HWND*)0x4D81F8;
		}

		static inline HWND StatusWin()
		{
			return *(HWND*)0x4D81e0;
		}

		static inline char* CurrentSave()
		{
			return (char*)0x4D8080;
		}

		static inline char* RomName()
		{
			return (char*)0x4af1f8;
		}

		typedef void(__cdecl* RomClosedFn) (void);

		static inline RomClosedFn GfxRomClosed()
		{
			return *(RomClosedFn*)0x004D803C;
		}

		static inline RomClosedFn GfxRomOpen()
		{
			return *(RomClosedFn*)0x004D81AC;
		}

		static inline RomClosedFn AiRomClosed()
		{
			return *(RomClosedFn*)0x004D7F84;
		}

		static inline RomClosedFn ContRomOpen()
		{
			return *(RomClosedFn*)0x004D7FE4;
		}

		static inline RomClosedFn ContRomClosed()
		{
			return *(RomClosedFn*)0x004D819C;
		}

		static inline RomClosedFn& RSPRomClosed()
		{
			return *(RomClosedFn*)0x004D7FB8;
		}

		static inline Timer* FPSTimer()
		{
			return (Timer*)0x0046BD00;
		}

		static inline HANDLE hCPU()
		{
			return *(HANDLE*) 0x004D75EC;
		}

		static inline DWORD CPU_Type()
		{
			return *(DWORD*) 0x4D7F70;
		}

		static inline WORD& RSPVersion()
		{
			return *(WORD*) 0x004D1544;
		}

		using GenericFn = void*;
		static inline GenericFn& DoRspCycles()
		{
			return *(GenericFn*)0x4D7F90;
		}

		static inline GenericFn& InitiateRSP_1_0()
		{
			return *(GenericFn*)0x4D7FF0;
		}

		static inline GenericFn& InitiateRSP_1_1()
		{
			return *(GenericFn*)0x4D7FEC;
		}

		static inline GenericFn& RSPCloseDLL()
		{
			return *(GenericFn*)0x4D804C;
		}

		static inline GenericFn& GetRspDebugInfo()
		{
			return *(GenericFn*)0x4D81F4;
		}

		static inline GenericFn& InitiateRSPDebugger()
		{
			return *(GenericFn*)0x4D7F9C;
		}

		static inline GenericFn& RSPDllConfig()
		{
			return *(GenericFn*)0x4D81CC;
		}

		static inline BOOL CPURunning()
		{
			return *(BOOL*)0x004D7610;
		}

		static inline BOOL InFullScreen()
		{
			return *(BOOL*)0x004D7600;
		}

		static inline HWND ManageWindow()
		{
			return *(HWND*)0x48E048;
		}

		typedef BOOL(__fastcall* R4300i_LW_VAddrFn)(DWORD VAddr, DWORD* Value);
		constexpr static inline R4300i_LW_VAddrFn R4300i_LW_VAddr()
		{
			return (R4300i_LW_VAddrFn)0x0042a870;
		}
	}
}
