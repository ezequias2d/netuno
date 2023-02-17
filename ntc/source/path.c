/*
MIT License

Copyright (c) 2022 Ezequias Silva <ezequiasmoises@gmail.com> and the Netuno
contributors. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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
