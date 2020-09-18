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
    bool (*valid_func)(char) = NULL;
    switch (operand) {
    case (RM_GPR|BITS16):
        opnd_src = random_reg16();
        valid_func = nasm_isidchar;
        break;
    case IMMEDIATE:
        opnd_src = random_imm();
        valid_func = nasm_isdigit;
        break;
    default:
        nasm_nonfatal("unsupported operand type");
        break;
    }
    data_copy(opnd_src, buffer, valid_func);
}

void gendata_init(void)
{
    srand((unsigned)time(NULL));
}

