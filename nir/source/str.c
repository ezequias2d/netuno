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
#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <netuno/array.h>
#include <netuno/memory.h>
#include <netuno/str.h>

#include "netuno/table.h"
#include "pstring.h"
#include <stdio.h>
#include <string.h>

char *ntStringToChar(const NT_STRING *string)
{
    return ntToCharFixed(string->chars, string->length);
}

char *ntToChar(const char_t *str)
{
    const size_t len = ntStrLen(str);
    return ntToCharFixed(str, len);
}

char *ntToCharFixed(const char_t *str, size_t len)
{
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));

    size_t size = 0;
    for (const char_t *i = str; (size_t)(i - str) < len; i++)
        size += c32rtomb(NULL, *i, &ps);

    char *s = (char *)ntMalloc((size + 1) * sizeof(char));

    char *j = s;
    for (const char_t *i = str; (size_t)(i - str) < len; i++)
        j += c32rtomb(j, *i, &ps);

    s[size] = 0;
    return s;
}

char_t *ntToCharT(const char *str)
{
    const size_t len = strlen(str) + 1;
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));

    size_t size = 0;
    for (const char *i = str; *i != 0;
         i += mbrtoc32(NULL, i, len + str - i, &ps))
        size++;

    char_t *s = (char_t *)ntMalloc((size + 1) * sizeof(char_t));
    char_t *j = s;

    for (const char *i = str; *i != 0;
         i += mbrtoc32(j++, i, len + str - i, &ps))
        ;

    s[size] = 0;
    return s;
}

void show_errno(void)
{
    const char *err_info = "unknown error";
    switch (errno)
    {
    case EDOM:
        err_info = "domain error";
        break;
    case EILSEQ:
        err_info = "illegal sequence";
        break;
    case ERANGE:
        err_info = "pole or range error";
        break;
    case 0:
        err_info = "no error";
    }
    fputs(err_info, stdout);
    puts(" occurred");
}

char_t *ntToCharTFixed(const char *str, size_t len)
{
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));

    size_t size = 0;
    for (const char *i = str; (size_t)(i - str) < len;)
    {
        size++;
        size_t result = mbrtoc32(NULL, i, len + str - i, &ps);
        switch (result)
        {
        case -1:
        case -2:
        case -3:
            show_errno();
            return NULL;
        default:
            break;
        }
        i += result;
    }

    char_t *s = (char_t *)ntMalloc((size + 1) * sizeof(char_t));
    char_t *j = s;

    for (const char *i = str; (size_t)(i - str) < len;
         i += mbrtoc32(j++, i, len + str - i, &ps))
        ;

    s[size] = 0;
    return s;
}

size_t ntStrLen(const char_t *str)
{
    size_t s = 0;
    for (const char_t *i = str; *i != 0; ++i)
        s++;
    return s;
}

const char_t *ntStrRChr(const char_t *str, char_t character)
{
    const char_t *last = NULL;
    do
    {
        if (*str == character)
            last = str;
    } while (*str++);
    return last;
}

const char_t *ntStrChrFixed(const char_t *str, size_t length, char_t character)
{
    const char_t *const max = str + length;
    for (; str < max && *str != U'\0'; ++str)
        if (*str == character)
            return str;
    return max;
}

const char_t *ntStrChr(const char_t *str, char_t character)
{
    for (; *str != character; ++str)
        if (*str == '\0')
            return NULL;
    return str;
}

static bool isDigit(char_t c)
{
    return c >= '0' && c <= '9';
}

