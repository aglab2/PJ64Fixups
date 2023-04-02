#pragma once

#include "Rsp #1.1.h"

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "Types.h"

typedef RECT rectangle;
typedef PAINTSTRUCT window_paint;

// Profiling
#define Default_ProfilingOn			FALSE
#define Default_IndvidualBlock		FALSE
#define Default_ShowErrors			FALSE
#define Default_AudioHle			FALSE

uint32_t AsciiToHex(char * HexValue);
void DisplayError(char * Message, ...);

#define InterpreterCPU	0
#define RecompilerCPU	1

extern int DebuggingEnabled, Profiling, IndvidualBlock, ShowErrors, BreakOnStart, LogRDP, LogX86Code;
extern uint32_t CPUCore;
extern DEBUG_INFO DebugInfo;
extern RSP_INFO RSPInfo;
extern HINSTANCE hinstDLL;

#if defined(__cplusplus)
}
#endif
