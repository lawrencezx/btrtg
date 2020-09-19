#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "gendata.h"
#include "error.h"
#include "gendata.h"
#include "regdis.h"
#include <string.h>

#define random(x) (rand() % x)

static char genbuf[20];

static void data_copy(const char *src, char *dst, bool (*is_validchar)(char))
{
    const char* r = src;
    while (is_validchar(*r))
        r++;
    memcpy(dst, src, r - src);
    dst[r - src] = '\0';
}

/* Generate random segment register. */
static const char* random_sreg(void)
{
    int sregi, n;
    enum reg_enum sreg;
    switch(globalbits) {
    case 16:
        n = 4;
        break;
    case 32:
        n = 6;
        break;
    case 64:
        n = 8;
        break;
    default:
        panic();
    }
    sregi = random(n);
    sreg = nasm_rd_sreg[sregi];
    return nasm_reg_names[sreg - EXPR_REG_START];
}

/* Generate random control register. */
static const char* random_creg(void)
{
    int cregi, n = 4;
    enum reg_enum creg;
    cregi = random(n);
    creg = nasm_rd_creg[cregi];
    return nasm_reg_names[creg - EXPR_REG_START];
}

/* Generate random debug register. */
static const char* random_dreg(void)
{
    int dregi, n;
    enum reg_enum dreg;
    switch(globalbits) {
    case 32:
        n = 8;
        break;
    case 64:
        n = 16;
        break;
    default:
        panic();
    }
    dregi = random(n);
    dreg = nasm_rd_dreg[dregi];
    return nasm_reg_names[dreg - EXPR_REG_START];
}

/* Generate random 16-bit register. */
static const char* random_reg8(void)
{
    int regi, n = 8;
    enum reg_enum reg;
    regi = random(n);
    reg = nasm_rd_reg8[regi];
    return nasm_reg_names[reg - EXPR_REG_START];
}

/* Generate random 16-bit register. */
static const char* random_reg16(void)
{
    int regi, n;
    enum reg_enum reg;
    switch(globalbits) {
    case 16:
    case 32:
        n = 8;
        break;
    case 64:
        n = 16;
        break;
    default:
        panic();
    }
    regi = random(n);
    reg = nasm_rd_reg16[regi];
    return nasm_reg_names[reg - EXPR_REG_START];
}

/* Generate random 16-bit register. */
static const char* random_reg32(void)
{
    int regi, n;
    enum reg_enum reg;
    switch(globalbits) {
    case 16:
    case 32:
        n = 8;
        break;
    case 64:
        n = 16;
        break;
    default:
        panic();
    }
    regi = random(n);
    reg = nasm_rd_reg32[regi];
    return nasm_reg_names[reg - EXPR_REG_START];
}

/* Generate random 16-bit register. */
static const char* random_reg64(void)
{
    int regi, n;
    enum reg_enum reg;
    switch(globalbits) {
    case 16:
    case 32:
        n = 8;
        break;
    case 64:
        n = 16;
        break;
    default:
        panic();
    }
    regi = random(n);
    reg = nasm_rd_reg64[regi];
    return nasm_reg_names[reg - EXPR_REG_START];
}

/* Generate int type immediate.
 * If it's larger than the limmit (8/16-bits imm), the high significant bytes
 * will be wipped away while assembling.
 * TODO: support 64-bit immediate
 */
static const char* random_imm(void)
{
    int num = random(INT_MAX);
    sprintf(genbuf, "%d", num);
    return genbuf;
}

/* Generate instruction opcode. */
void gen_op(enum opcode opcode, char *buffer)
{
    const char* insn_name = nasm_insn_names[opcode];
    data_copy(insn_name, buffer, nasm_isidchar);
}

/* Generate operand. */
void gen_opnd(opflags_t operand, char *buffer)
{
    const char *opnd_src = NULL;
    bool (*valid_func)(char);
    switch (operand) {
    case REG_AL:
        opnd_src = nasm_reg_names[R_AL - EXPR_REG_START];
        break;
    case REG_AX:
        opnd_src = nasm_reg_names[R_AX - EXPR_REG_START];
        break;
    case REG_EAX:
        opnd_src = nasm_reg_names[R_EAX - EXPR_REG_START];
        break;
    case REG_SREG:
        opnd_src = random_sreg();
        break;
    case REG_CREG:
        opnd_src = random_creg();
        break;
    case REG_DREG:
        opnd_src = random_dreg();
        break;
    case (REG_GPR|BITS8):
    case (RM_GPR|BITS8):
        opnd_src = random_reg8();
        break;
    case (REG_GPR|BITS16):
    case (RM_GPR|BITS16):
        opnd_src = random_reg16();
        break;
    case (REG_GPR|BITS32):
    case (RM_GPR|BITS32):
        opnd_src = random_reg32();
        break;
    case (REG_GPR|BITS64):
    case (RM_GPR|BITS64):
        opnd_src = random_reg64();
        break;
    case IMMEDIATE:
    case MEM_OFFS:
        opnd_src = random_imm();
        break;
    default:
        nasm_nonfatal("unsupported operand type");
        break;
    }
    if (operand == IMMEDIATE)
        valid_func = nasm_isdigit;
    else
        valid_func = nasm_isidchar;
    data_copy(opnd_src, buffer, valid_func);
}

void gendata_init(void)
{
    srand((unsigned)time(NULL));
}

