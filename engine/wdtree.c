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
    tree->weights = g_array_new(FALSE, FALSE, sizeof(int));
    tree->subtrees = g_array_new(FALSE, FALSE, sizeof(WDTree *));
    tree->consts = g_array_new(FALSE, FALSE, sizeof(constVal));

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
    g_array_free(tree->weights, true);
    if (tree->isleaf == true) {
        for (int i = 0; i < tree->size; i++) {
            constVal_clear(&g_array_index(tree->consts, constVal, i));
        }
        g_array_free(tree->consts, true);
    } else {
        g_array_free(tree->subtrees, true);
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

static int select_subtree(GArray *weights, int len)
{
    if (weights->len == 0) {
        return nasm_random32(len);
    }
    static const int total_weight = 100;
    int subtree, accWeight = 0;
    int random_chi = nasm_random32(total_weight);
    for (subtree = 0; subtree < len; subtree++) {
        accWeight += g_array_index(weights, int, subtree);
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
        return &g_array_index(tree->consts, constVal, subtree);
    }
    return wdtree_select_constval(g_array_index(tree->subtrees, WDTree *, subtree));
}

