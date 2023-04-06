#include <Windows.h>

#include <iostream>
#include <string>

#include <DbgHelp.h>
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>

// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main(int argc, char* argv[])
{
    if (argc < 3) 
    {
        std::cerr << "Usage: MiniDumpCollector <EventHandleValue> <ProcessID> <DumpFlags>" << std::endl;
        return 1;
    }

    char minidumpPath[MAX_PATH] = {};
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, minidumpPath);
    PathAppend(minidumpPath, "PJ64Fixups");
    CreateDirectory(minidumpPath, nullptr);
    PathAppend(minidumpPath, "minidump.dmp");

    HANDLE hEvent = reinterpret_cast<HANDLE>(std::stoull(argv[1]));
    DWORD processID = std::stoull(argv[2]);
    MINIDUMP_TYPE flags = (MINIDUMP_TYPE) std::stoull(argv[3]);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess == NULL)
    {
        std::cerr << "Failed to open the process: " << GetLastError() << std::endl;
        return 1;
    }

    HANDLE handles[] = { hEvent, hProcess };
    DWORD result = WaitForMultipleObjects(2, handles, false, INFINITE);
    CloseHandle(hEvent);

    if (WAIT_OBJECT_0 == result)
    {
        // Create the MiniDump file
        HANDLE hFile = CreateFile(minidumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Failed to create minidump file: " << GetLastError() << std::endl;
            CloseHandle(hProcess);
            return 1;
        }

        // Write the MiniDump
        BOOL result = MiniDumpWriteDump(hProcess, processID, hFile, flags, NULL, NULL, NULL);
        if (!result)
        {
            std::cerr << "Failed to write the minidump: " << GetLastError() << std::endl;
        }
        else
        {
            std::cout << "Minidump created successfully!" << std::endl;
        }
        CloseHandle(hFile);
    }
    else if (WAIT_OBJECT_0 + 1 == result)
    {
        std::cout << "Process is done...";
        // - do nothing, process is done
    }
    else
    {
        std::cerr << "WaitForMultipleObjects failed: " << GetLastError() << std::endl;
        return 1;
    }

    // Clean up
    CloseHandle(hProcess);

    return 0;
}
