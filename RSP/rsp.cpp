#define _CRT_SECURE_NO_WARNINGS

#include "rsp.h"
#include "resource1.h"

#include <io.h>
#include <fcntl.h>
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>

extern HINSTANCE hInstance;
namespace RSP
{
	static const char* getRSPPath()
	{
		static char path[MAX_PATH]{ 0 };
		if ('\0' == *path)
		{
			SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
			PathAppend(path, "PJ64Fixups");
			CreateDirectory(path, nullptr);
			PathAppend(path, "RSP.dll");
		}
		return path;
	}

	const char* unpack()
	{
		auto path = getRSPPath();
		int fd = _open(path, _O_BINARY | _O_WRONLY | _O_CREAT | _O_EXCL, 0777);
		if (-1 != fd)
		{
			auto rc = FindResource(hInstance, MAKEINTRESOURCE(IDR_RSP), RT_RCDATA);
			auto res = LoadResource(hInstance, rc);
			void* data = (wchar_t*)LockResource(res);
			size_t size = SizeofResource(hInstance, rc);
			_write(fd, data, size);
			_close(fd);
		}

		return path;
	}
}