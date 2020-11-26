#include "compiler.h"

#include "error.h"
#include "seed.h"
#include "generator.h"
#include "insnlist.h"
#include "ofmt.h"
#include "opflags.h"
#include "insn-test.h"
#include "x86pg.h"

static void insn_gen_operand_initialize(const operand *opnd_seed)
{
    opflags_t opnd_type;
    const_insn_seed const_seed;

    if (opnd_seed == NULL)
        return;

    bool seqMode = X86PGState.seqMode;
    X86PGState.seqMode = false;
    opnd_type = opnd_seed->type;
    const_seed.insn_seed.opcode = I_MOV;
    assign_arr5(const_seed.insn_seed.opd, opnd_seed->type,IMMEDIATE,0,0,0);
    insn mov_inst;
    if (is_class(REGISTER, opnd_type)) {
        if (!(REG_SREG & ~opnd_type))
            ;
        else {
            const_seed.oprs[0] = *opnd_seed;
            const_seed.oprs_random[0] = false;
            const_seed.oprs[1].type = IMMEDIATE;
            const_seed.oprs_random[1] = true;
            one_insn_gen_const(&const_seed, &mov_inst);
            insnlist_insert(X86PGState.instlist, &mov_inst);
        }
    } else {
        nasm_fatal("Unsupported operand initialize\n");
    }
    X86PGState.seqMode = seqMode;
}

void gsp(const insn_seed *seed, const struct ofmt *ofmt)
{
    insn new_inst;

    while (one_insn_gen(seed, &new_inst)) {
        for (int i = 0; i < new_inst.operands; i++) {
            insn_gen_operand_initialize(&new_inst.oprs[i]);
        }
        insnlist_insert(X86PGState.instlist, &new_inst);
    }
    insnlist_output(X86PGState.instlist, ofmt);
    insnlist_clear(X86PGState.instlist);
    insnlist_destroy(X86PGState.instlist);
}
