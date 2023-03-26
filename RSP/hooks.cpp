#include "hooks.h"
#include "hooks_priv.h"

#include "backtrace.h"
#include "config.h"
#include "minizip.h"
#include "opcode.h"
#include "pj64_globals.h"
#include "timer.h"
#include "reset_recognizer.h"
#include "unzdispatch.h"

#include <assert.h>
#include <stdint.h>

#include <Commctrl.h>

#include <mutex>
#include <thread>

constexpr char NAME[] = "LINK's Project64";

static HookManager gHookManager;

class UnprotectedMemoryScope
{
public:
    UnprotectedMemoryScope(void* address, size_t size)
        : address_(address)
        , size_(size)
    {
        BOOL result = VirtualProtect(address_, size_, PAGE_EXECUTE_READWRITE, &oldFlags_);
        if (FALSE == result)
            abort();
    }
    ~UnprotectedMemoryScope()
    {
        DWORD origFlags_;
        VirtualProtect(address_, size_, oldFlags_, &origFlags_);
        assert(origFlags_, PAGE_EXECUTE_READWRITE);
    }

    UnprotectedMemoryScope(const UnprotectedMemoryScope&) = delete;
    UnprotectedMemoryScope& operator=(const UnprotectedMemoryScope&) = delete;

private:
    void* address_;
    size_t size_;
    DWORD oldFlags_;
};

static uint8_t nops[] =
{ 0x90,
  0x66, 0x90,
  0x0F, 0x1F, 0x00,
  0x0F, 0x1F, 0x40, 0x00,
  0x0F, 0x1F, 0x44, 0x00, 0x00,
  0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00,
  0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
};
#define MAX_NOP_SIZE 9 

static void fillNop(uint8_t* address, size_t size)
{
#if 1
    while (size)
    {
        size_t n = size > MAX_NOP_SIZE ? MAX_NOP_SIZE : size;
        uint8_t* nopi = &nops[n * (n - 1) / 2];
        memcpy(address, nopi, n);
        size -= n;
        address += n;
    }
#else
    memset(address, 0x90, size);
#endif
}

static void doWriteCall(uintptr_t codeStart, size_t sz, void* fn)
{
    uint8_t* hook = (uint8_t*)codeStart;
    *hook = 0xE8;
    *(uintptr_t*)(hook + 1) = (uintptr_t)(fn)-(uintptr_t)(hook + 5);

    fillNop(hook + 5, sz - 5);
}

static void doWriteJump(uintptr_t codeStart, size_t sz, void* fn)
{
    uint8_t* hook = (uint8_t*)codeStart;
    *hook = 0xE9;
    *(uintptr_t*)(hook + 1) = (uintptr_t)(fn)-(uintptr_t)(hook + 5);

    fillNop(hook + 5, sz - 5);
}

static void writeCall(uintptr_t codeStart, size_t sz, void* fn)
{
    UnprotectedMemoryScope mem{ (void*)codeStart, sz };
    doWriteCall(codeStart, sz, fn);
}

static void writeJump(uintptr_t codeStart, size_t sz, void* fn)
{
    UnprotectedMemoryScope mem{ (void*)codeStart, sz };
    doWriteJump(codeStart, sz, fn);
}

