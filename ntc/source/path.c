#include "path.h"
#include <netuno/memory.h>
#include <netuno/str.h>

char_t *ntPathFilename(const char_t *path, bool withExtension)
{
    const char_t *filenameStart = ntStrRChr(path, U'/') + 1;

    size_t length;

    if (!withExtension)
        length = ntStrLen(filenameStart);
    else
    {
        const char_t *dot = ntStrChr(filenameStart, U'.');
        length = dot - filenameStart;
    }

    size_t size = (length + 1) * sizeof(char_t);
    char_t *filename = (char_t *)ntMalloc(size);
    ntMemcpy(filename, filenameStart, size);
    filename[length] = U'\0';

    return filename;
}
