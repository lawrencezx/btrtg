#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "gendata.h"
#include "error.h"
#include "gendata.h"
#include "regdis.h"
#include "randomlib.h"
#include "x86pg.h"
#include "operand.h"

static int imms[14] =
{
  0x0,        0x1,        0x7f,
  0x80,       0x7fff,     0x8000,
  0x03030303, 0x44444444, 0x7fffffff,
  0x80000000, 0x80000001, 0xcccccccc,
  0xf5f5f5f5, 0xffffffff
};

void create_specific_register(char *buffer, enum reg_enum R_reg)
{
    const char *src;
    src = nasm_reg_names[R_reg - EXPR_REG_START];
    sprintf(buffer, "%s\n", src);
}

void create_control_register(char *buffer)
{
    int cregi;
    enum reg_enum creg;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(REG_CREG);
    if (X86PGState.seqMode) {
        cregi = X86PGState.bseqi.indexes[SEQIFLAG_INDEX(bseqiflags)];
    } else {
        int cregn = SEQIFLAG_SEQINUM(bseqiflags);
        cregi = nasm_random(cregn);
    }
    creg = nasm_rd_creg[cregi];
    src = nasm_reg_names[creg - EXPR_REG_START];
    sprintf(buffer, "%s\n", src);
}

void create_segment_register(char *buffer)
{
    int sregi;
    enum reg_enum sreg;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(REG_SREG);
    if (X86PGState.seqMode) {
        sregi = X86PGState.bseqi.indexes[SEQIFLAG_INDEX(bseqiflags)];
    } else {
        int sregn = SEQIFLAG_SEQINUM(bseqiflags);
        sregi = nasm_random(sregn);
    }
    sreg = nasm_rd_sreg[sregi];
    src = nasm_reg_names[sreg - EXPR_REG_START];
    sprintf(buffer, "%s\n", src);
}

void create_unity(char *buffer, int shiftCount)
{
    int unity;
    unity = nasm_random(shiftCount + 1);
    sprintf(buffer, "%d\n", unity);
}

void create_gpr_register(char *buffer, opflags_t size)
{
    int gpri;
    enum reg_enum gpr;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(REG_GPR|size);
    if (X86PGState.seqMode) {
        gpri = X86PGState.bseqi.indexes[SEQIFLAG_INDEX(bseqiflags)];
    } else {
        int gprn = SEQIFLAG_SEQINUM(bseqiflags);
        gpri = nasm_random(gprn);
    }
    switch (size) {
        case BITS8:
            gpr = nasm_rd_reg8[gpri];
            break;
        case BITS16:
            gpr = nasm_rd_reg16[gpri];
            break;
        case BITS32:
            gpr = nasm_rd_reg32[gpri];
            break;
    }
    src = nasm_reg_names[gpr - EXPR_REG_START];
    sprintf(buffer, "%s\n", src);
}

/* Generate int type immediate.
 * If it's larger than the limmit (8/16-bits imm), the high significant bytes
 * will be wipped away while assembling.
 */
void create_immediate(char *buffer, opflags_t opflags)
{
    int immi, imm;
    
    bseqiflags_t bseqiflags = bseqi_flags(IMMEDIATE|opflags);
    if (X86PGState.seqMode) {
        immi = X86PGState.bseqi.indexes[SEQIFLAG_INDEX(bseqiflags)];
        imm = imms[immi];
    } else {
        int immn = INT_MAX;
        imm = nasm_random(immn);
    }
    sprintf(buffer, "%d\n", imm);
}
