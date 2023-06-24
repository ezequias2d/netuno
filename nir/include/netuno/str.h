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
#ifndef STR_H
#define STR_H

#include <netuno/common.h>

NT_HANDLE(NT_STRING)

// NIR_STRING nirCopyString(const NIR_STRING *str);
// void nirUpdateString(NIR_STRING *string, const NIR_STRING *newValue);

bool ntStrEquals(const char_t *str1, const char_t *str2);
bool ntStrEqualsFixed(const char_t *str1, const size_t size1,
                      const char_t *str2, const size_t size2);

char *ntStringToChar(const NT_STRING *string);
char *ntToChar(const char_t *str);
char *ntToCharFixed(const char_t *str, size_t len);
char_t *ntToCharT(const char *str);
char_t *ntToCharTFixed(const char *str, size_t len);
size_t ntStrLen(const char_t *str);
const char_t *ntStrRChr(const char_t *str, char_t character);
const char_t *ntStrChrFixed(const char_t *str, size_t length, char_t character);
const char_t *ntStrChr(const char_t *str, char_t character);
size_t ntEscapeChar(const char_t *str, char_t *result);
size_t ntUnescapeChar(const char_t *str, char_t *result);
char_t *ntEscapeString(const char_t *str, size_t *length);
char_t *ntUnescapeString(const char_t *str, size_t *length);

#endif
