#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "model.h"

struct hash_table hash_wdtrees;
struct hash_table hash_tks;
struct hash_table hash_templates;

Tmpltmodel tmpltm;

TKmodel *tkmodel_create(void)
{
    TKmodel *tkm;
    tkm = (TKmodel *)nasm_malloc(sizeof(TKmodel));
    tkm->initP = 0;
    tkm->wdtree = NULL;
    return tkm;
}

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

static int select_childi(int *weights, int len)
{
    if (weights == NULL) {
        return nasm_random32(len);
    }
    static const int total_weight = 100;
    int chi, accWeight = 0;
    int random_chi = nasm_random32(total_weight);
    for (chi = 0; chi < len; chi++) {
        accWeight += weights[chi];
        if (random_chi < accWeight)
            break;
    }
    return chi;
}

static constVal *wdtree_select_constval(WDTree *tree)
{
    int chi;

    if (tree == NULL || tree->size == 0) {
        return NULL;
    }
    chi = select_childi(tree->weights, tree->size);
    if (tree->isleaf) {
        return &tree->consts[chi];
    }
    return wdtree_select_constval(tree->children[chi]);
}

static TKmodel *get_tkm_from_hashtbl(const char *instName)
{
    struct hash_insert hi;
    return *(TKmodel **)hash_find(&hash_tks, instName, &hi);
}

bool request_initialize(const char *instName)
{
    TKmodel *tkm;
    tkm = get_tkm_from_hashtbl(instName);
    if (tkm)
        return likely_happen_p(tkm->initP);
    return false;
}

constVal *request_constVal(const char *instName, bool isSrc)
{
    constVal *cVal;
    TKmodel *tkm;
    tkm = get_tkm_from_hashtbl(instName);
    if (tkm->diffSrcDest == false) {
        cVal = wdtree_select_constval(tkm->wdtree);
    } else {
        if (isSrc) {
            cVal = wdtree_select_constval(tkm->wdsrctree);
        } else {
            cVal = wdtree_select_constval(tkm->wddesttree);
        }
    }
    return cVal;
}

char *select_inst(void)
{
    constVal *cVal;
    cVal = wdtree_select_constval(tmpltm.wdtree);
    return cVal->instName;
}
