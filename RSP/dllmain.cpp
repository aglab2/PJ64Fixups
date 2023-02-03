#include <Windows.h>

#include "rsp.h"
#include "tool_ui.h"

#include <utility>

extern HINSTANCE hInstance;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Just in case
        hInstance = hModule;
        RSP::gLib = nullptr;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if (auto rsp = std::exchange(RSP::gLib, nullptr))
        {
            FreeLibrary(rsp);
        }
        break;
    }
    return TRUE;
}

#define LOAD_RSP() if (!RSP::gLib) { RSP::load(); }

// These functions are really only necessary for a fallback, usually only 'GetDllInfo' is called
EXPORT void CALL CloseDLL(void) { LOAD_RSP(); RSP::gCloseDLL(); }
EXPORT void CALL DllAbout(HWND hParent) { LOAD_RSP(); RSP::gDllAbout(hParent); }
EXPORT void CALL DllConfig(HWND hParent) { LOAD_RSP(); UI::show(hParent); /* gDllConfig(hParent); */ }
EXPORT void CALL DllTest(HWND hParent) { LOAD_RSP(); RSP::gDllTest(hParent); }
EXPORT DWORD CALL DoRspCycles(DWORD Cycles) { LOAD_RSP(); return RSP::gDoRspCycles(Cycles); }
EXPORT void CALL GetDllInfo(PLUGIN_INFO* PluginInfo) { LOAD_RSP(); RSP::gGetDllInfo(PluginInfo); }
EXPORT void CALL GetRspDebugInfo(RSPDEBUG_INFO* RSPDebugInfo) { LOAD_RSP(); RSP::gGetRspDebugInfo(RSPDebugInfo); }
EXPORT void CALL InitiateRSP(RSP_INFO Rsp_Info, DWORD* CycleCount) { LOAD_RSP(); RSP::gInitiateRSP(Rsp_Info, CycleCount); }
EXPORT void CALL InitiateRSPDebugger(DEBUG_INFO DebugInfo) { LOAD_RSP(); RSP::gInitiateRSPDebugger(DebugInfo); }
EXPORT void CALL RomClosed(void) { LOAD_RSP(); RSP::gRomClosed(); }
