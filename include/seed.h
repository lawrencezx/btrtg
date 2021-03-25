#ifndef NASM_SEED_H
#define NASM_SEED_H

#include "nasm.h"
#include "iflag.h"

typedef struct operand_seed {
    bool            is_var;
    bool            is_opnd_type;
    bool            has_label;
    opflags_t       opndflags;
} operand_seed;

typedef struct insn_seed {
    enum opcode     opcode;
    opflags_t       opd[MAX_OPERANDS];
} insn_seed;

void init_opnd_seed(operand_seed *opnd_seed);
void create_insn_seed(insn_seed *seed, const char *instname);
void create_opnd_seed(operand_seed *opnd_seed, const char *asm_opnd);

#endif
