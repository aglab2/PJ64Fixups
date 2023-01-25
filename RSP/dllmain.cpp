#include <Windows.h>

#include "hooks.h"
#include "Rsp #1.1.h"

#include <filesystem>

static HMODULE gRSP = nullptr;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Just in case
        gRSP = nullptr;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if (auto rsp = std::exchange(gRSP, nullptr))
        {
            FreeLibrary(rsp);
        }
        break;
    }
    return TRUE;
}

typedef void (CALL *CloseDLLFn)(void);
typedef void (CALL *DllAboutFn)(HWND hParent);
typedef void (CALL *DllConfigFn)(HWND hParent);
typedef void (CALL *DllTestFn)(HWND hParent);
typedef DWORD(CALL *DoRspCyclesFn)(DWORD Cycles);
typedef void (CALL *GetDllInfoFn)(PLUGIN_INFO* PluginInfo);
typedef void (CALL *GetRspDebugInfoFn)(RSPDEBUG_INFO* RSPDebugInfo);
typedef void (CALL *InitiateRSPFn)(RSP_INFO Rsp_Info, DWORD* CycleCount);
typedef void (CALL *InitiateRSPDebuggerFn)(DEBUG_INFO DebugInfo);
typedef void (CALL *RomClosedFn)(void);

#define FN(x) static x##Fn g##x = nullptr;
#include "xmacro.h"
#undef FN

static void loadRSP()
{
    char path[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&loadRSP, &hm) == 0)
    {
        return;
    }
    if (GetModuleFileName(hm, path, sizeof(path)) == 0)
    {
        return;
    }

    bool ok = plantHooks();
    if (ok)
    {
        // Retain ourselves, forever and ever
        LoadLibrary(path);
    }

    // Load original RSP
    std::filesystem::path dllpath{ path };
    auto dir = dllpath.remove_filename();
    {
        auto rsppath = dir / "RSP.dat";
        gRSP = LoadLibraryW(rsppath.c_str());

#define FN(x) g##x = (x##Fn)GetProcAddress(gRSP, #x);
#include "xmacro.h"
#undef FN
    }
}

#define LOAD_RSP() if (!gRSP) { loadRSP(); }

EXPORT void CALL CloseDLL(void) { LOAD_RSP(); gCloseDLL(); }
EXPORT void CALL DllAbout(HWND hParent) { LOAD_RSP(); gDllAbout(hParent); }
EXPORT void CALL DllConfig(HWND hParent) { LOAD_RSP(); gDllConfig(hParent); }
EXPORT void CALL DllTest(HWND hParent) { LOAD_RSP(); gDllTest(hParent); }
EXPORT DWORD CALL DoRspCycles(DWORD Cycles) { LOAD_RSP(); return gDoRspCycles(Cycles); }
EXPORT void CALL GetDllInfo(PLUGIN_INFO* PluginInfo) { LOAD_RSP(); gGetDllInfo(PluginInfo); }
EXPORT void CALL GetRspDebugInfo(RSPDEBUG_INFO* RSPDebugInfo) { LOAD_RSP(); gGetRspDebugInfo(RSPDebugInfo); }
EXPORT void CALL InitiateRSP(RSP_INFO Rsp_Info, DWORD* CycleCount) { LOAD_RSP(); gInitiateRSP(Rsp_Info, CycleCount); }
EXPORT void CALL InitiateRSPDebugger(DEBUG_INFO DebugInfo) { LOAD_RSP(); gInitiateRSPDebugger(DebugInfo); }
EXPORT void CALL RomClosed(void) { LOAD_RSP(); gRomClosed(); }
