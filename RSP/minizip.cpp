#include "minizip.h"

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_mem.h"
#include "mz_strm_os.h"
#include "mz_strm_zlib.h"
#include "mz_zip.h"

#include "unzdispatch.h"

#include <stdio.h> /* SEEK */

#if 0
static int32_t zipConvertAppendToStreamMode(int append) {
    int32_t mode = MZ_OPEN_MODE_WRITE;
    if (!append) {
        mode |= MZ_OPEN_MODE_CREATE;
    } else {
        mode |= MZ_OPEN_MODE_CREATE | MZ_OPEN_MODE_APPEND;
    }

    return mode;
}

static zipFile zipOpen_MZ(void* stream, int append)  {
    mz_compat* compat = NULL;
    int32_t err = MZ_OK;
    int32_t mode = zipConvertAppendToStreamMode(append);
    void* handle = NULL;

    mz_zip_create(&handle);
    err = mz_zip_open(handle, stream, mode);

    if (err != MZ_OK) {
        mz_zip_delete(&handle);
        return NULL;
    }

    compat = (mz_compat*)MZ_ALLOC(sizeof(mz_compat));
    if (compat != NULL) {
        compat->handle = handle;
        compat->stream = stream;
    }
    else {
        mz_zip_delete(&handle);
    }

    return (zipFile)compat;
}

zipFile ZEXPORT zipOpen(const char* path, int append) {
    zipFile zip = NULL;
    int32_t mode = zipConvertAppendToStreamMode(append);
    void* stream = NULL;

    if (stream == NULL) {
        if (mz_stream_os_create(&stream) == NULL)
            return NULL;
    }

    if (mz_stream_open(stream, path, mode) != MZ_OK) {
        mz_stream_delete(&stream);
        return NULL;
    }

    zip = zipOpen_MZ(stream, append);

    if (zip == NULL) {
        mz_stream_delete(&stream);
        return NULL;
    }

    return zip;
}

static int32_t mz_zip_invalid_date(const tm_zip* ptm) {
#define datevalue_in_range(min, max, value) ((min) <= (value) && (value) <= (max))
    return (!datevalue_in_range(0, 127 + 80, ptm->tm_year) ||  /* 1980-based year, allow 80 extra */
        !datevalue_in_range(0, 11, ptm->tm_mon) ||
        !datevalue_in_range(1, 31, ptm->tm_mday) ||
        !datevalue_in_range(0, 23, ptm->tm_hour) ||
        !datevalue_in_range(0, 59, ptm->tm_min) ||
        !datevalue_in_range(0, 59, ptm->tm_sec));
#undef datevalue_in_range
}

static uint32_t zip_tm_to_dosdate(const tm_zip * ptm) {
    tm_zip fixed_tm;

    /* Years supported: */

    /* [00, 79]      (assumed to be between 2000 and 2079) */
    /* [80, 207]     (assumed to be between 1980 and 2107, typical output of old */
    /*                software that does 'year-1900' to get a double digit year) */
    /* [1980, 2107]  (due to format limitations, only years 1980-2107 can be stored.) */

    memcpy(&fixed_tm, ptm, sizeof(struct tm));
    if (fixed_tm.tm_year >= 1980) /* range [1980, 2107] */
        fixed_tm.tm_year -= 1980;
    else if (fixed_tm.tm_year >= 80) /* range [80, 207] */
        fixed_tm.tm_year -= 80;
    else /* range [00, 79] */
        fixed_tm.tm_year += 20;

    if (mz_zip_invalid_date(&fixed_tm))
        return 0;

    return (((uint32_t)fixed_tm.tm_mday + (32 * ((uint32_t)fixed_tm.tm_mon + 1)) + (512 * (uint32_t)fixed_tm.tm_year)) << 16) |
        (((uint32_t)fixed_tm.tm_sec / 2) + (32 * (uint32_t)fixed_tm.tm_min) + (2048 * (uint32_t)fixed_tm.tm_hour));
}