void HookManager::init()
{
    /*
    CloseCpu
    0041DD35  mov         eax,dword ptr ds:[004D803Ch]
    0041DD3A  cmp         eax,edi
    0041DD3C  je          0041DD40
    0041DD3E  call        eax
    0041DD40  mov         eax,dword ptr ds:[004D7F84h]
    0041DD45  cmp         eax,edi
    0041DD47  je          0041DD4B
    0041DD49  call        eax
    0041DD4B  mov         eax,dword ptr ds:[004D819Ch]
    0041DD50  cmp         eax,edi
    0041DD52  je          0041DD56
    0041DD54  call        eax
    0041DD56  mov         eax,dword ptr ds:[004D7FB8h]
    0041DD5B  cmp         eax,edi
    0041DD5D  je          0041DD61
    0041DD5F  call        eax
    >>>
    0041DD61  cmp         dword ptr ds:[4D81A4h],edi
    */
    if (Config::get().fastResets)
    {
        writeCall(0x0041DD35, 0x0041DD5F + 2 - 0x0041DD35, &hookCloseCpuRomClosed);
    }

    /*
    Machine_LoadState
    0041F2FE  mov         eax,dword ptr ds:[004D803Ch]
    0041F303  cmp         eax,ebx
    0041F305  je          0041F309
    0041F307  call        eax
    0041F309  mov         eax,dword ptr ds:[004D7F84h]
    0041F30E  cmp         eax,ebx
    0041F310  je          0041F314
    0041F312  call        eax
    0041F314  mov         eax,dword ptr ds:[004D819Ch]
    0041F319  cmp         eax,ebx
    0041F31B  je          0041F31F
    0041F31D  call        eax
    0041F31F  mov         eax,dword ptr ds:[004D7FB8h]
    0041F324  cmp         eax,ebx
    0041F326  je          0041F32A
    0041F328  call        eax
    0041F32A  mov         eax,dword ptr ds:[004D81ACh]
    0041F32F  cmp         eax,ebx
    0041F331  je          0041F335
    0041F333  call        eax
    0041F335  mov         eax,dword ptr ds:[004D7FE4h]
    0041F33A  cmp         eax,ebx
    0041F33C  je          0041F340
    0041F33E  call        eax
    */
    if (Config::get().fastStates)
    {
        writeCall(0x0041F2FE, 0x0041F33E - 0x0041F2FE + 2, &hookMachine_LoadStateRomReinit);
    }

    /*
    0041e2b6 ffd0            call    eax
    0041e2b8 a114764d00      mov     eax,dword ptr [Project64+0xd7614 (004d7614)]
    0041e2bd 6aff            push    0FFFFFFFFh
    0041e2bf 50              push    eax
    0041e2c0 ffd5            call    ebp
    0041e2c2 393de4754d00    cmp     dword ptr [Project64+0xd75e4 (004d75e4)],edi
    0041e2c8 741d            je      Project64+0x1e2e7 (0041e2e7)
    0041e2ca 8b0d14764d00    mov     ecx,dword ptr [Project64+0xd7614 (004d7614)]
    0041e2d0 51              push    ecx
    0041e2d1 ffd6            call    esi
    0041e2d3 8b15ec754d00    mov     edx,dword ptr [Project64+0xd75ec (004d75ec)]
    0041e2d9 52              push    edx
    0041e2da ff15b0704600    call    dword ptr [Project64+0x670b0 (004670b0)]
    0041e2e0 be01000000      mov     esi,1
    0041e2e5 eb1c            jmp     Project64+0x1e303 (0041e303)
    0041e2e7 a114764d00      mov     eax,dword ptr [Project64+0xd7614 (004d7614)]
    0041e2ec 50              push    eax
    0041e2ed ffd6            call    esi
    0041e2ef be01000000      mov     esi,1
    0041e2f4 eb0d            jmp     Project64+0x1e303 (0041e303)
    0041e2f6 8b0d14764d00    mov     ecx,dword ptr [Project64+0xd7614 (004d7614)]
    0041e2fc 51              push    ecx
    0041e2fd ff15b4704600    call    dword ptr [Project64+0x670b4 (004670b4)]
    0041e303 393d98734d00    cmp     dword ptr [Project64+0xd7398 (004d7398)],edi
    0041e309 5d              pop     ebp
    0041e30a 893d84734d00    mov     dword ptr [Project64+0xd7384 (004d7384)],edi
    0041e310 5b              pop     ebx
    0041e311 741b            je      Project64+0x1e32e (0041e32e)
    0041e313 893d98734d00    mov     dword ptr [Project64+0xd7398 (004d7398)],edi
    >>> 0041e319 e8f2110000      call    Project64+0x1f510 (0041f510) Machine_SaveState
    0041e31e 85c0            test    eax,eax
    0041e320 750c            jne     Project64+0x1e32e (0041e32e)
    0041e322 893598734d00    mov     dword ptr [Project64+0xd7398 (004d7398)],esi
    0041e328 893584734d00    mov     dword ptr [Project64+0xd7384 (004d7384)],esi
    0041e32e 393d9c734d00    cmp     dword ptr [Project64+0xd739c (004d739c)],edi
    0041e334 740b            je      Project64+0x1e341 (0041e341)
    0041e336 893d9c734d00    mov     dword ptr [Project64+0xd739c (004d739c)],edi
    >>> 0041e33c e83f030000      call    Project64+0x1e680 (0041e680) Machine_LoadState
    0041e341 3935a0734d00    cmp     dword ptr [Project64+0xd73a0 (004d73a0)],esi ds:002b:004d73a0=00000000
    0041e347 7506            jne     Project64+0x1e34f (0041e34f)
    0041e349 893584734d00    mov     dword ptr [Project64+0xd7384 (004d7384)],esi
    0041e34f 5f              pop     edi
    0041e350 5e              pop     esi
    */
    if (Config::get().fixLoadStateStutter)
    {
        writeCall(0x0041ff92, 5, &RefreshScreen_TimerProcess);
        writeCall(0x0041F4F3, 6, &hookMachine_LoadStateFinished);
        // writeCall(0x0041e319, 5, &hookMachine_SaveState); - it is not needed
        writeCall(0x0041e33c, 5, &hookMachine_LoadState);
    }
    
    if (Config::get().noLoadSlowdown)
    {
    /*
    OpenChosenFile
    00449ED6  jl          00449EC9  
    00449ED8  mov         ecx,dword ptr ds:[4D7F50h]  
    00449EDE  push        ecx  
    00449EDF  call        dword ptr ds:[46720Ch]  
    00449EE5  push        3E8h  
    // Useless Sleep(1000)
    > 00449EEA  call        dword ptr ds:[46719Ch]  
    00449EF0  call        0041DC00  
    */
        fillNop((uint8_t*)0x00449EE5, 0x00449EF0 - 0x00449EE5);
    /*
    0044A7E6  push        ecx  
    0044A7E7  call        dword ptr ds:[467300h]  
    // Useless Sleep(100)
    0044A7ED  push        64h  
    0044A7EF  call        dword ptr ds:[46719Ch] 
    0044A7F5  mov         eax,dword ptr ds:[004D526Ch] 
    */
        fillNop((uint8_t*)0x0044A7ED, 0x0044A7F5 - 0x0044A7ED);
    /*
    0044A31B  push        0  
    0044A31D  push        401h  
    0044A322  push        edx  
    0044A323  call        ebp  
    0044A325  push        64h  
    // Useless Sleep(100)
    0044A327  call        dword ptr ds:[46719Ch]  
    0044A32D  mov         eax,dword ptr ds:[004D526Ch] 
    */
        fillNop((uint8_t*)0x0044A325, 0x0044A32D - 0x0044A325);
    }

    /*
    0041DC00  mov         eax,dword ptr ds:[004D7610h]
    0041DC05  sub         esp,8
    0041DC08  push        esi
    0041DC09  xor         esi,esi
    0041DC0B  cmp         eax,esi
    0041DC0D  je          0041DD9B
    0041DC13  cmp         dword ptr ds:[4D75E4h],esi
    0041DC19  mov         dword ptr ds:[4D74C0h],esi
    0041DC1F  je          0041DC31
    0041DC21  call        0041FD00
    0041DC26  push        3E8h
    > Sleep
    0041DC2B  call        dword ptr ds:[46719Ch]
    0041DC31  mov         ecx,dword ptr ds:[4D7F50h]
    0041DC37  push        ebx
    0041DC38  push        ebp
    0041DC39  push        edi
    0041DC3A  mov         edi,dword ptr ds:[4D7FC4h]
    0041DC40  mov         dword ptr ds:[4D7FC4h],esi
    0041DC46  call        0040B460
    0041DC4B  mov         ebx,dword ptr ds:[4670A8h]
    0041DC51  mov         dword ptr ds:[4D7FC4h],edi
    0041DC57  mov         edi,dword ptr ds:[4670A4h]
    0041DC5D  mov         ebp,1
    0041DC62  mov         eax,dword ptr ds:[004D7380h]
    0041DC67  push        eax
    0041DC68  mov         dword ptr ds:[4D7388h],ebp
    0041DC6E  mov         dword ptr ds:[4D73A4h],0
    0041DC78  mov         dword ptr ds:[4D7384h],ebp
    0041DC7E  call        edi
    0041DC80  push        64h
    > Sleep
    0041DC82  call        dword ptr ds:[46719Ch]
    0041DC88  mov         edx,dword ptr ds:[4D75ECh]
    0041DC8E  lea         ecx,[esp+10h]
    0041DC92  push        ecx
    0041DC93  push        edx
    0041DC94  call        ebx
    0041DC96  cmp         dword ptr [esp+10h],103h
    0041DC9E  je          0041DCAF
    0041DCA0  mov         dword ptr ds:[4D75ECh],0
    0041DCAA  mov         esi,64h
    0041DCAF  inc         esi
    0041DCB0  cmp         esi,14h
    0041DCB3  jb          0041DC62
    0041DCB5  mov         eax,dword ptr ds:[004D75ECh]
    0041DCBA  xor         edi,edi
    0041DCBC  cmp         eax,edi
    0041DCBE  je          0041DCCE
    0041DCC0  push        edi
    0041DCC1  push        eax
    > TerminateThread
    0041DCC2  call        dword ptr ds:[4670A0h]
    0041DCC8  mov         dword ptr ds:[4D75ECh],edi
    0041DCCE  mov         ecx,dword ptr ds:[4D6A24h]
    */
    // Remove dumb sleep
    if (Config::get().fixSlowResets)
    {
        UnprotectedMemoryScope scope{ (void*)0x0041DC80, 0x0041DC96 - 0x0041DC80 };
        uint8_t code[] = { 0x8D, 0x4C, 0x24, 0x10, 0x51 };
        memcpy((void*)0x0041DC80, code, sizeof(code));
        doWriteCall(0x0041DC80 + sizeof(code), 0x0041DC96 - (0x0041DC80 + sizeof(code)), &hookCloseCpu);
    }

    /*
    StartRecompilerCPU
    00432EA6  push        ebx  
    00432EA7  push        eax  
    00432EA8  call        dword ptr ds:[46718Ch]  
    00432EAE  mov         dword ptr ds:[4AEB3Ch],ebx  
    00432EB4  mov         eax,dword ptr ds:[004D81ACh]  
    00432EB9  cmp         eax,ebx  
    00432EBB  je          00432EBF  
    00432EBD  call        eax  
    00432EBF  mov         eax,dword ptr ds:[004D7FE4h]  
    00432EC4  cmp         eax,ebx  
    00432EC6  je          00432ECA  
    00432EC8  call        eax  
    00432ECA  call        0042BA40  
    00432ECF  mov         ecx,803h  
    00432ED4  xor         eax,eax  
    00432ED6  mov         edi,53C660h  
    00432EDB  rep stos    dword ptr es:[edi]
    */
    // Only handling recompiler case here, don't care about other crap
    if (Config::get().fastResets)
    {
        writeCall(0x00432EB4, 0x00432ECA - 0x00432EB4, &hookStartRecompiledCpuRomOpen);
    }

    // hook create file just for debugging
    // *(uintptr_t*)0x467068 = (uintptr_t)&HookManager::hookCreateFileA;

    // Optimized unz
    // call        0045CC98 - sprintf
    // 0044A27D  je          0044A5DC - if (!Allocate_ROM())
    // 0044A343  jne         0044A5F5 - if ((int)RomFileSize != len)
    // 0044A118  or          ecx,0FFFFFFFFh  strcpy start
    // 4AF0F0h - CurrentFileName
 
    // 0044A175  call        00419FB0 - unzOpen
    // 0044A1B9  call        0041AEE0 - unzGoToFirstFile
    // 0044A203  call        0041A590 - unzGetCurrentFileInfo
    // 0044A213  call        0041AFB0 - unzLocateFile
    // 0044A224  call        0041B1A0 - unzOpenCurrentFile
    // 0044A23C  call        0041B5C0 - unzReadCurrentFile
    // 0044A5DD  call        0041B7B0 - unzCloseCurrentFile
    // 0044A5E3  call        0041A520 - unzClose
    // 0044A578  call        0041AF30 - unzGoToNextFile
    if (Config::get().useFastDecompression)
    {
        writeJump(0x00419FB0, 5, unzDispatchOpen);
        writeJump(0x0041AEE0, 5, unzDispatchGoToFirstFile);
        writeJump(0x0041A590, 5, unzDispatchGetCurrentFileInfo);
        writeJump(0x0041AFB0, 5, unzDispatchLocateFile);
        writeJump(0x0041B1A0, 5, unzDispatchOpenCurrentFile);
        writeJump(0x0041B5C0, 5, unzDispatchReadCurrentFile);
        writeJump(0x0041B7B0, 5, unzDispatchCloseCurrentFile);
        writeJump(0x0041A520, 5, unzDispatchClose);
        writeJump(0x0041AF30, 5, unzDispatchGoToNextFile);
    }
    else if (Config::get().useFastDecompression)
    {
        writeJump(0x00419FB0, 5, unzOpen);
        writeJump(0x0041AEE0, 5, unzGoToFirstFile);
        writeJump(0x0041A590, 5, unzGetCurrentFileInfo);
        writeJump(0x0041AFB0, 5, unzLocateFile);
        writeJump(0x0041B1A0, 5, unzOpenCurrentFile);
        writeJump(0x0041B5C0, 5, unzReadCurrentFile);
        writeJump(0x0041B7B0, 5, unzCloseCurrentFile);
        writeJump(0x0041A520, 5, unzClose);
        writeJump(0x0041AF30, 5, unzGoToNextFile);
    }

    // CF1 by default
    // 004492FA C7 05 E0 75 4D 00 02 00 00 00 mov         dword ptr ds:[4D75E0h],2 
    if (Config::get().cf1ByDefault)
    {
        UnprotectedMemoryScope scope{ (void*)0x449300, 1 };
        *(uint8_t*)0x449300 = 1;
    }

    // main runloop
    // 0040FCD0 A1 10 76 4D 00       mov         eax, dword ptr ds : [004D7610h]
    // 0040FCD5 85 C0                test        eax, eax
    // 0040FCD7 75 1F                jne         0040FCF8
    // 0040FCD9 8B 54 24 28          mov         edx, dword ptr[esp + 28h]
    // 0040FCDD A1 50 7F 4D 00       mov         eax, dword ptr ds : [004D7F50h]
    // 0040FCE2 8D 4C 24 54          lea         ecx, [esp + 54h]
    // 0040FCE6 51                   push        ecx
    // ...
    // 0040FD4E 51                   push        ecx  
    // 0040FD4F FF D3                call        ebx
    // 0040FD51 6A 00                push        0
    // 0040FD53 6A 00                push        0
    // 0040FD55 6A 00                push        0
    // 0040FD57 8D 54 24 60          lea         edx, [esp + 60h]
    // 0040FD5B 52                   push        edx
    // 0040FD5C FF D5                call        ebp
    // 0040FD5E 85 C0                test        eax, eax
    // 0040FD60 0F 85 6A FF FF FF    jne         0040FCD0

    // I want [esp + 54h] which is &msg to be passed as first param for convenience sake
    if (Config::get().overrideHotKeys)
    {
        uint8_t* ptr = (uint8_t*) 0x0040FCD0;
        size_t sz = 0x0040FD60 - 0x0040FCD0 + 6;
        UnprotectedMemoryScope scope{ ptr, sz };
        ptr[0] = 0x8D; // lea         ecx, [esp + 54h]
        ptr[1] = 0x4C;
        ptr[2] = 0x24;
        ptr[3] = 0x54;
        ptr[4] = 0x51; // push        ecx
        doWriteCall((uintptr_t) (ptr + 5), sz - 5, &HookManager::WinMain_RunLoopHook);
    }

    if (Config::get().fixRecompilerUnhandledOpCodeCrashes)
    {
        writeCall(0x004304ae, 5, &HookManager::hookR4300i_LW_VAddr);
        writeCall(0x00431516, 5, &HookManager::hookR4300i_LW_VAddr);
    }

    if (Config::get().removeCICChipWarning)
    {
        {
            UnprotectedMemoryScope scope{ (uint8_t*)0x445fc6, 5 };
            fillNop((uint8_t*)0x445fc6, 5);
        }
        {
            UnprotectedMemoryScope scope{ (uint8_t*)0x42082c, 5 };
            fillNop((uint8_t*)0x42082c, 5);
        }
    }

    if (Config::get().overclockVI)
    {
        // Welcome to MSVC codegen hell :)
        /*
			VI_INTR_TIME = (VI_V_SYNC_REG + 1) * 1500;
            ecx = (eax + 1) * 1500
            0041FEF7 8D 44 40 03          lea         eax,[eax+eax*2+3]  
            0041FEFB 8D 04 80             lea         eax,[eax+eax*4]  
            0041FEFE 8D 04 80             lea         eax,[eax+eax*4]  
            0041FF01 8D 0C 80             lea         ecx,[eax+eax*4]  
            0041FF04 C1 E1 02             shl         ecx,2  

            lea ecx, [eax + 1]
            imul ecx, ecx, 2200
        */
        {
            static const unsigned char code[] = { 0x8D, 0x48, 0x01, 
                                                  0x69, 0xC9, 0x98, 0x08, 0x00, 0x00 };
            UnprotectedMemoryScope scope{ (void*)0x0041FEF7, 0x0041FF07 - 0x0041FEF7 };
            memcpy((void*)0x41FEF7, code, sizeof(code));
            fillNop((uint8_t*)0x41FEF7 + sizeof(code), 0x0041FF07 - 0x0041FEF7 - sizeof(code));
        }

        /*
         	HalfLine = (Timers.Timer / 1500);
            edx = eax / 1500
            This is number shifting mult madness. I just used godbolt to generate a new payload in a similar "style"
            00447ee9 b8f1197605      mov     eax, 0x057619f1
            00447eee f7e9            imul    ecx
            00447ef0 c1fa05          sar     edx,5
            00447ef3 8bc2            mov     eax,edx
            00447ef5 c1e81f          shr     eax,1Fh
            00447ef8 03d0            add     edx,eax
            00447efa a174524d00      mov     eax,dword ptr [Project64+0xd5274 (004d5274)] ; ViFieldNumber

            Pay attention that result must be in edx so remove the last 'add' + 'shr' + 'mov' as they are the same
            int __fastcall hell1(int num) { return num / 2200; }
            int hell1(int) PROC                           ; hell1, COMDAT
              00000 b8 73 07 28 77     mov     eax, 1999112051                ; 77280773H
              00005 f7 e9        imul        ecx
              00007 c1 fa 0a   sar         edx, 10                        ; 0000000aH
              0000a 8b c2        mov         eax, edx
              0000c c1 e8 1f   shr         eax, 31                        ; 0000001fH
              0000f 03 c2        add         eax, edx
              00011 c3                 ret     0
            int hell1(int) ENDP                           ; hell1
        */
        {
            // static const unsigned char code[] = { 0xb8, 0x73, 0x07, 0x28, 0x77, 
            //                                       0xf7, 0xe9, 
            //                                       0xc1, 0xfa, 0x0a };
            UnprotectedMemoryScope scope{ (void*)0x00447ee9, 10 };
            *(uint32_t*)0x00447eea = 0x77280773;
            *(uint8_t*)0x00447ef2 = 10;
        }
    }

    if (Config::get().cf0)
    {
        UnprotectedMemoryScope scope{ (void*)0x00431513, 8 };
        // 00431513 8b4b24          mov     ecx, dword ptr[ebx + 24h]
        // 00431516 e815582806      call    RSP!HookManager::hookR4300i_LW_VAddr(066b6d30)
        // mov ecx, ebx + nop
        *(uint8_t*)0x00431513 = 0x89;
        *(uint8_t*)0x00431514 = 0xd9;
        *(uint8_t*)0x00431515 = 0x90;
        doWriteCall(0x00431516, 5, &HookManager::hookR4300i_LW_VAddrSection);
    }

    if (Config::get().fastResets || Config::get().fastStates || Config::get().fixSlowResets || Config::get().noLoadSlowdown)
    {
        writeCall(0x0041F355, 0x6, &HookManager::hookAiDacrateChanged);
        {
            UnprotectedMemoryScope scope{ (void*)0x0042B3C7, 0xf + 2 };
            *(uint8_t*)0x0042B3C7 = 0x6a;
            *(uint8_t*)0x0042B3C8 = 0x00;
            doWriteCall(0x0042B3C9, 0xf, &HookManager::hookAiDacrateChanged);
        }
    }
}

