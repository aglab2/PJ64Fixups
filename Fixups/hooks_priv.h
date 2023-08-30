#pragma once

#include <Windows.h>
#include <minwindef.h>

class HookManager
{
public:
    HookManager() = default;

    void init();

private:
    void romClosed(bool fromSavestate);

	typedef union tagUDWORD {
		double				D;
		_int64				DW;
		unsigned _int64		UDW;
		long				W[2];
		float				F[2];
		unsigned long		UW[2];
		short				HW[4];
		unsigned short		UHW[4];
		char				B[8];
		unsigned char		UB[8];
	} MIPS_DWORD;

	typedef struct {
		//r4k
		int		    MIPS_RegState[32];
		MIPS_DWORD	MIPS_RegVal[32];

		DWORD		x86reg_MappedTo[10];
		DWORD		x86reg_MapOrder[10];
		BOOL		x86reg_Protected[10];

		DWORD		CycleCount;
		DWORD		RandomModifier;

		//FPU
		DWORD		Stack_TopPos;
		DWORD		x86fpu_MappedTo[8];
		DWORD		x86fpu_State[8];
		DWORD		x86fpu_RoundingModel[8];

		BOOL        Fpu_Used;
		DWORD       RoundingModel;
	} REG_INFO;

	typedef struct {
		DWORD		TargetPC;
		char* BranchLabel;
		BYTE* LinkLocation;
		BYTE* LinkLocation2;
		BOOL		FallThrough;
		BOOL		PermLoop;
		BOOL		DoneDelaySlot;
		REG_INFO	RegSet;
	} JUMP_INFO;

	typedef struct {
		/* Block Connection info */
		void** ParentSection;
		void* ContinueSection;
		void* JumpSection;
		BYTE* CompiledLocation;


		DWORD		SectionID;
		DWORD		Test;
		DWORD		Test2;
		BOOL		InLoop;

		DWORD		StartPC;
		DWORD		CompilePC;

		/* Register Info */
		REG_INFO	RegStart;
		REG_INFO	RegWorking;

		/* Jump Info */
		JUMP_INFO   Jump;
		JUMP_INFO   Cont;
	} BLOCK_SECTION;

	// These functions do not really calls but they call __cdecl symbols
    static void __cdecl hookCloseCpuRomClosed();
    static void __cdecl hookMachine_LoadStateRomReinit();
	static void __cdecl hookStartRecompiledCpuRomOpen();
	static void __cdecl hookAiDacrateChanged(int systemType);

    static LRESULT WINAPI hookMachine_LoadStateFinished(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    static void WINAPI hookCloseCpu(DWORD* ExitCode);
    static BOOL __fastcall RefreshScreen_TimerProcess(DWORD* FrameRate);
    static void __stdcall WinMain_RunLoopHook(LPMSG);
    static BOOL __fastcall hookR4300i_LW_VAddr(DWORD VAddr, DWORD* Value);
    static BOOL __fastcall hookR4300i_LW_VAddrSection(BLOCK_SECTION* section, DWORD* Value);

    static BOOL __fastcall hookMachine_LoadState(void);
    static BOOL __fastcall hookMachine_SaveState(void);

	static void __fastcall hookTimerInitialize(double Hertz);
	static void __fastcall hookTimerStart(void);
	static void __fastcall hookTimerStop (void);

	static void __cdecl hookGetKeys(int, DWORD*);
};
