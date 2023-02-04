#pragma once

#include <Windows.h>

class HookManager
{
public:
    HookManager() = default;

    void init();

private:
    void romClosed(bool fromSavestate);

    static void hookCloseCpuRomClosed();
    static void hookMachine_LoadStateRomReinit();
    static LRESULT WINAPI hookMachine_LoadStateFinished(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static void hookStartRecompiledCpuRomOpen();
    static void __stdcall hookCloseCpu(DWORD* ExitCode);
    static BOOL __cdecl RefreshScreen_TimerProcess(DWORD* FrameRate);
    static void __stdcall WinMain_RunLoopHook(LPMSG);
};