int ZEXPORT zipOpenNewFileInZip(zipFile file, const char* filename, const zip_fileinfo* zipfi, const void* extrafield_local, uInt size_extrafield_local, const void* extrafield_global, uInt size_extrafield_global, const char* comment, int method, int level) {
    mz_compat* compat = (mz_compat*)file;
    mz_zip_file file_info;
    MZ_UNUSED(size_extrafield_local);
    MZ_UNUSED(extrafield_local);

    if (compat == NULL)
        return ZIP_PARAMERROR;

    memset(&file_info, 0, sizeof(file_info));

    if (zipfi != NULL) {
        uint64_t dos_date = 0;

        if (zipfi->dosDate != 0)
            dos_date = zipfi->dosDate;
        else
            dos_date = zip_tm_to_dosdate(&zipfi->tmz_date);

        file_info.modified_date = mz_zip_dosdate_to_time_t(dos_date);
        file_info.external_fa = zipfi->external_fa;
        file_info.internal_fa = (uint16_t) zipfi->internal_fa;
    }

    if (filename == NULL)
        filename = "-";

    file_info.compression_method = (uint16_t)method;
    file_info.filename = filename;
    /* file_info.extrafield_local = extrafield_local; */
    /* file_info.extrafield_local_size = size_extrafield_local; */
    file_info.extrafield = (const uint8_t*) extrafield_global;
    file_info.extrafield_size = size_extrafield_global;
    file_info.version_madeby = (uint16_t)MZ_VERSION_MADEBY;
    file_info.comment = comment;
    if (file_info.comment != NULL)
        file_info.comment_size = (uint16_t)strlen(file_info.comment);
    file_info.flag = (uint16_t)0;
    file_info.zip64 = MZ_ZIP64_DISABLE;

    return mz_zip_entry_write_open(compat->handle, &file_info, (int16_t)level, 0, NULL);

}

int ZEXPORT zipWriteInFileInZip(zipFile file, const void* buf, unsigned len) {
    mz_compat* compat = (mz_compat*)file;
    int32_t written = 0;
    if (compat == NULL || len >= INT32_MAX)
        return ZIP_PARAMERROR;
    written = mz_zip_entry_write(compat->handle, buf, (int32_t)len);
    if ((written < 0) || ((uint32_t)written != len))
        return ZIP_ERRNO;
    return ZIP_OK;
}

int ZEXPORT zipCloseFileInZip(zipFile file) {
    mz_compat* compat = (mz_compat*)file;
    if (compat == NULL)
        return ZIP_PARAMERROR;
    return mz_zip_entry_close(compat->handle);
}

/* Only closes the zip handle, does not close the stream */
static int zipClose2_MZ(zipFile file, const char* global_comment, uint16_t version_madeby) {
    mz_compat* compat = (mz_compat*)file;
    int32_t err = MZ_OK;

    if (compat == NULL)
        return ZIP_PARAMERROR;
    if (compat->handle == NULL)
        return err;

    if (global_comment != NULL)
        mz_zip_set_comment(compat->handle, global_comment);

    mz_zip_set_version_madeby(compat->handle, version_madeby);
    err = mz_zip_close(compat->handle);
    mz_zip_delete(&compat->handle);

    return err;
}

int ZEXPORT zipClose(zipFile file, const char* global_comment) {
    mz_compat* compat = (mz_compat*)file;
    int32_t err = MZ_OK;

    if (compat->handle != NULL)
        err = zipClose2_MZ(file, global_comment, MZ_VERSION_MADEBY);

    if (compat->stream != NULL) {
        mz_stream_close(compat->stream);
        mz_stream_delete(&compat->stream);
    }

    MZ_FREE(compat);

    return err;
}
#endif

static unzFile unzOpen_MZ(void* stream) {
    mz_compat* compat = NULL;
    int32_t err = MZ_OK;
    void* handle = NULL;

    mz_zip_create(&handle);
    err = mz_zip_open(handle, stream, MZ_OPEN_MODE_READ);

    if (err != MZ_OK) {
        mz_zip_delete(&handle);
        return NULL;
    }

    compat = (mz_compat*)MZ_ALLOC(sizeof(mz_compat));
    if (compat != NULL) {
        compat->handle = handle;
        compat->stream = stream;

        mz_zip_goto_first_entry(compat->handle);
    }
    else {
        mz_zip_delete(&handle);
    }

    return (unzFile)compat;
}

static int unzCtor_MZ(unzFile file, void* stream) {
    mz_compat* compat = (mz_compat*) file;
    int32_t err = MZ_OK;
    void* handle = NULL;

    mz_zip_create(&handle);
    err = mz_zip_open(handle, stream, MZ_OPEN_MODE_READ);

    if (err != MZ_OK) {
        mz_zip_delete(&handle);
        return UNZ_PARAMERROR;
    }

    compat->handle = handle;
    compat->stream = stream;

    mz_zip_goto_first_entry(compat->handle);
    return 0;
}

