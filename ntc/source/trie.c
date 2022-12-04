#include "trie.h"
#include <ctype.h>
#include <netuno/memory.h>

static bool findNode(const NT_TRIE_NODE *node, char_t character, uint32_t *result)
{
    for (uint32_t i = 0; i < node->childrenCount; ++i)
    {
        const char_t currentCharacter = node->children[i].character;
        if (toupper(currentCharacter) == toupper(character))
        {
            *result = i;
            return true;
        }
        else if (currentCharacter < character)
            *result = i;
    }
    return false;
}

NT_TRIE *ntTrieCreate(uint32_t keywordCount, const NT_KEYWORD_PAIR *keywords)
{
    NT_TRIE *trie = (NT_TRIE *)ntMalloc(sizeof(NT_TRIE));

    trie->root = (NT_TRIE_NODE *)ntMalloc(sizeof(NT_TRIE_NODE));
    *trie->root = (NT_TRIE_NODE){
        .childrenCount = 0,
    };

    for (uint32_t i = 0; i < keywordCount; ++i)
    {
        const NT_KEYWORD_PAIR *pair = &keywords[i];
        const char_t *keyword = pair->keyword;
        const uint64_t value = pair->value;
        NT_TRIE_NODE *current = trie->root;

        // insert
        for (uint32_t j = 0; keyword[j] != '\0'; ++j)
        {
            const char_t c = keyword[j];
            const bool isTerminal = keyword[j + 1] == '\0';

            if (current->childrenCount == 0)
            {
                NT_TRIE_NODE *newArray = (NT_TRIE_NODE *)ntMalloc(sizeof(NT_TRIE_NODE));
                current->childrenCount = 1;
                current->children = newArray;
                *newArray = (NT_TRIE_NODE){.character = c,
                                           .isTerminal = isTerminal,
                                           .value = value,
                                           .childrenCount = 0,
                                           .children = NULL};

                current = newArray;
                continue;
            }

            uint32_t k = 0;
            if (!findNode(current, c, &k))
            {
                // insert
                NT_TRIE_NODE *newArray =
                    (NT_TRIE_NODE *)ntMalloc(sizeof(NT_TRIE_NODE) * (current->childrenCount + 1));

                // copy [0..k-1]
                ntMemcpy(newArray, current->children, k * sizeof(NT_TRIE_NODE));

                // paste new trie node into k+1
                newArray[k] = (NT_TRIE_NODE){.character = c,
                                             .isTerminal = isTerminal,
                                             .value = value,
                                             .childrenCount = 0,
                                             .children = NULL};

                // copy [k..] into [k+1..]
                ntMemcpy(newArray + k + 1, current->children + k,
                         (current->childrenCount - k) * sizeof(NT_TRIE_NODE));

                ntFree(current->children);
                current->children = newArray;
                current->childrenCount++;
            }

            current = &current->children[k];
        }
    }

    return trie;
}

static void destroyNodes(NT_TRIE_NODE *node)
{
    for (size_t i = 0; i < node->childrenCount; ++i)
        destroyNodes(&node->children[i]);
    ntFree(node->children);
    node->children = NULL;
    node->childrenCount = 0;
}

void ntTrieDestroy(NT_TRIE *trie)
{
    NT_TRIE_NODE *current = trie->root;
    for (size_t i = 0; i < current->childrenCount; ++i)
        destroyNodes(&current->children[i]);
    ntFree(current->children);
    ntFree(current);
    ntFree(trie);
}

bool ntTrieTryGet(const NT_TRIE *trie, const char_t *keyword, uint32_t keywordLength,
                  uint32_t *result)
{
    NT_TRIE_NODE *current = trie->root;

    for (uint32_t i = 0; i < keywordLength; ++i)
    {
        const char_t c = keyword[i];

        uint32_t j = 0;
        if (findNode(current, c, &j))
        {
            current = &current->children[j];
        }
        else
            return false;
    }

    if (current->isTerminal)
        *result = current->value;

    return current->isTerminal;
}
