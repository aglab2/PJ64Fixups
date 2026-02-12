#include <Windows.h>

#include "rsp.h"
#include "tool_ui.h"
#include "parallel.h"

#include <utility>

extern HINSTANCE hInstance;
static bool sLoaded = false;

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
        sLoaded = false;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#define LOAD_RSP() if (!sLoaded) { RSP::load(); sLoaded = true; }

// These functions are really only necessary for a fallback, usually only 'GetDllInfo' is called
EXPORT void CALL CloseDLL(void) { LOAD_RSP(); Parallel::CloseDLL(); }
EXPORT void CALL DllAbout(HWND hParent) { LOAD_RSP(); }
EXPORT void CALL DllConfig(HWND hParent) { LOAD_RSP(); UI::show(hParent); }
EXPORT void CALL DllTest(HWND hParent) { LOAD_RSP(); }
EXPORT DWORD CALL DoRspCycles(DWORD Cycles) { LOAD_RSP(); return Parallel::DoRspCycles(Cycles); }
EXPORT void CALL GetDllInfo(PLUGIN_INFO* PluginInfo) { LOAD_RSP(); RSP::reload(); Parallel::GetDllInfo(PluginInfo); }
EXPORT void CALL GetRspDebugInfo(RSPDEBUG_INFO* RSPDebugInfo) { LOAD_RSP(); }
EXPORT void CALL InitiateRSP(RSP_INFO Rsp_Info, unsigned int* CycleCount) { LOAD_RSP(); Parallel::InitiateRSP(Rsp_Info, CycleCount); }
EXPORT void CALL InitiateRSPDebugger(DEBUG_INFO DebugInfo) { LOAD_RSP(); }
EXPORT void CALL RomClosed(void) { LOAD_RSP(); Parallel::RomClosed(); }
