#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "error.h"
#include "seed.h"
#include "gendata.h"
#include "regdis.h"
#include "operand.h"
#include <string.h>

srcdestflags_t calSrcDestFlags(enum opcode op, int opnum, int operands)
{
    srcdestflags_t srcdestflags = 0;

    switch (op) {
    case I_AAD:
    case I_AAM:
    case I_BOUND:
    case I_BT:
    case I_BTC:
    case I_BTR:
    case I_BTS:
    case I_CALL:
    case I_CMP:
    case I_DIV:
    case I_ENTER:
    case I_FBLD:
    case I_FCOM:
    case I_FCOMI:
    case I_FCOMIP:
    case I_FCOMP:
    case I_FCOMPP:
    case I_FICOM:
    case I_FICOMP:
    case I_FFREE:
    case I_FILD:
    case I_FLD:
    case I_FLDCW:
    case I_FLDENV:
    case I_FRSTOR:
    case I_FUCOM:
    case I_FUCOMI:
    case I_FUCOMIP:
    case I_FUCOMP:
    case I_FUCOMPP:
    case I_IDIV:
    case I_INT:
    case I_INVLPG:
    case I_JCXZ:
    case I_JECXZ:
    case I_JMP:
    case I_LGDT:
    case I_LIDT:
    case I_LLDT:
    case I_LMSW:
    case I_LOOP:
    case I_LOOPE:
    case I_LOOPNE:
    case I_LOOPNZ:
    case I_LOOPZ:
    case I_LTR:
    case I_MUL:
    case I_PUSH:
    case I_TEST:
    case I_VERR:
    case I_VERW:
    case I_COMISD:
    case I_COMISS:
    case I_UCOMISS:
    case I_UCOMISD:
    case I_Jcc:
        srcdestflags = OPSRC;
        break;
    case I_ADC:
    case I_ADD:
    case I_AND:
    case I_BSF:
    case I_BSR:
    case I_CMPXCHG:
    case I_CMPXCHG8B:
    case I_OR:
    case I_PACKSSDW:
    case I_PACKSSWB:
    case I_PACKUSWB:
    case I_PADDB:
    case I_PADDD:
    case I_PADDSB:
    case I_PADDSW:
    case I_PADDUSB:
    case I_PADDUSW:
    case I_PADDW:
    case I_PAND:
    case I_PANDN:
    case I_PMADDWD:
    case I_PMULHW:
    case I_PMULLW:
    case I_POR:
    case I_PSLLD:
    case I_PSLLQ:
    case I_PSLLW:
    case I_PSRAD:
    case I_PSRAW:
    case I_PSRLD:
    case I_PSRLQ:
    case I_PSRLW:
    case I_PSUBB:
    case I_PSUBD:
    case I_PSUBSB:
    case I_PSUBSW:
    case I_PSUBUSB:
    case I_PSUBUSW:
    case I_PSUBW:
    case I_PUNPCKHBW:
    case I_PUNPCKHDQ:
    case I_PUNPCKHWD:
    case I_PUNPCKHQDQ:
    case I_PUNPCKLBW:
    case I_PUNPCKLDQ:
    case I_PUNPCKLWD:
    case I_PUNPCKLQDQ:
    case I_PXOR:
    case I_RCL:
    case I_RCR:
    case I_ROL:
    case I_ROR:
    case I_SAL:
    case I_SAR:
    case I_SBB:
    case I_SHL:
    case I_SHLD:
    case I_SHR:
    case I_SHRD:
    case I_XCHG:
    case I_XOR:
    case I_ADDPS:
    case I_ADDSS:
    case I_ANDNPS:
    case I_ANDPS:
    case I_CVTPI2PS:
    case I_CVTPS2PI:
    case I_CVTSS2SI:
    case I_CVTTPS2PI:
    case I_CVTTSS2SI:
    case I_DIVPS:
    case I_DIVSS:
    case I_MAXPS:
    case I_MAXSS:
    case I_MINPS:
    case I_MINSS:
    case I_MULPS:
    case I_MULSS:
    case I_SUBPS:
    case I_SUBSS:
    case I_UNPCKHPS:
    case I_UNPCKLPS:
    case I_MASKMOVQ:
    case I_MOVNTQ:
    case I_PAVGB:
    case I_PAVGW:
    case I_PEXTRW:
    case I_PINSRW:
    case I_PMAXSW:
    case I_PMAXUB:
    case I_PMINSW:
    case I_PMINUB:
    case I_PMOVMSKB:
    case I_PMULHUW:
    case I_PSADBW:
    case I_PSHUFW:
    case I_MASKMOVDQU:
    case I_PADDQ:
    case I_PMULUDQ:
    case I_PSHUFD:
    case I_PSHUFHW:
    case I_PSHUFLW:
    case I_PSLLDQ:
    case I_PSRLDQ:
    case I_PSUBQ:
    case I_ADDPD:
    case I_ADDSD:
    case I_ANDNPD:
    case I_ANDPD:
    case I_CVTDQ2PD:
    case I_CVTDQ2PS:
    case I_CVTPD2DQ:
    case I_CVTPD2PI:
    case I_CVTPD2PS:
    case I_CVTPI2PD:
    case I_CVTPS2DQ:
    case I_CVTPS2PD:
    case I_CVTSD2SI:
    case I_CVTSD2SS:
    case I_CVTSI2SD:
    case I_CVTSS2SD:
    case I_CVTTPD2PI:
    case I_CVTTPD2DQ:
    case I_CVTTPS2DQ:
    case I_CVTTSD2SI:
    case I_DIVPD:
    case I_DIVSD:
    case I_MAXPD:
    case I_MAXSD:
    case I_MINPD:
    case I_MINSD:
    case I_MULPD:
    case I_MULSD:
    case I_ORPD:
    case I_SQRTPD:
    case I_SQRTSD:
    case I_SUBPD:
    case I_SUBSD:
    case I_UNPCKHPD:
    case I_UNPCKLPD:
    case I_XORPS:
    case I_XORPD:
    case I_LZCNT:
    case I_ARPL:
    case I_PCMPEQB:
    case I_PCMPEQD:
    case I_PCMPEQW:
    case I_PCMPGTB:
    case I_PCMPGTD:
    case I_PCMPGTW:
    case I_CMPEQPS:
    case I_CMPEQSS:
    case I_CMPLEPS:
    case I_CMPLESS:
    case I_CMPLTPS:
    case I_CMPLTSS:
    case I_CMPNEQPS:
    case I_CMPNEQSS:
    case I_CMPNLEPS:
    case I_CMPNLESS:
    case I_CMPNLTPS:
    case I_CMPNLTSS:
    case I_CMPORDPS:
    case I_CMPORDSS:
    case I_CMPUNORDPS:
    case I_CMPUNORDSS:
    case I_CMPEQPD:
    case I_CMPEQSD:
    case I_CMPLEPD:
    case I_CMPLESD:
    case I_CMPLTPD:
    case I_CMPLTSD:
    case I_CMPNEQPD:
    case I_CMPNEQSD:
    case I_CMPNLEPD:
    case I_CMPNLESD:
    case I_CMPNLTPD:
    case I_CMPNLTSD:
    case I_CMPORDPD:
    case I_CMPORDSD:
    case I_CMPUNORDPD:
    case I_CMPUNORDSD:
        if (opnum == 0) {
            srcdestflags = OPDEST;
            if (operands <= 2) {
                srcdestflags |= OPSRC;
            }
        } else {
            srcdestflags = OPSRC;
        }
        break;
    case I_BSWAP:
    case I_DEC:
    case I_FXCH:
    case I_INC:
    case I_NEG:
    case I_NOT:
        srcdestflags = OPSRC | OPDEST;
        break;
    case I_FADD:
    case I_FADDP:
    case I_FIADD:
    case I_FDIV:
    case I_FDIVP:
    case I_FIDIV:
    case I_FDIVR:
    case I_FDIVRP:
    case I_FIDIVR:
    case I_FMUL:
    case I_FMULP:
    case I_FIMUL:
    case I_FSUB:
    case I_FSUBP:
    case I_FISUB:
    case I_FSUBR:
    case I_FSUBRP:
    case I_FISUBR:
        srcdestflags = OPSRC;
        if (opnum == 0 &&
            operands >= 2) {
            srcdestflags |= OPDEST;
        }
        break;
    case I_FBSTP:
    case I_FIST:
    case I_FISTP:
    case I_POP:
    case I_SLDT:
    case I_SMSW:
    case I_STR:
    case I_SETcc:
    case I_FSAVE:
    case I_FNSAVE:
    case I_FST:
    case I_FSTP:
    case I_FSTCW:
    case I_FNSTCW:
    case I_FSTENV:
    case I_FNSTENV:
    case I_FSTSW:
    case I_FNSTSW:
        srcdestflags = OPDEST;
        break;
    case I_LAR:
    case I_LSL:
    case I_MOV:
    case I_MOVD:
    case I_MOVQ:
    case I_MOVSB:
    case I_MOVSD:
    case I_MOVSW:
    case I_MOVSX:
    case I_MOVZX:
    case I_MOVNTDQ:
    case I_MOVNTI:
    case I_MOVNTPD:
    case I_MOVDQA:
    case I_MOVDQU:
    case I_MOVDQ2Q:
    case I_MOVQ2DQ:
    case I_MOVAPD:
    case I_MOVHPD:
    case I_MOVLPD:
    case I_MOVMSKPD:
    case I_MOVUPD:
    case I_CMOVcc:
    case I_LDS:
    case I_LES:
    case I_LFS:
    case I_LGS:
    case I_LSS:
    case I_IN:
    case I_OUT:
    case I_RCPPS:
    case I_RCPSS:
    case I_RSQRTPS:
    case I_RSQRTSS:
    case I_SQRTPS:
    case I_SQRTSS:
    case I_FCMOVB:
    case I_FCMOVBE:
    case I_FCMOVE:
    case I_FCMOVNB:
    case I_FCMOVNBE:
    case I_FCMOVNE:
    case I_FCMOVNU:
    case I_FCMOVU:
        if (opnum == 0) {
            srcdestflags = OPDEST;
        } else {
            srcdestflags = OPSRC;
        }
        break;
    case I_IMUL:
        if (operands == 1) {
            srcdestflags = OPSRC;
        } else {
            if (opnum == 0) {
                srcdestflags = OPDEST;
                if (operands <= 2) {
                    srcdestflags |= OPSRC;
                }
            } else {
                srcdestflags = OPSRC;
            }
        }
        break;
    case I_SHUFPD:
    case I_SHUFPS:
        srcdestflags = OPSRC;
        if (opnum == 0) {
            srcdestflags |= OPDEST;
        }
        break;
    default:
        nasm_fatal("opcode XXX(TODO) should not have any operands or unsupported opcode");
        break;
    }
    return srcdestflags;
}