static inline BOOL r4300i_LW_VAddr(DWORD VAddr, DWORD* Value) 
{
    auto map = PJ64::Globals::TLB_ReadMap();
    if (map[VAddr >> 12] == 0) 
    {
        return FALSE; 
    }

    *Value = *(DWORD*)(map[VAddr >> 12] + VAddr);
    return TRUE;
}


BOOL __fastcall HookManager::hookR4300i_LW_VAddr(DWORD VAddr, DWORD* Value)
{
    OPCODE opcode;
    BOOL ok = r4300i_LW_VAddr(VAddr, &opcode.Hex);
    if (!ok)
        return FALSE;

    if (opcode.op == R4300i_SPECIAL)
    {
        if (R4300i_SPECIAL_TGE <= opcode.funct && opcode.funct <= R4300i_SPECIAL_TNE)
        {
            opcode.Hex = 0;
        }
        if (R4300i_SPECIAL_BREAK == opcode.funct)
        {
            opcode.Hex = 0;
        }
    }
    if (opcode.op == R4300i_REGIMM)
    {
        if (R4300i_REGIMM_TGEI <= opcode.rt && opcode.rt <= R4300i_REGIMM_TNEI)
        {
            opcode.Hex = 0;
        }
    }

    *Value = opcode.Hex;
    return TRUE;
}

