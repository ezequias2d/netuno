#ifndef STR_H
#define STR_H

#include "common.h"

char *ntToChar(const char_t *str);
char *ntToCharFixed(const char_t *str, size_t len);
char_t *ntToCharT(const char *str);
char_t *ntToCharTFixed(const char *str, size_t len);
size_t ntStrLen(const char_t *str);
const char_t *ntStrRChr(const char_t *str, char_t character);
const char_t *ntStrChr(const char_t *str, char_t character);

#endif