unzFile ZEXPORT unzOpen(const char* path) {
    unzFile unz = NULL;
    void* stream = NULL;

    if (mz_stream_os_create(&stream) == NULL)
        return NULL;

    if (mz_stream_open(stream, path, MZ_OPEN_MODE_READ) != MZ_OK) {
        mz_stream_delete(&stream);
        return NULL;
    }

    unz = unzOpen_MZ(stream);
    if (unz == NULL) {
        mz_stream_close(stream);
        mz_stream_delete(&stream);
        return NULL;
    }
    return unz;
}

int unzCtor(unzFile file, const char* path)
{
    void* stream = NULL;

    if (mz_stream_os_create(&stream) == NULL)
        return UNZ_PARAMERROR;

    if (mz_stream_open(stream, path, MZ_OPEN_MODE_READ) != MZ_OK) {
        mz_stream_delete(&stream);
        return UNZ_PARAMERROR;
    }

    int err = unzCtor_MZ(file, stream);
    if (err) {
        mz_stream_close(stream);
        mz_stream_delete(&stream);
        return err;
    }
    return 0;
}

int ZEXPORT unzOpenCurrentFile(unzFile file) {
    mz_compat* compat = (mz_compat*)file;
    mz_zip_file* file_info = NULL;
    int32_t err = MZ_OK;
    void* stream = NULL;

    if (compat == NULL)
        return UNZ_PARAMERROR;

    compat->total_out = 0;
    err = mz_zip_entry_read_open(compat->handle, 0, NULL);
    if (err == MZ_OK)
        err = mz_zip_entry_get_info(compat->handle, &file_info);
    if (err == MZ_OK)
        err = mz_zip_get_stream(compat->handle, &stream);
    if (err == MZ_OK)
        compat->entry_pos = mz_stream_tell(stream);
    return err;
}

int ZEXPORT unzGoToFirstFile(unzFile file) {
    mz_compat* compat = (mz_compat*)file;
    if (compat == NULL)
        return UNZ_PARAMERROR;
    compat->entry_index = 0;
    return mz_zip_goto_first_entry(compat->handle);
}

int ZEXPORT unzLocateFile(unzFile file, const char* filename, int case_sensitive) {
    mz_compat* compat = (mz_compat*)file;
    mz_zip_file* file_info = NULL;
    uint64_t preserve_index = 0;
    int32_t err = MZ_OK;
    int32_t result = 0;

    if (compat == NULL)
        return UNZ_PARAMERROR;

    preserve_index = compat->entry_index;

    err = mz_zip_goto_first_entry(compat->handle);
    while (err == MZ_OK) {
        err = mz_zip_entry_get_info(compat->handle, &file_info);
        if (err != MZ_OK)
            break;

        result = mz_path_compare_wc(filename, file_info->filename, !case_sensitive);

        if (result == 0)
            return MZ_OK;

        err = mz_zip_goto_next_entry(compat->handle);
    }

    compat->entry_index = preserve_index;
    return err;
}

static int unzClose_MZ(unzFile file) {
    mz_compat* compat = (mz_compat*)file;
    int32_t err = MZ_OK;

    if (compat == NULL)
        return UNZ_PARAMERROR;

    err = mz_zip_close(compat->handle);
    mz_zip_delete(&compat->handle);

    return err;
}

int ZEXPORT unzClose(unzFile file) {
    mz_compat* compat = (mz_compat*)file;
    int32_t err = MZ_OK;

    if (compat == NULL)
        return UNZ_PARAMERROR;

    if (compat->handle != NULL)
        err = unzClose_MZ(file);

    if (compat->stream != NULL) {
        mz_stream_close(compat->stream);
        mz_stream_delete(&compat->stream);
    }

    MZ_FREE(compat);

    return err;
}

int ZEXPORT unzReadCurrentFile(unzFile file, void* buf, unsigned len) {
    mz_compat* compat = (mz_compat*)file;
    int32_t err = MZ_OK;
    if (compat == NULL || len >= INT32_MAX)
        return UNZ_PARAMERROR;
    err = mz_zip_entry_read(compat->handle, buf, (int32_t)len);
    if (err > 0)
        compat->total_out += (uint32_t)err;
    return err;
}

