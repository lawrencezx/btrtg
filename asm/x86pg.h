#ifndef NASM_X86PG_H
#define NASM_X86PG_H

#include "insnlist.h"
#include "bseqi.h"

struct X86PGState {
    bool seqMode;
    big_sequence_index bseqi;
    const insn_seed *curr_seed;
    insn *curr_inst;
    insnlist_t *instlist;
};

extern struct X86PGState X86PGState;

void init_x86pgstate(void);

#endif
