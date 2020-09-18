#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "gendata.h"
#include "error.h"
#include "gendata.h"
#include "regdis.h"
#include <string.h>

#define random(x) (rand() % x)

static char genbuf[20];

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
    const char* r = insn_name + 1;
    while (nasm_isidchar(*r))
        r++;

    memcpy(buffer, insn_name, r - insn_name);
    buffer[r - insn_name] = '\0';
}

/* Generate operand. */
void gen_opnd(opflags_t operand, char *buffer)
{
    if (operand == (RM_GPR|BITS16)) {
        const char* reg_name = random_reg16();
        const char* r = reg_name + 1;
        while (nasm_isidchar(*r))
            r++;
    
        memcpy(buffer, reg_name, r - reg_name);
        buffer[r - reg_name] = '\0';
    } else if (operand == IMMEDIATE) {
        const char* imm = random_imm();
        const char* r = imm + 1;
        while (nasm_isdigit(*r))
            r++;
    
        memcpy(buffer, imm, r - imm);
        buffer[r - imm] = '\0';
    } else {
        nasm_nonfatal("unsupported operand type");
    }
}

void gendata_init(void)
{
    srand((unsigned)time(NULL));
}

