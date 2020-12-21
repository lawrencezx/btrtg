#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "seed.h"
#include "x86pg.h"


bseqiflags_t bseqi_flags(opflags_t opndflags)
{
    struct {
        opflags_t       opflags;
        bseqiflags_t    bseqiflags;
    } sequence_indexes[] = {
        {REG_SREG,          GEN_INDEXPOS(0) | GEN_INDEXSIZE(((globalbits == 16) ? 4 : 6))},
        {REG_CREG,          GEN_INDEXPOS(1) | GEN_INDEXSIZE(4)},
        {REG_GPR|BITS8,     GEN_INDEXPOS(2) | GEN_INDEXSIZE(8)},
        {RM_GPR|BITS8,      GEN_INDEXPOS(3) | GEN_INDEXSIZE(8)},
        {REG_GPR|BITS16,    GEN_INDEXPOS(4) | GEN_INDEXSIZE(4)},
        {RM_GPR|BITS16,     GEN_INDEXPOS(5) | GEN_INDEXSIZE(4)},
        {REG_GPR|BITS32,    GEN_INDEXPOS(6) | GEN_INDEXSIZE(4)},
        {RM_GPR|BITS32,     GEN_INDEXPOS(7) | GEN_INDEXSIZE(4)},
        {IMMEDIATE,         GEN_INDEXPOS(8) | GEN_INDEXSIZE(14)}
    };

    for (size_t i = 0; i < ARRAY_SIZE(sequence_indexes); i++) {
        if (opndflags == sequence_indexes[i].opflags)
            return sequence_indexes[i].bseqiflags;
    }
    return -1;
}

void bseqi_init(big_sequence_index *bseqi)
{
    bseqi->start = false;
    bseqi->i = 0;
    bseqi->num = 0;
    for (size_t i = 0; i < ARRAY_SIZE(bseqi->indexes); i++) {
        bseqi->indexes[i] = 0;
    }
}

bool bseqi_inc(big_sequence_index *bseqi, const insn_seed *seed, int opnum)
{
    if (!bseqi->start) {
        bseqi->start = true;
        bseqi->i = 1;
        bseqi->num = 1;
        for (int i = 0; i < opnum; i++) {
            bseqiflags_t bseqiflags = bseqi_flags(seed->opd[i]);
            bseqi->num *= BSEQIFLAG_INDEXSIZE(bseqiflags);
        }
    } else {
        if (bseqi->i >= bseqi->num)
            return false;
        int seqi = bseqi->i++;
        for (int i = 0; i < opnum; i++) {
            bseqiflags_t bseqiflags = bseqi_flags(seed->opd[i]);
            bseqi->indexes[BSEQIFLAG_INDEXPOS(bseqiflags)] = seqi % BSEQIFLAG_INDEXSIZE(bseqiflags);
            seqi /= BSEQIFLAG_INDEXSIZE(bseqiflags);
        }
    }
    return true;
}
