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

		static inline RomClosedFn RSPRomClosed()
		{
			return *(RomClosedFn*)0x004D7FB8;
		}

		static Timer* FPSTimer()
		{
			return (Timer*)0x0046BD00;
		}

		static HANDLE hCPU()
		{
			return *(HANDLE*) 0x004D75EC;
		}

		static DWORD CPU_Type()
		{
			return *(DWORD*) 0x4D7F70;
		}
	}
}