#define BlockCycleCount			section->RegWorking.CycleCount
#define BlockRandomModifier		section->RegWorking.RandomModifier
#define MAX_CF0_CYCLE_COUNT 1

BOOL __fastcall HookManager::hookR4300i_LW_VAddrSection(BLOCK_SECTION* section, DWORD* Value)
{
#if MAX_CF0_CYCLE_COUNT == 1
    if (BlockCycleCount)
#else
    if (BlockCycleCount > MAX_CF0_CYCLE_COUNT)
#endif
    {
        BlockCycleCount = MAX_CF0_CYCLE_COUNT;
    }
    return hookR4300i_LW_VAddr(section->CompilePC, Value);
}

#define INVOKE_PJ64_PLUGIN_CALLBACK(name) if (auto fn = PJ64::Globals::name()) { fn(); }

static bool gIsInitialized = false;
static std::mutex gAudioMutex;

void HookManager::hookCloseCpuRomClosed()
{
    auto type = ResetRecognizer::recognize(ResetRecognizer::stackAddresses());
    bool fastReset = type == ResetRecognizer::Type::OPEN_ROM || type == ResetRecognizer::Type::RESET;
    if (1 != PJ64::Globals::CPU_Type() || !fastReset)
    {
        gIsInitialized = false;
        INVOKE_PJ64_PLUGIN_CALLBACK(GfxRomClosed)
        INVOKE_PJ64_PLUGIN_CALLBACK(ContRomClosed)
    }
    {
        std::lock_guard<std::mutex> lck(gAudioMutex);
        INVOKE_PJ64_PLUGIN_CALLBACK(AiRomClosed)
    }
    INVOKE_PJ64_PLUGIN_CALLBACK(RSPRomClosed)
}

