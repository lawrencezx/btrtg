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

static void walkTrvBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);

    for (guint i = 0; i < blk->blks->len; i++) {
        blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
        walkBlkFuncs[subblk->type](subblk);
    }
}

/* walk element
 */
static void walkGElem(elem_struct *g_e);
static void walkPElem(elem_struct *p_e);
static void walkIElem(elem_struct *i_e);

static void (*walkElemFuncs[])(elem_struct *elem) =
{
    walkGElem,
    walkPElem,
    walkIElem
};

static void walkGElem(elem_struct *g_e)
{
    constVal *cVal;
    insn_seed seed;
    insn inst;

    stat_set_need_init(likely_happen_p(g_e->inip));

    cVal = wdtree_select_constval(g_e->wdtree);
    
    create_insn_seed(&seed, cVal->instName);
    one_insn_gen(&seed, &inst);
    stat_set_need_init(false);
}

static void gen_call_print_x86_state(void)
{
    one_insn_gen_const("pushad");
    one_insn_gen_const("pushfd");
    one_insn_gen_const("push cs");
    one_insn_gen_const("push ss");
    one_insn_gen_const("push ds");
    one_insn_gen_const("push es");
    one_insn_gen_const("push fs");
    one_insn_gen_const("push gs");
    one_insn_gen_ctrl("  call print_x86_state", INSERT_AFTER);
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("popfd");
    one_insn_gen_const("popad");
    one_insn_gen_const("popa");
}
static void gen_call_print_x87_state(void)
{
    one_insn_gen_const("sub esp, 0x200");
    one_insn_gen_const("fxsave [esp]");
    one_insn_gen_ctrl("  call print_x87_state", INSERT_AFTER);
    one_insn_gen_const("add esp, 0x200");
    one_insn_gen_const("fxrstor [esp]");
}

static void gen_call_print_all_state(void)
{
    one_insn_gen_const("pushad");
    one_insn_gen_const("pushfd");
    one_insn_gen_const("push cs");
    one_insn_gen_const("push ss");
    one_insn_gen_const("push ds");
    one_insn_gen_const("push es");
    one_insn_gen_const("push fs");
    one_insn_gen_const("push gs");
    one_insn_gen_const("sub esp, 0x200");
    one_insn_gen_const("fxsave [esp]");
    one_insn_gen_ctrl("  call print_all_state", INSERT_AFTER);
    one_insn_gen_const("add esp, 0x200");
    one_insn_gen_const("fxrstor [esp]");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("pop eax");
    one_insn_gen_const("popfd");
    one_insn_gen_const("popad");
    one_insn_gen_const("popa");
}

static void walkPElem(elem_struct *p_e)
{
    if (p_e->pType == X86_STATE) {
        gen_call_print_x86_state();
    } else if (p_e->pType == X87_STATE) {
        gen_call_print_x87_state();
    } else if (p_e->pType == ALL_STATE) {
        gen_call_print_all_state();
    } else {
        nasm_fatal("Unsupported print type: %d", p_e->pType);
    }
}

static void walkIElem(elem_struct *i_e)
{
    stat_set_need_init(likely_happen_p(i_e->inip));
    one_insn_gen_const(i_e->inst);
    stat_set_need_init(false);
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
    if (elem->type == INSN_ELEM)
        free(elem->inst);
    free(elem);
}

static void blk_free(blk_struct *blk)
{
    if (blk == NULL)
        return;
    if (blk->type == SEL_BLK) {
    } else if (blk->type == ELEM_BLK) {
        elem_free(g_array_index(blk->blks, elem_struct *, 0));
    } else {
        for (guint i = 0; i < blk->blks->len; i++) {
            blk_free(g_array_index(blk->blks, void *, i));
        }
    }
    free(blk->xfrName);
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
