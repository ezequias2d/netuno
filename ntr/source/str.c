#include <netuno/memory.h>
#include <netuno/str.h>
#include <string.h>

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
    for (const char_t *i = str; i - str < len; i++)
        size += c32rtomb(NULL, *i, &ps);

    char *s = (char *)ntMalloc((size + 1) * sizeof(char));

    char *j = s;
    for (const char_t *i = str; i - str < len; i++)
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
    for (const char *i = str; *i != 0; i += mbrtoc32(NULL, i, len + str - i, &ps))
        size++;

    char_t *s = (char_t *)ntMalloc((size + 1) * sizeof(char_t));
    char_t *j = s;

    for (const char *i = str; *i != 0; i += mbrtoc32(j++, i, len + str - i, &ps))
        ;

    s[size] = 0;
    return s;
}

char_t *ntToCharTFixed(const char *str, size_t len)
{
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));

    size_t size = 0;
    for (const char *i = str; i - str < len; i += mbrtoc32(NULL, i, len + str - i, &ps))
        size++;

    char_t *s = (char_t *)ntMalloc((size + 1) * sizeof(char_t));
    char_t *j = s;

    for (const char *i = str; i - str < len; i += mbrtoc32(j++, i, len + str - i, &ps))
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