void HookManager::hookStartRecompiledCpuRomOpen()
{
    if (!gIsInitialized)
    {
        gIsInitialized = true;
        INVOKE_PJ64_PLUGIN_CALLBACK(GfxRomOpen)
        INVOKE_PJ64_PLUGIN_CALLBACK(ContRomOpen)
    }
}

void HookManager::hookAiDacrateChanged(int systemType)
{
    std::lock_guard<std::mutex> lck(gAudioMutex);
    return PJ64::Globals::AiDacreateChanged()(systemType);
}

void HookManager::hookMachine_LoadStateRomReinit()
{
    // INVOKE_PJ64_PLUGIN_CALLBACK(GfxRomClosed)
    {
        std::lock_guard<std::mutex> lck(gAudioMutex);
        INVOKE_PJ64_PLUGIN_CALLBACK(AiRomClosed)
    }
    // INVOKE_PJ64_PLUGIN_CALLBACK(ContRomClosed)
    INVOKE_PJ64_PLUGIN_CALLBACK(RSPRomClosed)
    // INVOKE_PJ64_PLUGIN_CALLBACK(GfxRomOpen)
    // INVOKE_PJ64_PLUGIN_CALLBACK(ContRomOpen)
    int a = 0;
}

LRESULT HookManager::hookMachine_LoadStateFinished(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = SendMessageA(hWnd, Msg, wParam, lParam);
    PJ64::Globals::FPSTimer()->reset();
    return result;
}

