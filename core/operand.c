#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "error.h"
#include "seed.h"
#include "gendata.h"
#include "regdis.h"
#include "x86pg.h"
#include "operand.h"
#include "dfmt.h"
#include "model.h"
#include "generator.h"
#include "check.h"

void create_specific_register(enum reg_enum R_reg, operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create specific register\n");
    const char *src;
    src = nasm_reg_names[R_reg - EXPR_REG_START];
    if (X86PGState.need_init) {
        const char *instName = nasm_insn_names[X86PGState.curr_seed->opcode];
        constVal *cVal = request_constVal(instName, opnd_seed->srcdestflags & OPDEST);
        sprintf(buffer, "mov %s, 0x%x", src, (cVal == NULL) ? (int)nasm_random64(0x100000000) : cVal->imm32);
        one_insn_gen_const(buffer);
    }
    sprintf(buffer, "%s\n", src);
    dfmt->print("    done> new specific register: %s", buffer);
}

void create_control_register(operand_seed *opnd_seed, char *buffer)
{
    (void)opnd_seed;
    dfmt->print("    try> create creg\n");
    int cregi, cregn;
    enum reg_enum creg;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(REG_CREG);
    cregn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    cregi = nasm_random32(cregn);
    creg = nasm_rd_creg[cregi];
    src = nasm_reg_names[creg - EXPR_REG_START];
    sprintf(buffer, "%s\n", src);
    dfmt->print("    done> new creg: %s", buffer);
}

void create_segment_register(operand_seed *opnd_seed, char *buffer)
{
    (void)opnd_seed;
    dfmt->print("    try> create sreg\n");
    int sregi, sregn;
    enum reg_enum sreg;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(REG_SREG);
    sregn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    sregi = nasm_random32(sregn);
    sreg = nasm_rd_sreg[sregi];
    src = nasm_reg_names[sreg - EXPR_REG_START];
    sprintf(buffer, "%s\n", src);
    dfmt->print("    done> new sreg: %s", buffer);
}

void create_unity(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create unity\n");
    int unity, shiftCount;
    
    if (opnd_seed->opdsize == BITS8) {
        shiftCount = 8;
    } else if (opnd_seed->opdsize == BITS16) {
        shiftCount = 16;
    } else if (opnd_seed->opdsize == BITS32) {
        shiftCount = 32;
    }

    unity = nasm_random32(shiftCount + 1);
    if (X86PGState.need_init) {
        const char *instName = nasm_insn_names[X86PGState.curr_seed->opcode];
        constVal *cVal = request_constVal(instName, opnd_seed->srcdestflags & OPDEST);
        if (cVal != NULL)
            unity = cVal->imm32;
    }
    sprintf(buffer, "0x%x\n", unity);
    dfmt->print("    done> new unity: %s", buffer);
}

