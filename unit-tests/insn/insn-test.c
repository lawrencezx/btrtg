#include "compiler.h"

#include "error.h"
#include "generator.h"
#include "insnlist.h"
#include "ofmt.h"
#include "opflags.h"
#include "insn-test.h"

static void insn_gen_operand_initialize(const operand *opnd_seed, insnlist_t *instlist)
{
    opflags_t opnd_type;
    const_insn_seed const_seed;

    if (opnd_seed == NULL)
        return;

    opnd_type = opnd_seed->type;
    const_seed.opcode = I_MOV;
    const_seed.operands = 2;
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
            insnlist_insert_head(instlist, &mov_inst);
        }
    } else {
        nasm_fatal("Unsupported operand initialize\n");
    }
}

void gsp(const insn_seed *seed, const struct ofmt *ofmt)
{
    insnlist_t *instlist;
    insn new_inst;
    instlist = insnlist_create();

    while (one_insn_gen(seed, &new_inst)) {
    //    one_insn_gen(seed, &new_inst);
        insnlist_insert_head(instlist, &new_inst);
        for (int i = 0; i < new_inst.operands; i++) {
            insn_gen_operand_initialize(&new_inst.oprs[i], instlist);
        }
    }
    insnlist_output(instlist, ofmt);
    insnlist_clear(instlist);
    insnlist_destroy(instlist);
}
