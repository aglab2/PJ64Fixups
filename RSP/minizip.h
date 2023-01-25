#pragma once

#include "unzdispatch.h"

#include <stdint.h>

#if 0
// MARK: ZIP
#define ZIP_OK                                  (0)
#define ZIP_ERRNO               (-1)
#define ZIP_PARAMERROR                  (-102)
#define ZIP_INTERNALERROR               (-104)

typedef void* zipFile;

/* tm_zip contain date/time info */
typedef struct tm_zip_s
{
    uInt tm_sec;            /* seconds after the minute - [0,59] */
    uInt tm_min;            /* minutes after the hour - [0,59] */
    uInt tm_hour;           /* hours since midnight - [0,23] */
    uInt tm_mday;           /* day of the month - [1,31] */
    uInt tm_mon;            /* months since January - [0,11] */
    uInt tm_year;           /* years - [1980..2044] */
} tm_zip;

typedef struct
{
    tm_zip      tmz_date;       /* date in understandable format           */
    uLong       dosDate;       /* if dos_date == 0, tmu_date is used      */
/*    uLong       flag;        */   /* general purpose bit flag        2 bytes */

    uLong       internal_fa;    /* internal file attributes        2 bytes */
    uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

zipFile ZEXPORT zipOpen(const char* pathname, int append);
int ZEXPORT zipOpenNewFileInZip(zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level);
int ZEXPORT zipWriteInFileInZip(zipFile file, const void* buf, unsigned len);
int ZEXPORT zipCloseFileInZip(zipFile file);
int ZEXPORT zipClose(zipFile file, const char* global_comment);
#endif

// MARK: UNZ
#define UNZ_PARAMERROR                  (-102)

// returns from 'unzOpen'
typedef struct mz_compat_s {
    unzDispatchVTable _vtable;
    void* stream;
    void* handle;
    uint64_t entry_index;
    int64_t  entry_pos;
    int64_t  total_out;
} mz_compat;

/* tm_unz contain date/time info */
typedef struct tm_unz_s
{
    uInt tm_sec;            /* seconds after the minute - [0,59] */
    uInt tm_min;            /* minutes after the hour - [0,59] */
    uInt tm_hour;           /* hours since midnight - [0,23] */
    uInt tm_mday;           /* day of the month - [1,31] */
    uInt tm_mon;            /* months since January - [0,11] */
    uInt tm_year;           /* years - [1980..2044] */
} tm_unz;

/* unz_file_info contain information about a file in the zipfile */
typedef struct unz_file_info_s
{
    uLong version;              /* version made by                 2 bytes */
    uLong version_needed;       /* version needed to extract       2 bytes */
    uLong flag;                 /* general purpose bit flag        2 bytes */
    uLong compression_method;   /* compression method              2 bytes */
    uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uLong crc;                  /* crc-32                          4 bytes */
    uLong compressed_size;      /* compressed size                 4 bytes */
    uLong uncompressed_size;    /* uncompressed size               4 bytes */
    uLong size_filename;        /* filename length                 2 bytes */
    uLong size_file_extra;      /* extra field length              2 bytes */
    uLong size_file_comment;    /* file comment length             2 bytes */

    uLong disk_num_start;       /* disk number start               2 bytes */
    uLong internal_fa;          /* internal file attributes        2 bytes */
    uLong external_fa;          /* external file attributes        4 bytes */

    tm_unz tmu_date;
} unz_file_info;

unzFile ZEXPORT unzOpen(const char* path);
int unzCtor(unzFile file, const char* path);
int ZEXPORT unzOpenCurrentFile(unzFile file);
int ZEXPORT unzGoToFirstFile(unzFile file);
int ZEXPORT unzLocateFile(unzFile file, const char* szFileName, int iCaseSensitivity);
int ZEXPORT unzClose(unzFile file);
int ZEXPORT unzReadCurrentFile(unzFile file, void* buf, unsigned len);
int ZEXPORT unzCloseCurrentFile(unzFile file);
int ZEXPORT unzGetCurrentFileInfo(unzFile file, unz_file_info* pfile_info, char* szFileName, uLong fileNameBufferSize, void* extraField, uLong extraFieldBufferSize, char* szComment, uLong commentBufferSize);
int ZEXPORT unzGoToNextFile(unzFile file);