BOOL __fastcall HookManager::hookMachine_LoadState(void)
{
    auto start = timeGetTime();
    BOOL ok = PJ64::Globals::Machine_LoadState()();
    auto end = timeGetTime();
    PJ64::Globals::FPSTimer()->adjust(end - start);
    return ok;
}

BOOL __fastcall HookManager::hookMachine_SaveState(void)
{
    auto start = timeGetTime();
    BOOL ok = PJ64::Globals::Machine_SaveState()();
    auto end = timeGetTime();
    PJ64::Globals::FPSTimer()->adjust(end - start);
    return ok;
}

void __stdcall HookManager::hookCloseCpu(DWORD* ExitCode)
{
    auto tid = GetCurrentThreadId();
    std::thread waitThread{ [=]() {
        HANDLE hCPU = PJ64::Globals::hCPU();
        WaitForSingleObject(hCPU, 100);
        GetExitCodeThread(hCPU, ExitCode);
        PostThreadMessage(tid, WM_APP + 1, 0, 0);
    } };

    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        if (msg.message == WM_APP + 1)
            break;

        DispatchMessage(&msg);
    }
    waitThread.join();
}

BOOL __fastcall HookManager::RefreshScreen_TimerProcess(DWORD* FrameRate)
{
    return PJ64::Globals::FPSTimer()->process(nullptr);
}

