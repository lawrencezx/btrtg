#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "wdtree.h"

struct hash_table hash_wdtrees;

static WDTree **wdtrees_tempstorage = NULL;
static int wdtrees_tempsize = 0, wdtrees_templen = 0;
#define WDTREES_TEMP_DELTA 128

WDTree *wdtree_create(void)
{
    WDTree *tree;
    tree = (WDTree *)nasm_malloc(sizeof(WDTree));
    tree->isleaf = false;
    tree->size = 0;
    tree->weights = NULL;
    tree->children = NULL;
    tree->consts = NULL;

    if (wdtrees_templen >= wdtrees_tempsize) {
        wdtrees_tempsize += WDTREES_TEMP_DELTA;
        wdtrees_tempstorage = nasm_realloc(wdtrees_tempstorage,
                                           wdtrees_tempsize *
                                           sizeof(WDTree *));
    }
    wdtrees_tempstorage[wdtrees_templen++] = tree;

    return tree;
}

static void constVal_clear(constVal *cVal)
{
    if (cVal->type == CONST_INSN) {
        free(cVal->instName);
    }
}

void wdtree_clear(WDTree *tree)
{
    free(tree->weights);
    if (tree->isleaf == true) {
        for (int i = 0; i < tree->size; i++) {
            constVal_clear(&tree->consts[i]);
        }
        free(tree->consts);
    } else {
        free(tree->children);
    }
}

static void hash_wdtrees_clear(void)
{
    struct hash_iterator it;
    const struct hash_node *np;

    hash_for_each(&hash_wdtrees, it, np) {
        nasm_free((void *)np->key);
    }

    hash_free(&hash_wdtrees);
}

void wdtrees_free_all(void)
{
    for (int i = 0; i < wdtrees_templen; i++) {
        wdtree_clear(wdtrees_tempstorage[i]);
        free(wdtrees_tempstorage[i]);
    }
    free(wdtrees_tempstorage);
    wdtrees_tempstorage = NULL;

    hash_wdtrees_clear();
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