static opflags_t getCurOperandSize(opflags_t opflags)
{
    opflags_t opdsize = 0;
    if ((SIZE_MASK & opflags) != 0) {
        opdsize = SIZE_MASK & opflags;
    } else if (opflags == MEMORY ||
        opflags == (MEMORY|FAR)) {
        opdsize = (globalbits == 16) ? BITS16 : BITS32;
    } else {
        switch (opflags) {
            case REG_AL:
            case REG_CL:
                opdsize = BITS8;
                break;
            case REG_AX:
            case REG_CX:
            case REG_DX:
            case REG_ES:
            case REG_CS:
            case REG_SS:
            case REG_DS:
            case REG_FS:
            case REG_GS:
                opdsize = BITS16;
                break;
            case REG_EAX:
            case REG_ECX:
                opdsize = BITS32;
                break;
            default:
                break;
        }
    }
    return opdsize;
}

opflags_t calOperandSize(const insn_seed *seed, int opdi)
{
    opflags_t opdsize = 0;
    /* If operand's opflags contain size information, extract it. */
    if ((opdsize = getCurOperandSize(seed->opd[opdi])) == 0) {
    /* If operand's opflags does not contain size information
     * Copy the size information from other operands.
     */
        for (int i = 0; i < MAX_OPERANDS && seed->opd[i] != 0; i++) {
            if ((opdsize = getCurOperandSize(seed->opd[i])) != 0) {
                break;
            }
        }
    }
    if (opdsize == 0) {
        /* Instructions whose operands do not indicate it's operand sizeof()
         * We can calculate it according from instruction opcode.
         */
        switch (seed->opcode) {
            case I_RET:
            case I_RETF:
                opdsize = BITS16;
                break;
            case I_ENTER:
                opdsize = (opdi == 1) ? BITS16 : BITS8;
                break;
            default:
                opdsize = (globalbits == 16) ? BITS16 : BITS32;
                break;
        }
    }
    return opdsize;
}

