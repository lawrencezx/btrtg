#include "compiler.h"

#include "nasmlib.h"
#include "seed.h"
#include "insnlist.h"
#include "x86pg.h"
#include "dfmt.h"
#include "ctrl.h"


static int gen_label_after(void)
{
    insn label;
    char buffer[32];
    sprintf(buffer, "label%d:", X86PGState.labeli++);
    label.ctrl = nasm_strdup(buffer);
    X86PGState.insertpos = insnlist_insert_after(X86PGState.instlist, X86PGState.insertpos, &label);
    X86PGState.labelspos = (insnlist_entry_t **)nasm_realloc(X86PGState.labelspos, X86PGState.labeli * sizeof(insnlist_entry_t *));
    X86PGState.labelspos[X86PGState.labeli - 1] = X86PGState.insertpos;
    return X86PGState.labeli;
}

static int gen_label_before(void)
{
    insn label;
    char buffer[32];
    sprintf(buffer, "label%d:", X86PGState.labeli++);
    label.ctrl = nasm_strdup(buffer);
    X86PGState.insertpos = insnlist_insert_before(X86PGState.instlist, X86PGState.insertpos, &label);
    X86PGState.labelspos = (insnlist_entry_t **)nasm_realloc(X86PGState.labelspos, X86PGState.labeli * sizeof(insnlist_entry_t *));
    X86PGState.labelspos[X86PGState.labeli - 1] = X86PGState.insertpos;
    return X86PGState.labeli;
}

static int gen_label_tail(void)
{
    insn label;
    char buffer[32];
    sprintf(buffer, "label%d:", X86PGState.labeli++);
    label.ctrl = nasm_strdup(buffer);
    X86PGState.insertpos = insnlist_insert_tail(X86PGState.instlist, &label);
    X86PGState.labelspos = (insnlist_entry_t **)nasm_realloc(X86PGState.labelspos, X86PGState.labeli * sizeof(insnlist_entry_t *));
    X86PGState.labelspos[X86PGState.labeli - 1] = X86PGState.insertpos;
    return X86PGState.labeli;
}

void likely_gen_label(void)
{
    if (likely_happen_p(0.01)) {
        gen_label_after();
    }
}

static int select_one_label(void)
{
    if (X86PGState.labeli > 0) {
        return nasm_random32(X86PGState.labeli);
    }
    return -1;
}

static int next_label(void)
{
    return X86PGState.labeli;
}

static void skip_to_next_label(int label)
{
    char buffer[64];
    insn jmp;

    X86PGState.insertpos = X86PGState.labelspos[label];
    sprintf(buffer, "  jmp label%d", label);
    jmp.ctrl = nasm_strdup(buffer);
    X86PGState.insertpos = insnlist_insert_before(X86PGState.instlist, X86PGState.insertpos, &jmp);
    gen_label_after();
}

bool gen_control_transfer_insn(const insn_seed *seed, insn *result)
{
    if (seed->opcode == I_JMP) {
        char buffer[64];
        int label = select_one_label();
        sprintf(buffer, "  jmp label%d", next_label());
        result->ctrl = nasm_strdup(buffer);
        X86PGState.insertpos = insnlist_insert_after(X86PGState.instlist, X86PGState.insertpos, result);
        if (label == -1) {
            gen_label_before();
        } else {
            skip_to_next_label(label);
        }
        return true;
    }
    return false;
}

void gen_control_transfer_finish(void)
{
    char buffer[64];
    insn jmp;
    sprintf(buffer, "  jmp label%d", next_label());
    jmp.ctrl = nasm_strdup(buffer);
    X86PGState.insertpos = insnlist_insert_after(X86PGState.instlist, X86PGState.insertpos, &jmp);
    gen_label_tail();
}
