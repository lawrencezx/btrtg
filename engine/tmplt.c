#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "generator.h"
#include "tmplt.h"
#include "x86pg.h"

tmplt_struct tmpltm;

void init_blk_struct(blk_struct *blk)
{
    blk->parent = NULL;
    blk->type = -1;
    blk->xfrName = NULL;
    blk->times = 0;
    blk->trv_state = NULL;
    blk->blks = g_array_new(FALSE, FALSE, sizeof(void *));
    blk->vars = g_array_new(FALSE, FALSE, sizeof(blk_var));
}

void init_blk_var(blk_var *var)
{
    var->valid = false;
    var->name = NULL;
    var->opndflags = 0;
    var->asm_var = NULL;
}

void init_trv_state(struct trv_state *trv_state)
{
    trv_state->wdtrees = g_array_new(FALSE, FALSE, sizeof(WDTree *));
    trv_state->constVals = g_array_new(FALSE, FALSE, sizeof(constVal *));
}

blk_var *blk_search_var(blk_struct *blk, const char *var_name)
{
    if (blk == NULL)
        return NULL;
    for (guint i = 0; i < blk->vars->len; i++) {
        if (strcmp(g_array_index(blk->vars, blk_var, i).name, var_name) == 0) {
            return &g_array_index(blk->vars, blk_var, i);
        }
    }
    return blk_search_var(blk->parent, var_name);
}

static void init_elem_gen_state(void)
{
    stat_unlock_reg(LOCK_REG_CASE_MEM);
}

static void walkSeqBlk(blk_struct *blk);
static void walkSelBlk(blk_struct *blk);
static void walkXfrBlk(blk_struct *blk);
static void walkRptBlk(blk_struct *blk);
static void walkTrvBlk(blk_struct *blk);
static void walkElemBlk(blk_struct *blk);

static void (*walkBlkFuncs[])(blk_struct *) =
{
    walkSeqBlk,
    walkSelBlk,
    walkXfrBlk,
    walkRptBlk,
    walkTrvBlk,
    walkElemBlk
};

static void walkSeqBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);

    for (guint i = 0; i < blk->blks->len; i++) {
        blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
        walkBlkFuncs[subblk->type](subblk);
    }
}

static void walkSelBlk(blk_struct *blk)
{
    WDTree *wdtree;
    constVal *cVal;
    insn_seed seed;
    insn inst;

    stat_set_curr_blk(blk);
    init_elem_gen_state();

    wdtree = g_array_index(blk->blks, WDTree *, 0);
    cVal = wdtree_select_constval(wdtree);
    
    create_insn_seed(&seed, cVal->instName);
    one_insn_gen(&seed, &inst);
}

static void walkXfrBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);
    /* TODO */
}

static void walkRptBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);

    for (int k = 0; k < blk->times; k++) {
        for (guint i = 0; i < blk->blks->len; i++) {
            blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
            walkBlkFuncs[subblk->type](subblk);
        }
    }
}

static void pre_order_traverse_trv_state(blk_struct *blk, WDTree *tree, int num)
{
    struct trv_state *trv_state = blk->trv_state;

    if (tree == NULL || tree->size == 0)
        return;

    for (int i = 0; i < tree->size; i++) {
        if (tree->isleaf) {
            constVal *const_val = &g_array_index(tree->consts, constVal, i);
            g_array_append_val(trv_state->constVals, const_val);
            if ((size_t)num + 1 == trv_state->wdtrees->len) {
                for (guint i = 0; i < blk->blks->len; i++) {
                    blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
                    walkBlkFuncs[subblk->type](subblk);
                }
            } else
                pre_order_traverse_trv_state(blk, g_array_index(trv_state->wdtrees, 
                            WDTree *, num + 1), num + 1);
            g_array_remove_index(trv_state->constVals, num);
        } else
            pre_order_traverse_trv_state(blk, g_array_index(tree->subtrees,
                        WDTree *, i), num);
    }
}

static void walkTrvBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);
    
    GArray *wdtrees = blk->trv_state->wdtrees;
    if (wdtrees->len != 0)
        pre_order_traverse_trv_state(blk, g_array_index(wdtrees, WDTree *, 0), 0);
}

/* walk element
 */
static void walkGElem(elem_struct *g_e);
static void walkCElem(elem_struct *p_e);
static void walkIElem(elem_struct *i_e);

static void (*walkElemFuncs[])(elem_struct *elem) =
{
    NULL, /* variable element */
    walkGElem,
    walkCElem,
    walkIElem
};

static void walkGElem(elem_struct *g_e)
{
    constVal *cVal;
    insn_seed seed;
    insn inst;

    stat_set_need_init(likely_happen_p(g_e->inip));
    init_elem_gen_state();

    cVal = wdtree_select_constval(g_e->wdtree);
    
    create_insn_seed(&seed, cVal->instName);
    one_insn_gen(&seed, &inst);
    stat_set_need_init(false);
}

static void walkCElem(elem_struct *c_e)
{
    char call_check_function[128];
    char *checkType = c_e->checkType;

    if (checkType == NULL)
        nasm_fatal("no check point\n");
    if (checkType[0] == '@') {
        blk_var *var = blk_search_var(stat_get_curr_blk(), checkType + 1);
        if (!var->valid)
            nasm_fatal("checking value: %s has not been initialized", checkType);
        checkType = var->asm_var;
    }
    sprintf(call_check_function, "  check %s", checkType);
    one_insn_gen_ctrl(call_check_function, INSERT_AFTER);
}

static void walkIElem(elem_struct *i_e)
{
    stat_set_need_init(likely_happen_p(i_e->inip));
    init_elem_gen_state();
    stat_set_constVals(i_e->constVals);

    one_insn_gen_const(i_e->inst);

    stat_set_need_init(false);
    stat_set_constVals(NULL);
}

static void walkElemBlk(blk_struct *blk)
{
    stat_set_opcode(I_none);
    elem_struct *elem = g_array_index(blk->blks, elem_struct *, 0);
    walkElemFuncs[elem->type](elem);
}

void walk_tmplt(void)
{
    blk_struct *blk = (blk_struct *)tmpltm.blk;
    walkBlkFuncs[blk->type](blk);
}


static void elem_free(elem_struct *elem)
{
    if (elem->type == I_ELEM)
        free(elem->inst);
    free(elem);
}

static void blk_free(blk_struct *blk)
{
    if (blk == NULL)
        return;
    for (guint i = 0; i < blk->vars->len; i++) {
        blk_var *var = &g_array_index(blk->vars, blk_var, i);
        free(var->name);
        free(var->asm_var);
    }
    if (blk->type == SEL_BLK) {
    } else if (blk->type == ELEM_BLK) {
        elem_free(g_array_index(blk->blks, elem_struct *, 0));
    } else {
        for (guint i = 0; i < blk->blks->len; i++) {
            blk_free(g_array_index(blk->blks, void *, i));
        }
    }
    free(blk->xfrName);
    g_array_free(blk->vars, true);
    g_array_free(blk->blks, true);
    free(blk);
}

void tmplt_clear(tmplt_struct *tmpltm)
{
    blk_free((blk_struct *)tmpltm->blk);
}

void tmplt_free(tmplt_struct *tmpltm)
{
    tmplt_clear(tmpltm);
    free(tmpltm);
}