/* Generate instruction opcode. */
void gen_opcode(enum opcode opcode, char *buffer)
{
    const char* insn_name = nasm_insn_names[opcode];
    sprintf(buffer, "%s\n", insn_name);
}

/* Generate operand. */
void gen_operand(operand_seed *opnd_seed, char *buffer)
{
    opflags_t opndflags;

    opndflags = opnd_seed->opndflags;
    switch (opndflags) {
    /* specific registers */
    case REG_AL:
        create_specific_register(buffer, R_AL);
        break;
    case REG_AX:
        create_specific_register(buffer, R_AX);
        break;
    case REG_EAX:
        create_specific_register(buffer, R_EAX);
        break;
    case REG_CL:
        create_specific_register(buffer, R_CL);
        break;
    case REG_CX:
        create_specific_register(buffer, R_CX);
        break;
    case REG_ECX:
        create_specific_register(buffer, R_ECX);
        break;
    case REG_DX:
        create_specific_register(buffer, R_DX);
        break;

    /* segment registers */
    case REG_ES:
        create_specific_register(buffer, R_ES);
        break;
    case REG_CS:
        create_specific_register(buffer, R_CS);
        break;
    case REG_SS:
        create_specific_register(buffer, R_SS);
        break;
    case REG_DS:
        create_specific_register(buffer, R_DS);
        break;
    case REG_FS:
        create_specific_register(buffer, R_FS);
        break;
    case REG_GS:
        create_specific_register(buffer, R_GS);
        break;

    /* special register class */
    case REG_CREG:
        create_control_register(buffer);
        break;
    case REG_SREG:
        create_segment_register(buffer);
        break;

    /*special immediate values*/
    case UNITY:
    {
        int shiftCount = 0;
        if (opnd_seed->opdsize == BITS8) {
            shiftCount = 8;
        } else if (opnd_seed->opdsize == BITS16) {
            shiftCount = 16;
        } else if (opnd_seed->opdsize == BITS32) {
            shiftCount = 32;
        }
        create_unity(buffer, shiftCount);
        break;
    }

//TODO:    /* special types of EAs */
//TODO:    case MEM_OFFS:

    /* immediate */
    case IMMEDIATE:
        create_immediate(buffer, opndflags & ~IMMEDIATE);
        break;
//TODO:    case IMMEDIATE|BITS8:
//TODO:    case IMMEDIATE|BITS16:
//TODO:    case IMMEDIATE|BITS32:
//TODO:    case IMMEDIATE|COLON:
//TODO:    case IMMEDIATE|BITS16|COLON:
//TODO:    case IMMEDIATE|BITS16|NEAR:
//TODO:    case IMMEDIATE|BITS32|COLON:
//TODO:    case IMMEDIATE|BITS32|NEAR:
//TODO:    case IMMEDIATE|SHORT:
//TODO:    case IMMEDIATE|NEAR:
//TODO:
//TODO:
//TODO:    /* memory */
//TODO:    case MEMORY:
//TODO:    case MEMORY|BITS8:
//TODO:    case MEMORY|BITS16:
//TODO:    case MEMORY|BITS32:
//TODO:    case MEMORY|FAR:

    /* general purpose registers */
    case REG_GPR|BITS8:
    case REG_GPR|BITS16:
    case REG_GPR|BITS32:
        create_gpr_register(buffer, opndflags & SIZE_MASK);
        break;

//TODO:    /* r/m */
//TODO:    case RM_GPR|BITS8:
//TODO:    case RM_GPR|BITS16:
//TODO:    case RM_GPR|BITS32:

    default:
        nasm_nonfatal("unsupported opnd type");
        break;
    }
}

void gendata_init(void)
{
    srand((unsigned)time(NULL));
}
