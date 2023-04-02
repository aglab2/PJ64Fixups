#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               4
#define VERSION_MINOR               0
#define VERSION_REVISION            0
#define VERSION_BUILD               6090
#define VERSION_PREFIX              "Dev-"
#define VERSION_BUILD_YEAR          2023

#define GIT_REVISION                "96792b18c87139dd3c8d2a5096dd3a8e28da6b14"
#define GIT_REVISION_SHORT          "96792b18"
#define GIT_DIRTY                   "Dirty"
#define GIT_VERSION                 "96792b18-Dirty"

#define VER_FILE_DESCRIPTION_STR    "Project64-RSP"
#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD
#define VER_FILE_VERSION_STR        VERSION_PREFIX STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_MINOR)    \
                                    "." STRINGIZE(VERSION_REVISION) \
                                    "." STRINGIZE(VERSION_BUILD)    \
                                    "-" GIT_VERSION

#define VER_PRODUCTNAME_STR         "Project64-RSP"
#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_ORIGINAL_FILENAME_STR   VER_PRODUCTNAME_STR ".dll"
#define VER_INTERNAL_NAME_STR       VER_PRODUCTNAME_STR
#define VER_COPYRIGHT_STR           "Copyright (C) " STRINGIZE(VERSION_BUILD_YEAR)

#ifdef _DEBUG
#define VER_VER_DEBUG             VS_FF_DEBUG
#else
#define VER_VER_DEBUG             0
#endif

#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               VER_VER_DEBUG
#define VER_FILETYPE                VFT_DLL