static void setCurrentSaveState(HWND hWnd, int state) {
    HMENU hMenu = GetMenu(hWnd);
    if (!PJ64::Globals::CPURunning()) 
    { 
        state = ID_CURRENTSAVE_DEFAULT;
    }

    if (0 == strlen(PJ64::Globals::RomName())) 
    { 
        return;
    }

    sprintf_s(PJ64::Globals::CurrentSave(), 256, "%s.pj%s", PJ64::Globals::RomName(), vkToString(state).c_str());
    auto string = std::string("LINK's state: ") + PJ64::Globals::CurrentSave();
    SendMessage(PJ64::Globals::StatusWin(), SB_SETTEXT, 0, (LPARAM)string.c_str());
}

void __stdcall HookManager::WinMain_RunLoopHook(LPMSG msg)
{
    auto mainWindow = PJ64::Globals::MainWindow();
    const auto& cfg = Config::get();
    do
    {
        if (!PJ64::Globals::CPURunning() && cfg.accelRomBrowser && TranslateAccelerator(mainWindow, cfg.accelRomBrowser, msg)) 
        { 
            continue;
        }
        if (PJ64::Globals::CPURunning())
        {
            if (cfg.accelCPURunning && TranslateAccelerator(mainWindow, cfg.accelCPURunning, msg)) 
            { 
                continue;
            }

            if (!PJ64::Globals::InFullScreen() && cfg.accelWinMode && TranslateAccelerator(mainWindow, cfg.accelWinMode, msg)) 
            { 
                continue;
            }
        }

        if (Config::get().experimental_eachKeySavestateSlot && msg->message == WM_KEYDOWN)
        {
            if (HotKey::virtualCodeAllowed(msg->wParam) 
             && Config::get().ignoredSavestateSlotKeys.find(msg->wParam) == Config::get().ignoredSavestateSlotKeys.end())
            {
                setCurrentSaveState(PJ64::Globals::MainWindow(), (int) msg->wParam);
            }
        }

        if (IsDialogMessage(PJ64::Globals::ManageWindow(), msg)) { continue; }
        TranslateMessage(msg);
        DispatchMessage(msg);
    } while (GetMessage(msg, NULL, 0, 0));
}

