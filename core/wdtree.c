#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "wdtree.h"

struct hash_table hash_wdtrees;

WDTree *wdtree_create(void)
{
    WDTree *tree;
    tree = (WDTree *)nasm_malloc(sizeof(WDTree));
    tree->isleaf = false;
    tree->size = 0;
    tree->weights = NULL;
    tree->children = NULL;
    tree->consts = NULL;
    return tree;
}

void wdtree_clear(WDTree *tree)
{
    if (tree->isleaf == true) {
        free(tree->consts);
        return;
    }
    for (int i = 0; i < tree->size; i++) {
        wdtree_clear(tree->children[i]);
    }
    free(tree->children);
}

static int select_subtree(int *weights, int len)
{
    if (weights == NULL) {
        return nasm_random32(len);
    }
    static const int total_weight = 100;
    int subtree, accWeight = 0;
    int random_chi = nasm_random32(total_weight);
    for (subtree = 0; subtree < len; subtree++) {
        accWeight += weights[subtree];
        if (random_chi < accWeight)
            break;
    }
    return subtree;
}

constVal *wdtree_select_constval(WDTree *tree)
{
    int subtree;

    if (tree == NULL || tree->size == 0) {
        return NULL;
    }
    subtree = select_subtree(tree->weights, tree->size);
    if (tree->isleaf) {
        return &tree->consts[subtree];
    }
    return wdtree_select_constval(tree->children[subtree]);
}

