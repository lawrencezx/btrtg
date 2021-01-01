#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "generator.h"
#include "tmplt.h"

struct tmplt tmpltm;

void init_blk_struct(blk_struct *blk)
{
    blk->type = -1;
    blk->num = 0;
    blk->xfrName = NULL;
    blk->times = 0;
    blk->blks = NULL;
}

static void walkSeqBlk(blk_struct *blk);
static void walkSelBlk(blk_struct *blk);
static void walkXfrBlk(blk_struct *blk);
static void walkRptBlk(blk_struct *blk);
static void walkElemBlk(blk_struct *blk);

static void (*walkBlkFuncs[])(blk_struct *) =
{
    walkSeqBlk,
    walkSelBlk,
    walkXfrBlk,
    walkRptBlk,
    walkElemBlk
};

static void walkSeqBlk(blk_struct *blk)
{
    for (int i = 0; i < blk->num; i++) {
        blk_struct *subblk = blk->blks[i];
        walkBlkFuncs[subblk->type](subblk);
    }
}

static void walkSelBlk(blk_struct *blk)
{
    WDTree *wdtree;
    constVal *cVal;
    insn_seed seed;
    insn inst;

    wdtree = (WDTree *)blk->blks[0];
    cVal = wdtree_select_constval(wdtree);
    
    create_insn_seed(&seed, cVal->instName);
    one_insn_gen(&seed, &inst);
}

static void walkXfrBlk(blk_struct *blk)
{
    /* TODO */
}

static void walkRptBlk(blk_struct *blk)
{
    for (int k = 0; k < blk->times; k++) {
        for (int i = 0; i < blk->num; i++) {
            blk_struct *subblk = blk->blks[i];
            walkBlkFuncs[subblk->type](subblk);
        }
    }
}

/* walk element
 */
static void walkIsetElem(elem_struct *iset_e);
static void walkPrintElem(elem_struct *print_e);
static void walkIElem(elem_struct *i_e);

static void (*walkElemFuncs[])(elem_struct *elem) =
{
    walkIsetElem,
    walkPrintElem,
    walkIElem
};

static void walkIsetElem(elem_struct *iset_e)
{
    constVal *cVal;
    insn_seed seed;
    insn inst;

    cVal = wdtree_select_constval(iset_e->wdtree);
    
    create_insn_seed(&seed, cVal->instName);
    one_insn_gen(&seed, &inst);
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
    one_insn_gen_const("call print_x86_state");
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
    one_insn_gen_const("call print_x87_state");
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
    one_insn_gen_const("call print_all_state");
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

static void walkPrintElem(elem_struct *print_e)
{
    if (print_e->printType == X86_STATE) {
        gen_call_print_x86_state();
    } else if (print_e->printType == X87_STATE) {
        gen_call_print_x87_state();
    } else if (print_e->printType == ALL_STATE) {
        gen_call_print_all_state();
    } else {
        nasm_fatal("Unsupported print type: %d", print_e->printType);
    }
}

static void walkIElem(elem_struct *i_e)
{
    /* TODO */
}

static void walkElemBlk(blk_struct *blk)
{
    elem_struct *elem = (elem_struct *)blk->blks[0];
    walkElemFuncs[elem->type](elem);
}

void walk_tmplt(void)
{
    blk_struct *blk = (blk_struct *)tmpltm.blk;
    walkBlkFuncs[blk->type](blk);
}