static bool tryNamePJ64() noexcept
{
    bool ok = false;
    __try
    {
        ok = true;
        ok = !ok ? ok : SetWindowText(PJ64::Globals::MainWindow(), NAME);
        ok = !ok ? ok : SetWindowText(PJ64::Globals::HiddenWin(), NAME);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // not ok
        return false;
    }

    return ok;
}

// MARK: Enterance from 'RSP' init
bool plantHooks()
{
    bool ok = false;
    // Check if we are in PJ64 1.6, heuristically
    {
        auto addresses = Backtrace::collectStackAddresses();
        constexpr uintptr_t PJ64GetDllInfoSym0 = 0x0044dce7;
        constexpr uintptr_t PJ64GetDllInfoSym1 = 0x0044dcf7;
        for (uintptr_t addr : addresses)
        {
            if (PJ64GetDllInfoSym0 == addr || PJ64GetDllInfoSym1 == addr)
            {
                ok = true;
                break;
            }
        }
    } 

    ok = !ok ? ok : tryNamePJ64();

    if (ok)
    {
        gHookManager.init();
    }

    return ok;
}

void zapRSPInit()
{
    /*
     Zapping the function writes prolog immediately after DllInfo call
     0044DD04 83 C4 04             add         esp,4  
     ...
     0044DE59 5F                   pop         edi  
     0044DE5A 5E                   pop         esi  
     ...
     0044DE60 B8 01 00 00 00       mov         eax,1  
     0044DE65 5B                   pop         ebx  
     0044DE66 81 C4 9C 01 00 00    add         esp,19Ch  
     0044DE6C C3                   ret  
    */
    static const unsigned char code[] = { 0x83, 0xC4, 0x04, 
                                          0x5F, 0x5E, 
                                          0xB8, 0x01, 0x00, 0x00, 0x00, 0x5B, 0x81, 0xC4, 0x9C, 0x01, 0x00, 0x00, 0xC3 };

    UnprotectedMemoryScope scope{ (void*)0x44DCF7, sizeof(code) };
    memcpy((void*)0x44DCF7, code, sizeof(code));
}
