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
    char buffer[32];

    if (stat_ctrl_locked())
        return -1;

    sprintf(buffer, "label%d:", stat_get_labeli());
    one_insn_gen_ctrl(buffer, pos);
    stat_inc_labeli();
    insnlist_entry_t **labelspos = stat_get_labelspos();
    insnlist_entry_t *insertpos = stat_get_insertpos();
    labelspos = (insnlist_entry_t **)nasm_realloc(labelspos, stat_get_labeli() * sizeof(insnlist_entry_t *));
    labelspos[stat_get_labeli() - 1] = insertpos;
    stat_set_labelspos(labelspos);
    return stat_get_labeli() - 1;
}

static int select_one_label(void)
{
    int labeli = stat_get_labeli();
    return labeli ? nasm_random32(labeli) : -1;
}

static void gen_jmp_to_label(int label)
{
    char buffer[64];

    update_insert_pos_to_label(label);
    sprintf(buffer, "  jmp label%d", label);
    one_insn_gen_ctrl(buffer, INSERT_BEFORE);
    gen_label(INSERT_AFTER);
}

void update_insert_pos_to_label(int label)
{
    insnlist_entry_t **labelspos = stat_get_labelspos();
    stat_set_insertpos(labelspos[label]);
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
    case I_JCXZ:
    case I_JECXZ:
        return true;
        break;
    default:
        break;
    }
    return false;
}

void likely_gen_label(void)
{
    if (likely_happen_p(0.01))
        gen_label(INSERT_AFTER);
}

int gen_control_transfer_insn(enum opcode opcode, int times)
{
    int label = -1;
    char buffer[64];

    if (stat_ctrl_locked())
        return label;

    if (opcode == I_JMP) {
        /*   ...
         *   jmp label2
         * label1:
         *   ... <- next insert position
         * label2:
         *   ...
         *   jmp label1 <- the generated jmp
         *   ...
         */
        sprintf(buffer, "  %s label%d", nasm_insn_names[opcode], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_AFTER);
        label = select_one_label();
        if (label == -1) {
            gen_label(INSERT_BEFORE);
        } else {
            gen_jmp_to_label(label);
        }
    } else if (is_jcc(opcode)) {
        /*   ...
         *   jmp label2
         * label1:                          <- target address 1
         *   ... <- next insert position
         *   jcc label1
         *   jmp label1                     <- target address 2
         * label2:
         *   ...
         */
        sprintf(buffer, "  %s label%d", nasm_insn_names[opcode], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_AFTER);
        /* jmp lable(n+1) */
        sprintf(buffer, "  %s label%d", nasm_insn_names[I_JMP], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_AFTER);
        label = select_one_label();

        if (label == -1) {
            gen_label(INSERT_BEFORE);
        } else {
            gen_jmp_to_label(label);
        }
    } else if ((opcode == I_LOOP) ||
        (opcode == I_LOOPE) ||
        (opcode == I_LOOPNE) ||
        (opcode == I_LOOPNZ) ||
        (opcode == I_LOOPZ)) {
        /*   ...
         *   mov ecx, $times
         * label1:                          <- target address 1
         *   ... <- next insert position
         *   loopxx label1
         * label2:                          <- target address 2
         *   ...
         */
        label = gen_label(INSERT_AFTER);

        sprintf(buffer, "  %s label%d", nasm_insn_names[opcode], stat_get_labeli());
        one_insn_gen_ctrl(buffer, INSERT_BEFORE);

        sprintf(buffer, "  mov ecx,0x%x", times);
        one_insn_gen_ctrl(buffer, INSERT_BEFORE);

        gen_label(INSERT_AFTER);
        stat_lock_ctrl();
        stat_lock_reg(R_ECX, LOCK_REG_CASE_LOOP);
    } else if (opcode == I_CALL) {
        /*   ...
         *   jmp label2
         * label1:                          <- target address 1
         *   ... <- next insert position
         *   ret
         * label2:
         *   call label1
         * label3:                          <- target address 2
         *   ...
         */
        sprintf(buffer, "  jmp label%d", stat_get_labeli() + 1);
        one_insn_gen_ctrl(buffer, INSERT_AFTER);
        int labeli = gen_label(INSERT_AFTER);

        sprintf(buffer, "  %s", nasm_insn_names[I_RET]);
        one_insn_gen_ctrl(buffer, INSERT_AFTER);

        gen_label(INSERT_AFTER);

        sprintf(buffer, "  %s label%d", nasm_insn_names[opcode], labeli);
        one_insn_gen_ctrl(buffer, INSERT_AFTER);

        label = gen_label(INSERT_AFTER);

        update_insert_pos_to_label(labeli);
        stat_lock_ctrl();
    }
    return label;
}

/* jmp to the tail from current context
 */
void gen_control_transfer_finish(void)
{
    char buffer[64];
    sprintf(buffer, "  %s label%d", nasm_insn_names[I_JMP], stat_get_labeli());
    one_insn_gen_ctrl(buffer, INSERT_AFTER);
    gen_label(INSERT_TAIL);
}
