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
    tkm->wdtree = NULL;
    return tkm;
}

void tks_free_all(void)
{
    hash_free_all(&hash_tks, true);
}

static TKmodel *get_tkm_from_hashtbl(const char *instName)
{
    struct hash_insert hi;
    void **tkmpp;
    tkmpp = hash_find(&hash_tks, instName, &hi);
    return tkmpp == NULL ? NULL : *(TKmodel **)tkmpp;
}

void create_trv_state(char *asm_inst, struct trv_state *trv_state)
{
    char inst_name[128];
    int i = 0;
    while (asm_inst[i] != ' ' && asm_inst[i] != '\n') {
        inst_name[i] = asm_inst[i];
        i++;
    }
    inst_name[i] = '\0';
    TKmodel *tkm = get_tkm_from_hashtbl(inst_name);
    g_array_append_val(trv_state->wdtrees, tkm->wdtree);
    while (asm_inst[i] != '\0' && asm_inst[i] != '\n')
        if (asm_inst[i++] == ',')
            g_array_append_val(trv_state->wdtrees, tkm->wdtree);
}

struct const_node *request_val_node(const char *instName, bool isDest)
{
    struct const_node *val_node;
    TKmodel *tkm;
    tkm = get_tkm_from_hashtbl(instName);
//if (tkm->diffSrcDest == false) {
        val_node = wdtree_select_leaf_node(tkm->wdtree);
//    } else {
//        if (isDest) {
//            cVal = wdtree_select_leaf_node(tkm->wddesttree);
//        } else {
//            cVal = wdtree_select_leaf_node(tkm->wdsrctree);
//        }
//    }
    return val_node;
}
