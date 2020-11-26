#ifndef NASM_SEED_H
#define NASM_SEED_H

#include "nasm.h"
#include "iflag.h"

typedef uint64_t srcdestflags_t;
#define OPSRC       0x1
#define OPDEST      0x2

typedef struct operand_seed {
    enum opcode     opcode;               /* point to instruction */
    opflags_t       opndflags;
    srcdestflags_t  srcdestflags;         /* source and destination operand flag */
    opflags_t       opdsize;
} operand_seed;

typedef struct insn_seed {
    enum opcode     opcode;
    opflags_t       opd[MAX_OPERANDS];
} insn_seed;

typedef struct const_insn_seed {
    insn_seed       insn_seed;
    bool            oprs_random[MAX_OPERANDS];
    operand         oprs[MAX_OPERANDS];
} const_insn_seed;

void create_insn_seed(insn_seed *seed);
void create_const_insn_seed(const_insn_seed *const_seed);

#endif
