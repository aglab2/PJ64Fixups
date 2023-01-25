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
    static void hookStartRecompiledCpuRomOpen();
    static void __stdcall hookCloseCpu(DWORD* ExitCode);
    static LRESULT WINAPI hookSendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static LRESULT WINAPI hookSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static HANDLE WINAPI hookCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
};
