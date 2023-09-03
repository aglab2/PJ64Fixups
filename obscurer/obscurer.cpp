#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>

#include "zlib-ng.h"

// #define USE_ENCRYPTION

#ifdef USE_ENCRYPTION
const uint64_t Magic = 0b0100100111011011101010101010111001110101110001000100001100110000;

static constexpr uint64_t rotl(uint64_t x, unsigned int n)
{
    const unsigned int mask = (CHAR_BIT * sizeof(x) - 1);
    n &= mask;
    return (x << n) | (x >> ((-n) & mask));
}
#endif

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("%s INPUT_PATH OUTPUT_PATH", argv[0]);
        return -1;
    }

    const char* pathToObscure = argv[1];
    const char *pathResult    = argv[2];

    int fd = _open(pathToObscure, _O_RDONLY | _O_BINARY);
    if (-1 == fd)
    {
        printf("Failed to open file!\n");
        return -1;
    }

    struct _stat sb;
    if (_fstat(fd, &sb))
    {
        printf("Failed to stat file!\n");
        return -1;
    }
    if (sb.st_size > 1024 * 1024)
    {
        printf("File is too large!\n");
        return -1;
    }
    size_t srcSize  = sb.st_size;
    uint8_t *srcBuf = (uint8_t *)malloc(srcSize);
    if (_read(fd, srcBuf, srcSize) != srcSize)
    {
        printf("Failed to read the whole file!\n");
        return -1;
    }
    _close(fd);

    size_t dstSize = zng_compressBound(sb.st_size);
    uint8_t *dstBuf = (uint8_t *)malloc(dstSize);
    int32_t zerr = zng_compress2(dstBuf, &dstSize, srcBuf, srcSize, Z_BEST_COMPRESSION);
    if (zerr != Z_OK)
    {
        printf("Failed to compress buffer!\n");
        return -1;
    }

    free(srcBuf);

#ifdef USE_ENCRYPTION
    uint64_t *values = (uint64_t *)dstBuf;
    uint64_t valuesCount = (dstSize / sizeof(uint64_t)) + 1;
    for (int i = 0; i < valuesCount; i++)
    {
        values[i] ^= rotl(Magic * 1125899906842597ULL, i * 2654435761U);
    }
#endif

    fd = _open(pathResult, _O_WRONLY | _O_TRUNC | _O_BINARY | _O_CREAT, 0666);
    if (-1 == fd)
    {
        printf("Failed to open output file!\n");
        return -1;
    }
    _write(fd, dstBuf, dstSize);
    _close(fd);

    free(dstBuf);

    return 0;
}
