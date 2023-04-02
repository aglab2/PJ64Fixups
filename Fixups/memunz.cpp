#include "memunz.h"

#include "minizip.h"

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

#define MEM_CACHE_SIZE 0x900000
static uint8_t sMemCache[0x900000];

struct unz_mem_cache
{
	mz_compat _mz;
	uint8_t* ptr;
};

unzFile ZEXPORT unzMemCacheOpen(const char* path)
{
	unz_mem_cache* memCache = (unz_mem_cache*) malloc(sizeof(unz_mem_cache));
	if (!memCache)
		return NULL;

	if (unzCtor((unzFile) memCache, path))
	{
		free(memCache);
		return NULL;
	}

	memCache->ptr = sMemCache;
	return (unzFile)memCache;
}

int ZEXPORT unzMemCacheReadCurrentFile(unzFile file, void* buf, unsigned len)
{
	unz_mem_cache* memCache = (unz_mem_cache*)file;
	int sz = unzReadCurrentFile(file, buf, len);
	if (sz > 0 && memCache->ptr + sz < sMemCache + MEM_CACHE_SIZE)
	{
		memcpy(memCache->ptr, buf, sz);
		memCache->ptr += sz;
	}

	return 0;
}

int ZEXPORT unzMemCacheCloseCurrentFile(unzFile file)
{
	// revert back to the start if current file is closed
	unz_mem_cache* memCache = (unz_mem_cache*)file;
	memCache->ptr = sMemCache;
	return unzCloseCurrentFile(file);
}

struct unz_mem_replay
{
	unzDispatchVTable _vtable;
	uint8_t* ptr;
	bool first;
};

unzFile ZEXPORT unzOpenFromMem()
{
	unz_mem_replay* memReplay = (unz_mem_replay*)malloc(sizeof(unz_mem_replay));
	if (!memReplay)
		return NULL;

	memReplay->ptr = sMemCache;
	memReplay->first = true;
	return (unzFile)memReplay;
}

int ZEXPORT unzMemReplayOpenCurrentFile(unzFile file)
{
	unz_mem_replay* memReplay = (unz_mem_replay*)file;
	return memReplay->first ? 0 : UNZ_PARAMERROR;
}

int ZEXPORT unzMemReplayGoToFirstFile(unzFile file)
{
	unz_mem_replay* memReplay = (unz_mem_replay*)file;
	memReplay->first = true;
	return 0;
}

int ZEXPORT unzMemReplayLocateFile(unzFile file, const char* szFileName, int iCaseSensitivity)
{
	unz_mem_replay* memReplay = (unz_mem_replay*)file;
	return memReplay->first ? 0 : UNZ_PARAMERROR;
}

int ZEXPORT unzMemReplayClose(unzFile file)
{
	free(file);
	return 0;
}

int ZEXPORT unzMemReplayReadCurrentFile(unzFile file, void* buf, unsigned len)
{
	unz_mem_replay* memReplay = (unz_mem_replay*)file;
	if (memReplay->ptr + len >= sMemCache + MEM_CACHE_SIZE)
		return UNZ_PARAMERROR;

	memcpy(buf, memReplay->ptr, len);
	memReplay->ptr += len;
	return 0;
}

int ZEXPORT unzMemReplayCloseCurrentFile(unzFile file)
{
	unz_mem_replay* memReplay = (unz_mem_replay*)file;
	memReplay->first = false;
	return 0;
}

int ZEXPORT unzMemReplayGetCurrentFileInfo(unzFile file, unz_file_info* pfile_info, char* szFileName, uLong fileNameBufferSize, void* extraField, uLong extraFieldBufferSize, char* szComment, uLong commentBufferSize)
{
	return 0;
}

int ZEXPORT unzMemReplayGoToNextFile(unzFile file)
{
	unz_mem_replay* memReplay = (unz_mem_replay*)file;
	memReplay->first = false;
	return 0;
}
