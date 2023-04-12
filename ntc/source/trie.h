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
#ifndef TRIE_H
#define TRIE_H

#include <netuno/ntc.h>

typedef struct _NT_TRIE_NODE
{
    uint32_t childrenCount;
    struct _NT_TRIE_NODE *children;
    char_t character;
    bool isTerminal;
    uint64_t value;
} NT_TRIE_NODE;

typedef struct _NT_TRIE
{
    NT_TRIE_NODE *root;
} NT_TRIE;

typedef struct _NT_KEYWORD_PAIR
{
    const char_t *keyword;
    uint32_t value;
} NT_KEYWORD_PAIR;

NT_TRIE *ntTrieCreate(uint32_t keywordCount, const NT_KEYWORD_PAIR *keywords);
void ntTrieDestroy(NT_TRIE *trie);
bool ntTrieTryGet(const NT_TRIE *trie, const char_t *keyword, uint32_t keywordLength,
                  uint32_t *result);

#endif
