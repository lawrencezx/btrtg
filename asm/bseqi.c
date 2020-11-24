#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "bseqi.h"


bseqiflags_t bseqi_flags(opflags_t opndflags)
{
    struct {
        opflags_t       opflags;
        bseqiflags_t    bseqiflags;
    } sequence_indexes[SEQ_INDEXES_NUM] = {
        {REG_SREG,          GEN_INDEX(0) | GEN_SEQINUM(((globalbits == 16) ? 4 : 6))},
        {REG_CREG,          GEN_INDEX(1) | GEN_SEQINUM(4)},
        {REG_GPR|BITS8,     GEN_INDEX(2) | GEN_SEQINUM(8)},
        {REG_GPR|BITS16,    GEN_INDEX(3) | GEN_SEQINUM(8)},
        {REG_GPR|BITS32,    GEN_INDEX(4) | GEN_SEQINUM(8)},
        {IMMEDIATE,         GEN_INDEX(5) | GEN_SEQINUM(14)}
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
            bseqi->num *= SEQIFLAG_SEQINUM(bseqiflags);
        }
    } else {
        if (bseqi->i >= bseqi->num)
            return false;
        int seqi = bseqi->i++;
        for (int i = 0; i < opnum; i++) {
            bseqiflags_t bseqiflags = bseqi_flags(seed->opd[i]);
            bseqi->indexes[SEQIFLAG_INDEX(bseqiflags)] = seqi % SEQIFLAG_SEQINUM(bseqiflags);
            seqi /= SEQIFLAG_SEQINUM(bseqiflags);
        }
    }
    return true;
}
