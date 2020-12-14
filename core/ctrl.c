#include "compiler.h"

#include "nasmlib.h"
#include "seed.h"
#include "insnlist.h"
#include "x86pg.h"
#include "dfmt.h"
#include "ctrl.h"


static int gen_label(enum position pos)
{
    if (stat_ctrl_locked()) {
        return -1;
    }
    insn label;
    char buffer[32];
    sprintf(buffer, "label%d:", stat_get_labeli());
    stat_inc_labeli();
    label.ctrl = nasm_strdup(buffer);
    stat_insert_insn(&label, pos);
    X86PGState.labelspos = (insnlist_entry_t **)nasm_realloc(X86PGState.labelspos, stat_get_labeli() * sizeof(insnlist_entry_t *));
    X86PGState.labelspos[stat_get_labeli() - 1] = X86PGState.insertpos;
    return stat_get_labeli();
}

void likely_gen_label(void)
{
    if (likely_happen_p(0.01)) {
        gen_label(INSERT_AFTER);
    }
}

static int select_one_label(void)
{
    if (stat_get_labeli() > 0) {
        return nasm_random32(stat_get_labeli());
    }
    return -1;
}

static void skip_to_stat_get_labeli(int label)
{
    char buffer[64];
    insn jmp;

    X86PGState.insertpos = X86PGState.labelspos[label];
    sprintf(buffer, "  jmp label%d", label);
    jmp.ctrl = nasm_strdup(buffer);
    stat_insert_insn(&jmp, INSERT_BEFORE);
    gen_label(INSERT_AFTER);
}

bool gen_control_transfer_insn(const insn_seed *seed, insn *result)
{
    if (stat_ctrl_locked()) {
        return true;
    }
    char buffer[64];
    if (seed->opcode == I_JMP) {
        int label = select_one_label();
        sprintf(buffer, "  %s label%d", nasm_insn_names[seed->opcode], stat_get_labeli());
        result->ctrl = nasm_strdup(buffer);
        stat_insert_insn(result, INSERT_AFTER);
        if (label == -1) {
            gen_label(INSERT_BEFORE);
        } else {
            skip_to_stat_get_labeli(label);
        }
        return true;
    } else if ((seed->opcode == I_LOOP) ||
        (seed->opcode == I_LOOPE) ||
        (seed->opcode == I_LOOPNE) ||
        (seed->opcode == I_LOOPNZ) ||
        (seed->opcode == I_LOOPZ)) {
        sprintf(buffer, "  %s label%d", nasm_insn_names[seed->opcode], stat_get_labeli());
        result->ctrl = nasm_strdup(buffer);
        stat_insert_insn(result, INSERT_AFTER);
        gen_label(INSERT_BEFORE);
        stat_lock_ctrl();
        stat_lock_ecx();
    } else if (seed->opcode == I_CALL) {
        sprintf(buffer, "  %s label%d", nasm_insn_names[seed->opcode], stat_get_labeli());
        result->ctrl = nasm_strdup(buffer);
        stat_insert_insn(result, INSERT_AFTER);
        sprintf(buffer, "  %s", nasm_insn_names[I_RET]);
        result->ctrl = nasm_strdup(buffer);
        stat_insert_insn(result, INSERT_BEFORE);
        gen_label(INSERT_BEFORE);
        stat_lock_ctrl();
    }
    return false;
}

void gen_control_transfer_finish(void)
{
    char buffer[64];
    insn jmp;
    sprintf(buffer, "  %s label%d", nasm_insn_names[I_JMP], stat_get_labeli());
    jmp.ctrl = nasm_strdup(buffer);
    stat_insert_insn(&jmp, INSERT_AFTER);
    gen_label(INSERT_TAIL);
}
