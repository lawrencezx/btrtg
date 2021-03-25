#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "seed.h"
#include "tmplt.h"
#include "x86pg.h"
#include "reg.h"

struct X86PGState X86PGState;

void init_x86pgstate(void)
{
    init_text_sec(&X86PGState.text_sec);
    init_data_sec(&X86PGState.data_sec);
    X86PGState.labeli = 0;
    X86PGState.opcode = I_none;
    X86PGState.need_init = false;
    X86PGState.opi = 0;
    X86PGState.trv_nodes = NULL;
    X86PGState.instlist = insnlist_create();
    X86PGState.insertpos = NULL;
    X86PGState.lock_ctrl = false;
    for (int i = 0; i < LOCK_REG_NUM; i++) {
        X86PGState.lock_reg_cases[i] = LOCK_REG_CASE_NULL;
    }
}

void reset_x86pgstate(void)
{
    init_text_sec(&X86PGState.text_sec);
    init_data_sec(&X86PGState.data_sec);
    X86PGState.labeli = 0;
    X86PGState.opcode = I_none;
    X86PGState.need_init = false;
    X86PGState.opi = 0;
    X86PGState.trv_nodes = NULL;
    X86PGState.insertpos = NULL;
    X86PGState.lock_ctrl = false;
    for (int i = 0; i < LOCK_REG_NUM; i++) {
        X86PGState.lock_reg_cases[i] = LOCK_REG_CASE_NULL;
    }
}

struct section *stat_get_data_sec(void)
{
    return &X86PGState.data_sec;
}

blk_struct *stat_get_curr_blk(void)
{
    return X86PGState.curr_blk;
}

void stat_set_curr_blk(blk_struct *blk)
{
    X86PGState.curr_blk = blk;
}

insnlist_entry_t *stat_get_insertpos(void)
{
    return X86PGState.insertpos;
}

void stat_set_insertpos(insnlist_entry_t *insertpos)
{
    X86PGState.insertpos = insertpos;
}

int stat_get_labeli(void)
{
    return X86PGState.labeli;
}

void stat_inc_labeli(void)
{
    X86PGState.labeli++;
}

insnlist_entry_t **stat_get_labelspos(void)
{
    return X86PGState.labelspos;
}

void stat_set_labelspos(insnlist_entry_t **labelspos)
{
    X86PGState.labelspos = labelspos;
}

enum opcode stat_get_opcode(void)
{
    return X86PGState.opcode;
}

void stat_set_opcode(enum opcode opcode)
{
    X86PGState.opcode = opcode;
}

bool stat_get_need_init(void)
{
    return X86PGState.need_init;
}

void stat_set_need_init(bool need_init)
{
    X86PGState.need_init = need_init;
}

int stat_get_opi(void)
{
    return X86PGState.opi;
}

void stat_set_opi(int opi)
{
    X86PGState.opi = opi;
}

bool stat_get_has_mem_opnd(void)
{
    return X86PGState.has_mem_opnd;
}

void stat_set_has_mem_opnd(bool has_mem_opnd)
{
    X86PGState.has_mem_opnd = has_mem_opnd;
}

char *stat_get_init_mem_addr(void)
{
    return (char *)X86PGState.init_mem_addr;
}

GArray *stat_get_trv_nodes(void)
{
    return X86PGState.trv_nodes;
}

void stat_set_trv_nodes(GArray *trv_nodes)
{
    X86PGState.trv_nodes = trv_nodes;
}

void stat_lock_ctrl(void)
{
    X86PGState.lock_ctrl = true;
}

void stat_unlock_ctrl(void)
{
    X86PGState.lock_ctrl = false;
}

bool stat_ctrl_locked(void)
{
    return X86PGState.lock_ctrl == true;
}

static uint32_t get_lock_reg_type(enum reg_enum reg)
{
    if (is_reg_ax(reg)) {
        return LOCK_REG_AX;
    } else if (is_reg_bx(reg)) {
        return LOCK_REG_BX;
    } else if (is_reg_cx(reg)) {
        return LOCK_REG_CX;
    } else if (is_reg_dx(reg)) {
        return LOCK_REG_DX;
    } else if (is_reg_si(reg)) {
        return LOCK_REG_SI;
    } else if (is_reg_di(reg)) {
        return LOCK_REG_DI;
    }
    return 0;
}

void stat_lock_reg(enum reg_enum reg, enum lock_reg_case lr_case)
{
    enum lock_reg_type lock_reg_type;
    lock_reg_type = get_lock_reg_type(reg);
    X86PGState.lock_reg_cases[lock_reg_type] = lr_case;
}

void stat_unlock_reg(enum lock_reg_case lr_case)
{
    for (int i = 0; i < LOCK_REG_NUM; i++) {
        if (X86PGState.lock_reg_cases[i] == lr_case) {
            X86PGState.lock_reg_cases[i] = LOCK_REG_CASE_NULL;
        }
    }
}

bool stat_reg_locked(enum reg_enum reg)
{
    enum lock_reg_type lock_reg_type;
    lock_reg_type = get_lock_reg_type(reg);
    return X86PGState.lock_reg_cases[lock_reg_type] != LOCK_REG_CASE_NULL;
}

void stat_insert_insn(insn *inst, enum position pos) {
    switch (pos) {
        case INSERT_AFTER:
            X86PGState.insertpos = insnlist_insert_after(X86PGState.instlist, X86PGState.insertpos, inst);
            break;
        case INSERT_BEFORE:
            X86PGState.insertpos = insnlist_insert_before(X86PGState.instlist, X86PGState.insertpos, inst);
            break;
        case INSERT_TAIL:
            X86PGState.insertpos = insnlist_insert_tail(X86PGState.instlist, inst);
            break;
        default:
            break;
    }
}
