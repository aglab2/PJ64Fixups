#pragma once

#include <Windows.h>

class HookManager
{
public:
    HookManager() = default;

    void init();

private:
    void romClosed(bool fromSavestate);

    static void __fastcall hookCloseCpuRomClosed();
    static void __fastcall hookMachine_LoadStateRomReinit();
    static LRESULT WINAPI hookMachine_LoadStateFinished(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static void __fastcall hookStartRecompiledCpuRomOpen();
    static void WINAPI hookCloseCpu(DWORD* ExitCode);
    static BOOL __fastcall RefreshScreen_TimerProcess(DWORD* FrameRate);
    static void __stdcall WinMain_RunLoopHook(LPMSG);
    static BOOL __fastcall hookR4300i_LW_VAddr(DWORD VAddr, DWORD* Value);
    static BOOL __fastcall hookMachine_LoadState(void);
    static BOOL __fastcall hookMachine_SaveState(void);
};
