#include "unzdispatch.h"

#include "backtrace.h"
#include "memunz.h"
#include "minizip.h"

#include <string>

static std::string sMemCachePath;
static struct stat sStatePathStat;

enum class DispatchType
{
	MINIZIP,
	MEM_RECORD,
	MEM_REPLAY,
};

unzFile ZEXPORT unzDispatchOpen(const char* path)
{
	auto trace = Backtrace::collectStackAddresses();
	constexpr uintptr_t PJ64LoadStateFn = 0x0041e872;
	bool loadingState = false;
	for (auto fn : trace)
	{
		if (fn == PJ64LoadStateFn)
		{
			loadingState = true;
		}
	}

	struct stat sb;
	DispatchType dispatchType = DispatchType::MINIZIP;
	if (loadingState && 0 == stat(path, &sb))
	{
		if (sMemCachePath == path
		 && sb.st_dev == sStatePathStat.st_dev
		 && sb.st_ino == sStatePathStat.st_ino
		 && sb.st_size == sStatePathStat.st_size
		 && sb.st_mtime == sStatePathStat.st_mtime
		 && sb.st_ctime == sStatePathStat.st_ctime)
		{
			dispatchType = DispatchType::MEM_REPLAY;
		}
		else
		{
			sMemCachePath = path;
			sStatePathStat = sb;
			dispatchType = DispatchType::MEM_RECORD;
		}
	}

	unzFile file = NULL;
	switch (dispatchType)
	{
	case DispatchType::MINIZIP:
	{
		file = unzOpen(path);
		if (file)
		{
			file->openCurrentFile = unzOpenCurrentFile;
			file->goToFirstFile = unzGoToFirstFile;
			file->locateFile = unzLocateFile;
			file->close = unzClose;
			file->readCurrentFile = unzReadCurrentFile;
			file->closeCurrentFile = unzCloseCurrentFile;
			file->getCurrentFileInfo = unzGetCurrentFileInfo;
			file->goToNextFile = unzGoToNextFile;
		}
	}
	break;
	case DispatchType::MEM_RECORD:
	{
		file = unzMemCacheOpen(path);
		if (file)
		{
			file->openCurrentFile = unzOpenCurrentFile;
			file->goToFirstFile = unzGoToFirstFile;
			file->locateFile = unzLocateFile;
			file->close = unzClose;
			file->readCurrentFile = unzMemCacheReadCurrentFile;
			file->closeCurrentFile = unzMemCacheCloseCurrentFile;
			file->getCurrentFileInfo = unzGetCurrentFileInfo;
			file->goToNextFile = unzGoToNextFile;
		}
	}
	break;
	case DispatchType::MEM_REPLAY:
	{
		file = unzOpenFromMem();
		if (file)
		{
			file->openCurrentFile = unzMemReplayOpenCurrentFile;
			file->goToFirstFile = unzMemReplayGoToFirstFile;
			file->locateFile = unzMemReplayLocateFile;
			file->close = unzMemReplayClose;
			file->readCurrentFile = unzMemReplayReadCurrentFile;
			file->closeCurrentFile = unzMemReplayCloseCurrentFile;
			file->getCurrentFileInfo = unzMemReplayGetCurrentFileInfo;
			file->goToNextFile = unzMemReplayGoToNextFile;
		}
	}
	break;
	}
	return file;
}

int ZEXPORT unzDispatchOpenCurrentFile(unzFile file)
{
	return file->openCurrentFile(file);
}

int ZEXPORT unzDispatchGoToFirstFile(unzFile file)
{
	return file->goToFirstFile(file);
}

int ZEXPORT unzDispatchLocateFile(unzFile file, const char* szFileName, int iCaseSensitivity)
{
	return file->locateFile(file, szFileName, iCaseSensitivity);
}

int ZEXPORT unzDispatchClose(unzFile file)
{
	return file->close(file);
}

int ZEXPORT unzDispatchReadCurrentFile(unzFile file, void* buf, unsigned len)
{
	return file->readCurrentFile(file, buf, len);
}

int ZEXPORT unzDispatchCloseCurrentFile(unzFile file)
{
	return file->closeCurrentFile(file);
}

int ZEXPORT unzDispatchGetCurrentFileInfo(unzFile file, unz_file_info* pfile_info, char* szFileName, uLong fileNameBufferSize, void* extraField, uLong extraFieldBufferSize, char* szComment, uLong commentBufferSize)
{
	return file->getCurrentFileInfo(file, pfile_info, szFileName, fileNameBufferSize, extraField, extraFieldBufferSize, szComment, commentBufferSize);
}

int ZEXPORT unzDispatchGoToNextFile(unzFile file)
{
	return file->goToNextFile(file);
}
