#pragma once

#ifndef ZEXPORT
#define ZEXPORT __cdecl
#endif

typedef struct unz_file_info_s unz_file_info;

typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */

typedef struct unzDispatchVTable* unzFile;

typedef int (ZEXPORT *DispatchOpenCurrentFileFn)(unzFile file);
typedef int (ZEXPORT *DispatchGoToFirstFileFn)(unzFile file);
typedef int (ZEXPORT *DispatchLocateFileFn)(unzFile file, const char* szFileName, int iCaseSensitivity);
typedef int (ZEXPORT *DispatchCloseFn)(unzFile file);
typedef int (ZEXPORT *DispatchReadCurrentFileFn)(unzFile file, void* buf, unsigned len);
typedef int (ZEXPORT *DispatchCloseCurrentFileFn)(unzFile file);
typedef int (ZEXPORT *DispatchGetCurrentFileInfoFn)(unzFile file, unz_file_info* pfile_info, char* szFileName, uLong fileNameBufferSize, void* extraField, uLong extraFieldBufferSize, char* szComment, uLong commentBufferSize);
typedef int (ZEXPORT *DispatchGoToNextFileFn)(unzFile file);

struct unzDispatchVTable
{
	DispatchOpenCurrentFileFn	 openCurrentFile;
	DispatchGoToFirstFileFn	     goToFirstFile;
	DispatchLocateFileFn		 locateFile;
	DispatchCloseFn				 close;
	DispatchReadCurrentFileFn	 readCurrentFile;
	DispatchCloseCurrentFileFn   closeCurrentFile;
	DispatchGetCurrentFileInfoFn getCurrentFileInfo;
	DispatchGoToNextFileFn		 goToNextFile;
};

unzFile ZEXPORT unzDispatchOpen(const char* path);
int ZEXPORT unzDispatchOpenCurrentFile(unzFile file);
int ZEXPORT unzDispatchGoToFirstFile(unzFile file);
int ZEXPORT unzDispatchLocateFile(unzFile file, const char* szFileName, int iCaseSensitivity);
int ZEXPORT unzDispatchClose(unzFile file);
int ZEXPORT unzDispatchReadCurrentFile(unzFile file, void* buf, unsigned len);
int ZEXPORT unzDispatchCloseCurrentFile(unzFile file);
int ZEXPORT unzDispatchGetCurrentFileInfo(unzFile file, unz_file_info* pfile_info, char* szFileName, uLong fileNameBufferSize, void* extraField, uLong extraFieldBufferSize, char* szComment, uLong commentBufferSize);
int ZEXPORT unzDispatchGoToNextFile(unzFile file);
