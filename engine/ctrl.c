#include "compiler.h"

#include "nasmlib.h"
#include "seed.h"
#include "insnlist.h"
#include "x86pg.h"
#include "dfmt.h"
#include "ctrl.h"
#include "generator.h"


static int gen_label(enum position pos)
{
    if (stat_ctrl_locked()) {
        return -1;
    }
    char buffer[32];
    sprintf(buffer, "label%d:", stat_get_labeli());
    one_insn_gen_ctrl(buffer, pos);
    stat_inc_labeli();
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

    X86PGState.insertpos = X86PGState.labelspos[label];
    sprintf(buffer, "  jmp label%d", label);
    one_insn_gen_ctrl(buffer, INSERT_BEFORE);
    gen_label(INSERT_AFTER);
}

static bool is_jcc(enum opcode opcode)
{
    switch (opcode) {
    case I_JA:
    case I_JAE:
    case I_JB:
    case I_JBE:
    case I_JC:
    case I_JE:
    case I_JG:
    case I_JGE:
    case I_JL:
    case I_JLE:
    case I_JNA:
    case I_JNAE:
    case I_JNB:
    case I_JNBE:
    case I_JNC:
    case I_JNE:
    case I_JNG:
    case I_JNGE:
    case I_JNL:
    case I_JNLE:
    case I_JNO:
    case I_JNP:
    case I_JNS:
    case I_JNZ:
    case I_JO:
    case I_JP:
    case I_JS:
    case I_JZ:
    case I_JPE:
    case I_JPO:
        return true;
        break;
    default:
        break;
    }
    return false;
}

bool gen_control_transfer_insn(const insn_seed *seed)
{
    if (stat_ctrl_locked()) {
        return true;
    }
    char buffer[64];
    if (seed->opcode == I_JMP) {
        /* jmp lable(n+1) */
        sprintf(buffer, "  %s label%d", nasm_insn_names[seed->opcode], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_AFTER);
        int label = select_one_label();
        if (label == -1) {
            gen_label(INSERT_BEFORE);
        } else {
            skip_to_stat_get_labeli(label);
        }
        return true;
    } else if (is_jcc(seed->opcode)) {
        /* jcc lable(n+1) */
        sprintf(buffer, "  %s label%d", nasm_insn_names[seed->opcode], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_AFTER);
        /* jmp lable(n+1) */
        sprintf(buffer, "  %s label%d", nasm_insn_names[I_JMP], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_AFTER);
        int label = select_one_label();
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
        /* loopxx lable(n+1) */
        sprintf(buffer, "  %s label%d", nasm_insn_names[seed->opcode], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_AFTER);

        gen_label(INSERT_BEFORE);
        stat_lock_ctrl();
        stat_lock_ecx();
        return true;
    } else if (seed->opcode == I_CALL) {
        /* call lable(n+1) */
        sprintf(buffer, "  %s label%d", nasm_insn_names[seed->opcode], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_AFTER);

        sprintf(buffer, "  %s", nasm_insn_names[I_RET]);
        one_insn_gen_ctrl(buffer, INSERT_BEFORE);
        gen_label(INSERT_BEFORE);
        stat_lock_ctrl();
        return true;
    }
    return false;
}

void gen_control_transfer_finish(void)
{
    char buffer[64];
    sprintf(buffer, "  %s label%d", nasm_insn_names[I_JMP], stat_get_labeli());
    one_insn_gen_ctrl(buffer, INSERT_AFTER);
    gen_label(INSERT_TAIL);
}