void create_gpr_register(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create gpr\n");
    int gpri, gprn;
    enum reg_enum gpr;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(opnd_seed->opndflags);

gen_gpr:
    gprn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    gpri = nasm_random32(gprn);

    switch (opnd_seed->opdsize) {
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
    if (!check_reg_valid(gpr))
        goto gen_gpr;  

    src = nasm_reg_names[gpr - EXPR_REG_START];

    if (X86PGState.need_init) {
        const char *instName = nasm_insn_names[X86PGState.curr_seed->opcode];
        constVal *cVal = request_constVal(instName, opnd_seed->srcdestflags & OPDEST);
        sprintf(buffer, "mov %s, 0x%x", src, (cVal == NULL) ? (int)nasm_random64(0x100000000) : cVal->imm32);
        one_insn_gen_const(buffer);
    }
    sprintf(buffer, "%s\n", src);
    dfmt->print("    done> new gpr: %s", buffer);
}

/* Generate int type immediate.
 * If it's larger than the limmit (8/16-bits imm), the high significant bytes
 * will be wipped away while assembling.
 */
void create_immediate(operand_seed* opnd_seed, char *buffer)
{
    dfmt->print("    try> create immediate\n");
    int imm;
    
    long long immn;
    switch (opnd_seed->opdsize) {
        case BITS8:
            immn = 0x100;
            break;
        case BITS16:
            immn = 0x10000;
            break;
        case BITS32:
            immn = 0x100000000;
            break;
        default:
            nasm_fatal("wrong immediate size");
            break;
    }
    imm = (int)nasm_random64(immn);

    if (X86PGState.need_init) {
        const char *instName = nasm_insn_names[X86PGState.curr_seed->opcode];
        constVal *cVal = request_constVal(instName, opnd_seed->srcdestflags & OPDEST);
        if (cVal != NULL)
            imm = cVal->imm32;
    }
    sprintf(buffer, "0x%x\n", imm);
    dfmt->print("    done> new immediate: %s", buffer);
}

static void create_random_sib(char *buffer)
{
    int bi, si;
    const char *base[8] = {
        "eax",   "ecx",   "edx",   "ebx",   "esp", "",      "esi",   "edi"
    };
    const char *scaledIndex[32] = {
        "eax",   "ecx",   "edx",   "ebx",   "",    "ebp",   "esi",   "edi",
        "eax*2", "ecx*2", "edx*2", "ebx*2", "",    "ebp*2", "esi*2", "edi*2",
        "eax*4", "ecx*4", "edx*4", "ebx*4", "",    "ebp*4", "esi*4", "edi*4",
        "eax*8", "ecx*8", "edx*8", "ebx*8", "",    "ebp*8", "esi*8", "edi*8",
    };
    bi = nasm_random32((int)ARRAY_SIZE(base));
    si = nasm_random32((int)ARRAY_SIZE(scaledIndex));
    if (strcmp(base[bi], "") == 0 && strcmp(scaledIndex[si], "") == 0) {
    } else if (strcmp(base[bi], "") == 0) {
        sprintf(buffer, "%s", scaledIndex[si]);
    } else if (strcmp(scaledIndex[si], "") == 0) {
        sprintf(buffer, "%s", base[bi]);
    } else {
        sprintf(buffer, "%s + %s", base[bi], scaledIndex[si]);
    }
}

static int random_disp8()
{
    return nasm_random32(0x80);
}

static int random_disp32()
{
    return (int)nasm_random64(0x80000000);
}

static void create_random_modrm(char *buffer)
{
    int modrmi, disp = 0;
    char sib[32];
    /* [ebx + disp8] */
    modrmi = 012;
    stat_lock_edx();
    //const int modrmn = 24;
    //modrmi = nasm_random32(modrmn);
    if (modrmi == 004 || modrmi == 014 || modrmi == 024) {
        create_random_sib(sib);
    }
    if ((modrmi & 030) == 010) {
        disp = random_disp8();
    } else if ((modrmi & 030) == 020 || modrmi == 005) {
        disp = random_disp32();
    }
    switch (modrmi & 007) {
        /* mod 00 */
        case 000:
            sprintf(buffer, "[eax + 0x%x]", disp);
            break;
        case 001:
            sprintf(buffer, "[ecx + 0x%x]", disp);
            break;
        case 002:
            sprintf(buffer, "[edx + 0x%x]", disp);
            break;
        case 003:
            sprintf(buffer, "[ebx + 0x%x]", disp);
            break;
        case 004:
            if (strcmp(sib, "")) {
                sprintf(buffer, "[0x%x]", disp);
            } else {
                sprintf(buffer, "[%s + 0x%x]", sib, disp);
            }
            break;
        case 005:
            if (modrmi == 005) {
                sprintf(buffer, "0x%x", disp);
            } else {
                sprintf(buffer, "[ebp + 0x%x]", disp);
            }
            break;
        case 006:
            sprintf(buffer, "[esi + 0x%x]", disp);
            break;
        case 007:
            sprintf(buffer, "[edi + 0x%x]", disp);
            break;
        default:
            nasm_nonfatal("unsupported modr/m form");
            break;
    }
}

void create_memory(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create memory\n");
    char modrm[64];
    char src[128];
    static const char *memsize[3] = {"byte", "word", "dword"};
    int whichmemsize = opnd_seed->opdsize == BITS8 ? 0 :
        (opnd_seed->opdsize == BITS16 ? 1 : 2);
    if (globalbits == 16) {
        nasm_fatal("unsupported 16-bit memory type");
    } else {
        create_random_modrm(modrm);
        if (opnd_seed->explicitmemsize) {
            sprintf(src, "%s %s", memsize[whichmemsize], modrm);
        } else {
            sprintf(src, "%s", modrm);
        }
    }
    if (X86PGState.need_init) {
        const char *instName = nasm_insn_names[X86PGState.curr_seed->opcode];
        constVal *cVal = request_constVal(instName, opnd_seed->srcdestflags & OPDEST);
        sprintf(buffer, "mov %s %s, 0x%x", memsize[whichmemsize], modrm, (cVal == NULL) ? (int)nasm_random64(0x100000000) : cVal->imm32);
        one_insn_gen_const(buffer);
    }
    sprintf(buffer, "%s\n", src);
    dfmt->print("    done> new memory: %s", buffer);
}

void init_specific_register(enum reg_enum R_reg, bool isDest)
{
    char buffer[128];
    const char *instName = nasm_insn_names[X86PGState.curr_seed->opcode];
    const char *src;
    src = nasm_reg_names[R_reg - EXPR_REG_START];
    constVal *cVal = request_constVal(instName, isDest);
    sprintf(buffer, "mov %s, 0x%x", src, (cVal == NULL) ? (int)nasm_random64(0x100000000) : cVal->imm32);
    one_insn_gen_const(buffer);
}
