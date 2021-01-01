#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "tk.h"

struct hash_table hash_tks;

TKmodel *tkmodel_create(void)
{
    TKmodel *tkm;
    tkm = (TKmodel *)nasm_malloc(sizeof(TKmodel));
    tkm->initP = 0;
    tkm->wdtree = NULL;
    return tkm;
}

static TKmodel *get_tkm_from_hashtbl(const char *instName)
{
    struct hash_insert hi;
    void **tkmpp;
    tkmpp = hash_find(&hash_tks, instName, &hi);
    return tkmpp == NULL ? NULL : *(TKmodel **)tkmpp;
}

bool request_initialize(const char *instName)
{
    TKmodel *tkm;
    tkm = get_tkm_from_hashtbl(instName);
    if (tkm)
        return likely_happen_p(tkm->initP);
    return false;
}

constVal *request_constVal(const char *instName, bool isDest)
{
    constVal *cVal;
    TKmodel *tkm;
    tkm = get_tkm_from_hashtbl(instName);
    if (tkm->diffSrcDest == false) {
        cVal = wdtree_select_constval(tkm->wdtree);
    } else {
        if (isDest) {
            cVal = wdtree_select_constval(tkm->wddesttree);
        } else {
            cVal = wdtree_select_constval(tkm->wdsrctree);
        }
    }
    return cVal;
}
