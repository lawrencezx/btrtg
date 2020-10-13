#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "gendata.h"
#include "error.h"
#include "gendata.h"
#include "regdis.h"
#include <string.h>

bool sequence = false;
static char genbuf[20];

#define random(x) (rand() % x)

#define random_regi(opflag) random(get_opnd_num(opflag))
#define sequence_regi(reg) sqi.reg##i
#define new_reg(reg,opflag) (sequence ? \
        nasm_reg_names[nasm_rd_##reg[sequence_regi(reg)] - EXPR_REG_START] : \
        nasm_reg_names[nasm_rd_##reg[random_regi(opflag)] - EXPR_REG_START])
#define new_opnd(opnd) (sequence ? sequence_##opnd() : random_##opnd())

static void data_copy(const char *src, char *dst, bool (*is_validchar)(char))
{
    const char* r = src;
    while (is_validchar(*r))
        r++;
    memcpy(dst, src, r - src);
    dst[r - src] = '\0';
}

static int get_opnd_num(opflags_t operand)
{
    int n = 0;
    switch(operand) {
    case REG_SREG:
        n = (globalbits == 16) ? 4 :
            (globalbits == 32) ? 6 :
            (globalbits == 64) ? 8 : -1;
        break;
    case REG_CREG:
        n = 4;
        break;
    case REG_DREG:
        n = (globalbits == 32) ? 8 :
            (globalbits == 64) ? 16 : -1;
        break;
    case (REG_GPR|BITS8):
    case (RM_GPR|BITS8):
        n = 8;
        break;
    case (REG_GPR|BITS16):
    case (RM_GPR|BITS16):
    case (REG_GPR|BITS32):
    case (RM_GPR|BITS32):
    case (REG_GPR|BITS64):
    case (RM_GPR|BITS64):
        n = (globalbits == 16 || globalbits == 32) ? 8 :
            (globalbits == 64) ? 16 : -1;
        break;
    case IMMEDIATE:
    case MEM_OFFS:
        n = 14;
        break;
    default:
        nasm_nonfatal("unsupported opnd type");
        break;
    }
    return n;
}

/* Generate int type immediate randomly.
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

/* sequence_index:
 *     A super index structure used to traverse all the operand combinations.
 */
struct sequence_index {
    bool start;
    int i;
    int num;
    int sregi;
    int cregi;
    int dregi;
    int reg8i;
    int reg16i;
    int reg32i;
    int reg64i;
    int immi;
} sqi;

static int imms[14] =
{
  0x0,        0x1,        0x7f,
  0x80,       0x7fff,     0x8000,
  0x03030303, 0x44444444, 0x7fffffff,
  0x80000000, 0x80000001, 0xcccccccc,
  0xf5f5f5f5, 0xffffffff
};

static void sqi_init(void)
{
    sqi.start = false;
    sqi.i = 0;
    sqi.num = 0;
    sqi.sregi = 0;
    sqi.cregi = 0;
    sqi.dregi = 0;
    sqi.reg8i = 0;
    sqi.reg16i = 0;
    sqi.reg32i = 0;
    sqi.reg64i = 0;
    sqi.immi = 0;
}

static void sqi_set_opi(opflags_t operand, int i)
{
    switch(operand) {
    case REG_SREG:
        sqi.sregi = i;
        break;
    case REG_CREG:
        sqi.cregi = i;
        break;
    case REG_DREG:
        sqi.dregi = i;
        break;
    case (REG_GPR|BITS8):
    case (RM_GPR|BITS8):
        sqi.reg8i = i;
        break;
    case (REG_GPR|BITS16):
    case (RM_GPR|BITS16):
        sqi.reg16i = i;
        break;
    case (REG_GPR|BITS32):
    case (RM_GPR|BITS32):
        sqi.reg32i = i;
        break;
    case (REG_GPR|BITS64):
    case (RM_GPR|BITS64):
        sqi.reg64i = i;
        break;
    case IMMEDIATE:
    case MEM_OFFS:
        sqi.immi = i;
        break;
    default:
        nasm_nonfatal("unsupported opnd type");
        break;
    }
}

bool sqi_inc(insn_seed *seed, int opnum)
{
    if (sequence) {
        if (!sqi.start) {
            sqi.start = true;
            sqi.num = 1;
            for (int i = 0; i < opnum; i++) {
                sqi.num *= get_opnd_num(seed->opd[i]);
            }
        } else {
            if (sqi.i >= sqi.num)
                return false;
            int seqi = sqi.i++;
            for (int i = 0; i < opnum; i++) {
                sqi_set_opi(seed->opd[i], seqi % get_opnd_num(seed->opd[i]));
                seqi /= get_opnd_num(seed->opd[i]);
            }
        }
    }
    return true;
}

/* Generate int type immediate sequentially.
 * If it's larger than the limmit (8/16-bits imm), the high significant bytes
 * will be wipped away while assembling.
 * TODO: support 64-bit immediate
 */
static const char* sequence_imm(void)
{
    int num = imms[sqi.immi];
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
        opnd_src = new_reg(sreg, operand);
        break;
    case REG_CREG:
        opnd_src = new_reg(creg, operand);
        break;
    case REG_DREG:
        opnd_src = new_reg(dreg, operand);
        break;
    case (REG_GPR|BITS8):
    case (RM_GPR|BITS8):
        opnd_src = new_reg(reg8, operand);
        break;
    case (REG_GPR|BITS16):
    case (RM_GPR|BITS16):
        opnd_src = new_reg(reg16, operand);
        break;
    case (REG_GPR|BITS32):
    case (RM_GPR|BITS32):
        opnd_src = new_reg(reg32, operand);
        break;
    case (REG_GPR|BITS64):
    case (RM_GPR|BITS64):
        opnd_src = new_reg(reg64, operand);
        break;
    case IMMEDIATE:
    case MEM_OFFS:
        opnd_src = new_opnd(imm);
        break;
    default:
        nasm_nonfatal("unsupported opnd type");
        break;
    }
    if (operand == IMMEDIATE)
        valid_func = nasm_isdigit;
    else
        valid_func = nasm_isidchar;
    data_copy(opnd_src, buffer, valid_func);
}

void gendata_init(bool set_sequence)
{
    sequence = set_sequence;
    if (set_sequence) {
        sqi_init();
    } else {
        srand((unsigned)time(NULL));
    }
}
