#pragma once

#include "unzdispatch.h"

unzFile ZEXPORT unzMemCacheOpen(const char* path);
int ZEXPORT unzMemCacheReadCurrentFile(unzFile file, void* buf, unsigned len);
int ZEXPORT unzMemCacheCloseCurrentFile(unzFile file);

unzFile ZEXPORT unzOpenFromMem();
int ZEXPORT unzMemReplayOpenCurrentFile(unzFile file);
int ZEXPORT unzMemReplayGoToFirstFile(unzFile file);
int ZEXPORT unzMemReplayLocateFile(unzFile file, const char* szFileName, int iCaseSensitivity);
int ZEXPORT unzMemReplayClose(unzFile file);
int ZEXPORT unzMemReplayReadCurrentFile(unzFile file, void* buf, unsigned len);
int ZEXPORT unzMemReplayCloseCurrentFile(unzFile file);
int ZEXPORT unzMemReplayGetCurrentFileInfo(unzFile file, unz_file_info* pfile_info, char* szFileName, uLong fileNameBufferSize, void* extraField, uLong extraFieldBufferSize, char* szComment, uLong commentBufferSize);
int ZEXPORT unzMemReplayGoToNextFile(unzFile file);