static bool isXDigit(char_t c)
{
    return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool isOctDigit(char_t c)
{
    return c >= '0' && c <= '7';
}

static size_t hex_char(const char_t *str, char_t *result)
{
    const char_t *const start = str++;
    char_t c = *str;
    char_t r = 0;
    while (isXDigit(c))
    {
        if (c >= '0' && c <= U'9')
            r = (r << 4) | (c - '0');
        else if (c >= 'a' && c <= 'f')
            r = (r << 4) | (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            r = (r << 4) | (c - 'A' + 10);
        c = *++str;
    }
    *result = r;
    return str - start;
}

static char_t oct_char(const char_t *str, char_t *result)
{
    const char_t *const start = str++;
    char_t c = *str;
    char_t r = 0;
    while (isOctDigit(c))
    {
        r = (r << 3) | (c - '0');
        c = *++str;
    }
    *result = r;
    return str - start;
}

static char_t unicode_char(const char_t *str, const uint32_t len,
                           char_t *result)
{
    const char_t *const start = str++;
    char_t r = 0;
    for (uint32_t i = 0; i < len; ++i)
    {
        const char_t c = *str++;

        if (c >= '0' && c <= '9')
            r = (r << 4) | (c - '0');
        else if (c >= 'a' && c <= 'f')
            r = (r << 4) | (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            r = (r << 4) | (c - 'A' + 10);
        else
            return 0x200000;
    }
    *result = r;
    return str - start;
}

size_t ntEscapeChar(const char_t *str, char_t *result)
{
    const char_t c = *str;
    switch (c)
    {
    case '\'':
    case '"':
    case '?':
    case '\\':
        *result = c;
        break;
    case 'a':
        *result = '\a';
        break;
    case 'b':
        *result = '\b';
        break;
    case 'f':
        *result = '\f';
        break;
    case 'n':
        *result = '\n';
        break;
    case 'r':
        *result = '\r';
        break;
    case 't':
        *result = '\t';
        break;
    case 'v':
        *result = '\v';
        break;
    case 'e':
        *result = '\x1B';
        break;
    case 'x':
        return hex_char(str, result);
    case 'u':
        return unicode_char(str, 4, result);
    case 'U':
        return unicode_char(str, 8, result);
    case '0':
        return oct_char(str, result);
    default:
        return 0;
    }

    return 1;
}

size_t ntUnescapeChar(const char_t *str, char_t *result)
{
    const char_t c = *str;
    char_t escape = ' ';
    switch (c)
    {
    case '\'':
    case '"':
    case '?':
    case '\\':
        escape = c;
        break;
    case '\a':
        escape = 'a';
        break;
    case '\b':
        escape = 'b';
        break;
    case '\f':
        escape = 'f';
        break;
    case '\n':
        escape = 'n';
        break;
    case '\r':
        escape = 'r';
        break;
    case '\t':
        escape = 't';
        break;
    case '\v':
        escape = 'v';
        break;
    case '\x1B':
        escape = 'e';
        break;
    default:
        *result = c;
        return 1;
    }
    *result = '\\';
    *(result + 1) = escape;
    return 2;
}

char_t *ntEscapeString(const char_t *str, size_t *length)
{
    const char_t *max = str + *length;
    NT_ARRAY tmp;
    ntInitArray(&tmp);

    const char_t *escape;
    do
    {
        escape = ntStrChrFixed(str, *length, '\\');

        const size_t copy = (escape - str) * sizeof(char_t);
        ntArrayAdd(&tmp, str, copy);

        if (*escape == '\\')
        {
            char_t c;
            str = escape + 1 + ntEscapeChar(escape + 1, &c);
            ntArrayAdd(&tmp, &c, sizeof(char_t));
        }
        else
            str = escape;
    } while (str < max);

    ntArrayAdd(&tmp, U"\0", sizeof(char_t));

    *length = (tmp.count / sizeof(char_t)) - 1;
    return ntRealloc(tmp.data, tmp.count);
}

char_t *ntUnescapeString(const char_t *str, size_t *length)
{
    const char_t *max = str + *length;
    NT_ARRAY tmp;
    ntInitArray(&tmp);

    while (str < max)
    {
        char_t c[2];
        const size_t s = ntUnescapeChar(str, c);
        str++;
        ntArrayAdd(&tmp, &c, s * sizeof(char_t));
    }

    ntArrayAdd(&tmp, U"\0", sizeof(char_t));

    *length = (tmp.count / sizeof(char_t)) - 1;
    return ntRealloc(tmp.data, tmp.count);
}

bool ntStrEquals(const char_t *str1, const char_t *str2)
{
    for (size_t i = 0; str1[i] != '\0' || str2[i] != '\0'; ++i)
        if (str1[i] != str2[i])
            return false;
    return true;
}

bool ntStrEqualsFixed(const char_t *str1, const size_t size1,
                      const char_t *str2, const size_t size2)
{
    if (size1 != size2)
        return false;

    for (size_t i = 0; i < size1; ++i)
        if (str1[i] != str2[i])
            return false;
    return true;
}
