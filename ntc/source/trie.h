#ifndef TRIE_H
#define TRIE_H

#include <netuno/common.h>

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
