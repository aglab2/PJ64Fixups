#include <Windows.h>

#include "resource1.h"

#include <io.h>
#include <fcntl.h>
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>

#include <string>

#define WANT_BIG_DUMP

extern HINSTANCE hInstance;

static HANDLE hChildProcess;
static HANDLE hChildThread;
static HANDLE hEvent;

static const char* getMiniDumpCollectorPath()
{
	static char path[MAX_PATH]{ 0 };
	if ('\0' == *path)
	{
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
		PathAppend(path, "PJ64Fixups");
		CreateDirectory(path, nullptr);
		PathAppend(path, "MiniDumpCollector.exe");
	}
	return path;
}

static const char* unpack()
{
	auto path = getMiniDumpCollectorPath();
	int fd = _open(path, _O_BINARY | _O_WRONLY | _O_CREAT | _O_EXCL, 0777);
	if (-1 != fd)
	{
		auto rc = FindResource(hInstance, MAKEINTRESOURCE(IDR_MDC), RT_RCDATA);
		auto res = LoadResource(hInstance, rc);
		void* data = (wchar_t*)LockResource(res);
		size_t size = SizeofResource(hInstance, rc);
		_write(fd, data, size);
		_close(fd);
	}

	return path;
}

static void collectMinidump(EXCEPTION_POINTERS* ex)
{
	SetEvent(hEvent);
	WaitForSingleObject(hChildProcess, 10000);
	CloseHandle(hChildThread);
	CloseHandle(hChildProcess);
}

static LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS* ex)
{
    collectMinidump(ex);
    ExitProcess(1);
}

void setupExceptionFilters()
{
    // Create an unnamed, inheritable event
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    hEvent = CreateEvent(&sa, FALSE, FALSE, NULL);

	STARTUPINFOA startupInfo = { 0 };
	PROCESS_INFORMATION processInfo = { 0 };

	std::string path = unpack();
	std::string cmdLine = path 
						+ ' ' + std::to_string((uint64_t) hEvent)
						+ ' ' + std::to_string(GetCurrentProcessId()) 
						+ ' ' + std::to_string(0);

	if (!CreateProcess(NULL, (LPSTR) cmdLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo))
		return;

	hChildProcess = processInfo.hProcess;
	hChildThread = processInfo.hThread;

	SetUnhandledExceptionFilter(&unhandledExceptionFilter);
}