int ZEXPORT unzCloseCurrentFile(unzFile file) {
    mz_compat* compat = (mz_compat*)file;
    int32_t err = MZ_OK;
    if (compat == NULL)
        return UNZ_PARAMERROR;
    err = mz_zip_entry_close(compat->handle);
    return err;
}

#if defined(_MSC_VER) || defined(__MINGW32__)
#  define localtime_r(t1,t2) (localtime_s(t2,t1) == 0 ? t1 : NULL)
#endif

static int32_t zip_time_t_to_tm(time_t unix_time, tm_unz* ptm) {
    struct tm ltm;
    if (ptm == NULL)
        return MZ_PARAM_ERROR;
    if (localtime_r(&unix_time, &ltm) == NULL) { /* Returns a 1900-based year */
        /* Invalid date stored, so don't return it */
        memset(ptm, 0, sizeof(struct tm));
        return MZ_INTERNAL_ERROR;
    }
    memcpy(ptm, &ltm, sizeof(struct tm));
    return MZ_OK;
}

int ZEXPORT unzGetCurrentFileInfo(unzFile file, unz_file_info* pfile_info, char* filename, uLong filename_size, void* extrafield, uLong extrafield_size, char* comment, uLong comment_size) {
    mz_compat* compat = (mz_compat*)file;
    mz_zip_file* file_info = NULL;
    uint16_t bytes_to_copy = 0;
    int32_t err = MZ_OK;

    if (compat == NULL)
        return UNZ_PARAMERROR;

    err = mz_zip_entry_get_info(compat->handle, &file_info);
    if (err != MZ_OK)
        return err;

    if (pfile_info != NULL) {
        pfile_info->version = file_info->version_madeby;
        pfile_info->version_needed = file_info->version_needed;
        pfile_info->flag = file_info->flag;
        pfile_info->compression_method = file_info->compression_method;
        pfile_info->dosDate = mz_zip_time_t_to_dos_date(file_info->modified_date);
        zip_time_t_to_tm(file_info->modified_date, &pfile_info->tmu_date);
        pfile_info->tmu_date.tm_year += 1900;
        pfile_info->crc = file_info->crc;

        pfile_info->size_filename = file_info->filename_size;
        pfile_info->size_file_extra = file_info->extrafield_size;
        pfile_info->size_file_comment = file_info->comment_size;

        pfile_info->disk_num_start = (uint16_t)file_info->disk_number;
        pfile_info->internal_fa = file_info->internal_fa;
        pfile_info->external_fa = file_info->external_fa;

        pfile_info->compressed_size = (uint32_t)file_info->compressed_size;
        pfile_info->uncompressed_size = (uint32_t)file_info->uncompressed_size;
    }
    if (filename_size > 0 && filename != NULL && file_info->filename != NULL) {
        bytes_to_copy = (uint16_t)filename_size;
        if (bytes_to_copy > file_info->filename_size)
            bytes_to_copy = file_info->filename_size;
        memcpy(filename, file_info->filename, bytes_to_copy);
        if (bytes_to_copy < filename_size)
            filename[bytes_to_copy] = 0;
    }
    if (extrafield_size > 0 && extrafield != NULL) {
        bytes_to_copy = (uint16_t)extrafield_size;
        if (bytes_to_copy > file_info->extrafield_size)
            bytes_to_copy = file_info->extrafield_size;
        memcpy(extrafield, file_info->extrafield, bytes_to_copy);
    }
    if (comment_size > 0 && comment != NULL && file_info->comment != NULL) {
        bytes_to_copy = (uint16_t)comment_size;
        if (bytes_to_copy > file_info->comment_size)
            bytes_to_copy = file_info->comment_size;
        memcpy(comment, file_info->comment, bytes_to_copy);
        if (bytes_to_copy < comment_size)
            comment[bytes_to_copy] = 0;
    }
    return err;
}

int ZEXPORT unzGoToNextFile(unzFile file) {
    mz_compat* compat = (mz_compat*)file;
    int32_t err = MZ_OK;
    if (compat == NULL)
        return UNZ_PARAMERROR;
    err = mz_zip_goto_next_entry(compat->handle);
    if (err != MZ_END_OF_LIST)
        compat->entry_index += 1;
    return err;
}
