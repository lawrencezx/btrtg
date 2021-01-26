#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "error.h"
#include "seed.h"
#include "gendata.h"
#include "regdis.h"
#include "operand.h"
#include "x86pg.h"
#include "buf2token.h"
#include "tk.h"
#include "asmlib.h"
#include "generator.h"


static bool is_label_consumer(operand_seed *opnd_seed)
{
    if (is_class(MEM_OFFS, opnd_seed->opndflags)) {
        return true;
    }
    return false;
}

static srcdestflags_t calSrcDestFlags(const insn_seed *seed, int opi)
{
    srcdestflags_t srcdestflags = 0;
    enum opcode op = seed->opcode;
    int operands = 0;
    while (seed->opd[operands]) {
        operands++;
    }

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
    case I_JA:
    case I_JAE:
    case I_JB:
    case I_JBE:
    case I_JC:
    case I_JE:
    case I_JG:
    case I_JGE:
    case I_JL:
    case I_JLE:
    case I_JNA:
    case I_JNAE:
    case I_JNB:
    case I_JNBE:
    case I_JNC:
    case I_JNE:
    case I_JNG:
    case I_JNGE:
    case I_JNL:
    case I_JNLE:
    case I_JNO:
    case I_JNP:
    case I_JNS:
    case I_JNZ:
    case I_JO:
    case I_JP:
    case I_JS:
    case I_JZ:
    case I_JPE:
    case I_JPO:
        srcdestflags = OPSRC;
        break;
    case I_ADC:
    case I_SUB:
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
        if (opi == 0) {
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
        if (opi == 0 &&
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
    case I_SETA:
    case I_SETAE:
    case I_SETB:
    case I_SETBE:
    case I_SETC:
    case I_SETE:
    case I_SETG:
    case I_SETGE:
    case I_SETL:
    case I_SETLE:
    case I_SETNA:
    case I_SETNAE:
    case I_SETNB:
    case I_SETNBE:
    case I_SETNC:
    case I_SETNE:
    case I_SETNG:
    case I_SETNGE:
    case I_SETNL:
    case I_SETNLE:
    case I_SETNO:
    case I_SETNP:
    case I_SETNS:
    case I_SETNZ:
    case I_SETO:
    case I_SETP:
    case I_SETPE:
    case I_SETPO:
    case I_SETS:
    case I_SETZ:
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
    case I_CMOVA:
    case I_CMOVAE:
    case I_CMOVB:
    case I_CMOVBE:
    case I_CMOVC:
    case I_CMOVE:
    case I_CMOVG:
    case I_CMOVGE:
    case I_CMOVL:
    case I_CMOVLE:
    case I_CMOVNA:
    case I_CMOVNAE:
    case I_CMOVNB:
    case I_CMOVNBE:
    case I_CMOVNC:
    case I_CMOVNE:
    case I_CMOVNG:
    case I_CMOVNGE:
    case I_CMOVNL:
    case I_CMOVNLE:
    case I_CMOVNO:
    case I_CMOVNP:
    case I_CMOVNS:
    case I_CMOVNZ:
    case I_CMOVO:
    case I_CMOVP:
    case I_CMOVPE:
    case I_CMOVPO:
    case I_CMOVS:
    case I_CMOVZ:
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
        if (opi == 0) {
            srcdestflags = OPDEST;
        } else {
            srcdestflags = OPSRC;
        }
        break;
    case I_IMUL:
        if (operands == 1) {
            srcdestflags = OPSRC;
        } else {
            if (opi == 0) {
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
        if (opi == 0) {
            srcdestflags |= OPDEST;
        }
        break;
    case I_XADD:
        srcdestflags = OPSRC | OPDEST;
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
    }
    /* or if current operand opflags does not have size property
     * get it from current instruction's other properties
     */
    return opdsize;
}

static opflags_t calOperandSize(const insn_seed *seed, int opdi)
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
        /*TODO: Instructions whose name end with B/W/D. */
    }
    return opdsize;
}

static void init_implicit_operands(const insn_seed *seed)
{
    if (seed->opcode == I_DIV ||
        seed->opcode == I_IDIV) {
        switch (calOperandSize(seed, 0)) {
            case BITS8:
                init_specific_register(R_AX, true);
                break;
            case BITS16:
                init_specific_register(R_DX, true);
                init_specific_register(R_AX, true);
                break;
            case BITS32:
                init_specific_register(R_EDX, true);
                init_specific_register(R_EAX, true);
                break;
            default:
                break;
        }
    }
}

static void gen_comma(char *buffer)
{
    sprintf(buffer, ",");
}

/* Generate instruction opcode. */
bool gen_opcode(const insn_seed *seed)
{
    if (seed == NULL)
        return true;
    const char *inst_name;

    /* write instruction name to token_buf */
    inst_name = nasm_insn_names[seed->opcode];
    sprintf(get_token_cbufptr(), "%s ", inst_name);

    /* initialize implicit operands, exp: eax in mul  */
    if (stat_get_need_init())
        init_implicit_operands(seed);

    return true;
}

static void init_register_opnd(char *asm_opnd, operand_seed *opnd_seed)
{
    char asm_mov_inst[128];
    struct const_node *val_node;
    GArray *val_nodes = stat_get_val_nodes();
    if (val_nodes == NULL) {
        const char *instName = nasm_insn_names[stat_get_opcode()];
        val_node = request_val_node(instName, opnd_seed->srcdestflags & OPDEST);
    } else {
        val_node = g_array_index(val_nodes, struct const_node *, stat_get_opi());
    }
    sprintf(asm_mov_inst, "mov %s, 0x%x", asm_opnd, (val_node == NULL) ?
            (int)nasm_random64(RAND_BITS32_BND) : val_node->imm32);
    one_insn_gen_const(asm_mov_inst);
}

static void init_immediate_opnd(char *asm_opnd, operand_seed *opnd_seed)
{
    int immediate = 0;
    struct const_node *val_node;
    GArray *val_nodes = stat_get_val_nodes();
    if (val_nodes == NULL) {
        const char *instName = nasm_insn_names[stat_get_opcode()];
        val_node = request_val_node(instName, opnd_seed->srcdestflags & OPDEST);
    } else {
        val_node = g_array_index(val_nodes, struct const_node *, stat_get_opi());
    }
    if (val_node != NULL) {
        immediate = val_node->imm32;
        sprintf(asm_opnd, "0x%x", immediate);
    }
}

static void init_memory_opnd(char *asm_opnd, operand_seed *opnd_seed)
{
    char asm_mov_inst[128];
    struct const_node *val_node;
    GArray *val_nodes = stat_get_val_nodes();
    if (val_nodes == NULL) {
        const char *instName = nasm_insn_names[stat_get_opcode()];
        val_node = request_val_node(instName, opnd_seed->srcdestflags & OPDEST);
    } else {
        val_node = g_array_index(val_nodes, struct const_node *, stat_get_opi());
    }
    sprintf(asm_mov_inst, "mov %s, 0x%x", asm_opnd, (val_node == NULL) ?
            (int)nasm_random64(RAND_BITS32_BND) : val_node->imm32);
    preappend_mem_size(asm_mov_inst + 4, opnd_seed->opdsize);
    one_insn_gen_const(asm_mov_inst);
}

static void init_memoffs_opnd(char *asm_opnd, operand_seed *opnd_seed)
{
    char asm_mov_inst[128];
    struct const_node *val_node;
    GArray *val_nodes = stat_get_val_nodes();
    if (val_nodes == NULL) {
        const char *instName = nasm_insn_names[stat_get_opcode()];
        val_node = request_val_node(instName, opnd_seed->srcdestflags & OPDEST);
    } else {
        val_node = g_array_index(val_nodes, struct const_node *, stat_get_opi());
    }
    sprintf(asm_mov_inst, "mov %s, 0x%x", asm_opnd, (val_node == NULL) ?
            (int)nasm_random64(RAND_BITS32_BND) : val_node->imm32);
    preappend_mem_size(asm_mov_inst + 4, opnd_seed->opdsize);
    one_insn_gen_const(asm_mov_inst);
}

/******************************************************************************
*
* Function name: init_opnd
* Description: initialize the value of an operand
*              Call the related initializing function according the operand type.
* Parameter:
*       @asm_opnd: string, an operand in assembly
*       @opnd_seed: operand seed used to generate the operand
*       @var: variable used to generate the operand, or NULL if the operand is 
*             not generated from a variable. It is only used when the operand
*             is MEMORY type.
* Return: bool
******************************************************************************/
static void init_opnd(char *asm_opnd, operand_seed *opnd_seed, blk_var *var)
{
    if (!stat_get_need_init())
        return;

    stat_set_need_init(false);
    opflags_t opndflags = opnd_seed->opndflags;
    if (is_class(REGISTER, opndflags))
        init_register_opnd(asm_opnd, opnd_seed);
    else if (is_class(REGMEM, opndflags)) {
        if (is_class(MEM_OFFS, opndflags)) {
            init_memoffs_opnd(asm_opnd, opnd_seed);
        } else {
            stat_set_has_mem_opnd(false);
            if (var)
                one_insn_gen_ctrl(var->init_mem_addr, INSERT_AFTER);
            else
                one_insn_gen_ctrl(stat_get_init_mem_addr(), INSERT_AFTER);
            init_memory_opnd(asm_opnd, opnd_seed);
            stat_set_has_mem_opnd(true);
        }
    }
    stat_set_need_init(true);
}

static bool gen_register(operand_seed *opnd_seed, char *buffer)
{
    opflags_t opndflags;
    opndflags = opnd_seed->opndflags;

    static const struct {
        opflags_t       flags;
        enum reg_enum   reg;
    } specific_registers[] = {
        {REG_AL,  R_AL},
        {REG_AX,  R_AX},
        {REG_EAX, R_EAX},
        {REG_DL,  R_DL},
        {REG_DX,  R_DX},
        {REG_EDX, R_EDX},
        {REG_CL,  R_CL},
        {REG_CX,  R_CX},
        {REG_ECX, R_ECX},
        {FPU0,    R_ST0},
        {XMM0,    R_XMM0},
        {YMM0,    R_YMM0},
        {ZMM0,    R_ZMM0},
        {REG_ES,  R_ES},
        {REG_CS,  R_CS},
        {REG_SS,  R_SS},
        {REG_DS,  R_DS},
        {REG_FS,  R_FS},
        {REG_GS,  R_GS},
    };

    for (size_t i = 0; i < ARRAY_SIZE(specific_registers); i++)
        if (!(specific_registers[i].flags & ~opndflags)) {
            return create_specific_register(specific_registers[i].reg, opnd_seed, buffer);
        }

    if (is_class(REG_CLASS_CDT, opndflags)) {
        if (is_class(REG_CREG, opndflags)) {
            return create_control_register(opnd_seed, buffer);
        } else {
            /* TODO */
        }
    } else if (is_class(REG_CLASS_GPR, opndflags)) {
        return create_gpr_register(opnd_seed, buffer);
    } else if (is_class(REG_CLASS_SREG, opndflags)) {
        return create_segment_register(opnd_seed, buffer);
    } else if (is_class(REG_CLASS_FPUREG, opndflags)) {
        /* TODO */
    } else if (is_class(REG_CLASS_RM_MMX, opndflags)) {
        /* TODO */
    } else if (is_class(REG_CLASS_RM_XMM, opndflags)) {
        /* TODO */
    } else if (is_class(REG_CLASS_RM_YMM, opndflags)) {
        /* TODO */
    } else if (is_class(REG_CLASS_RM_ZMM, opndflags)) {
        /* TODO */
    } else if (is_class(REG_CLASS_BND, opndflags)) {
        /* TODO */
    } else if (is_class(REG_CLASS_RM_TMM, opndflags)) {
        /* TODO */
    } else {
        nasm_fatal("OPFLAGS: register optype without register class");
    }
    return false;
}

static bool gen_immediate(operand_seed *opnd_seed, char *buffer)
{
    opflags_t opndflags;
    opndflags = opnd_seed->opndflags;

    if (is_class(UNITY, opndflags)) {
        if (stat_get_need_init()) {
            init_immediate_opnd(buffer, opnd_seed);
            return true;
        }
        return create_unity(opnd_seed, buffer);
    } else if (is_class(SBYTEDWORD, opndflags)) {
        /* TODO */
    } else if (is_class(SBYTEWORD, opndflags)) {
        /* TODO */
    } else if (is_class(IMMEDIATE, opndflags)) {
        if (stat_get_need_init()) {
            init_immediate_opnd(buffer, opnd_seed);
            return true;
        }
        return create_immediate(opnd_seed, buffer);
    } else {
        nasm_fatal("OPFLAGS: not immediate optype");
    }
    return false;
}

static bool gen_reg_mem(operand_seed *opnd_seed, char *buffer)
{
    opflags_t opndflags;
    opndflags = opnd_seed->opndflags;

    if (is_class(MEMORY, opndflags)) {
        stat_set_has_mem_opnd(true);
        if (is_class(MEM_OFFS, opndflags)) {
            return create_memoffs(opnd_seed, buffer);
        } else {
            return create_memory(opnd_seed, buffer);
        }
    } else {
        bool select_mem = likely_happen_p(0.5);
        if (select_mem) {
            stat_set_has_mem_opnd(true);
            opnd_seed->opndflags = (opnd_seed->opndflags & ~OPTYPE_MASK) | MEMORY;
            return create_memory(opnd_seed, buffer);
        } else {
            opnd_seed->opndflags = (opnd_seed->opndflags & ~OPTYPE_MASK) | REGISTER;
            return gen_register(opnd_seed, buffer);
        }
    }
    return false;
}

/* Generate operand. */
static bool gen_operand_internal(operand_seed *opnd_seed, char *buffer)
{
    opflags_t opndflags;
    opndflags = opnd_seed->opndflags;

    if (is_class(REGISTER, opndflags)) {
        /* REGISTER condition must be judged before REGMEM
         */
        return gen_register(opnd_seed, buffer);
    } else if (is_class(IMMEDIATE, opndflags)) {
        return gen_immediate(opnd_seed, buffer);
    } else if (is_class(REGMEM, opndflags)) {
        return gen_reg_mem(opnd_seed, buffer);
    } else {
        nasm_fatal("Wrong operand odflags with no optype");
    }
    return false;

//TODO:    case IMMEDIATE|COLON:
//TODO:    case IMMEDIATE|BITS16|COLON:
//TODO:    case IMMEDIATE|BITS16|NEAR:
//TODO:    case IMMEDIATE|BITS32|COLON:
//TODO:    case IMMEDIATE|BITS32|NEAR:
//TODO:    case IMMEDIATE|SHORT:
//TODO:    case IMMEDIATE|NEAR:


//TODO:    case MEMORY|BITS8:
//TODO:    case MEMORY|BITS16:
//TODO:    case MEMORY|BITS32:
//TODO:    case MEMORY|FAR:
}

/******************************************************************************
*
* Function name: gen_operand
* Description: generate an assembly operand into token_bufptr
*              We can generate it from these cases:
*              1. instruction seed
*              2. an existed variable
*              3. pseudo code of operand type
*              4. pseudo code of operand
* Parameter:
*       @seed: instruction seed
*       @opi: operand index in current instruction
*       @label_consumer: true if current instruction use an assembly label as
*       it's operand
* Return: success or fail
******************************************************************************/
bool gen_operand(const insn_seed *seed, int opi, bool *label_consumer)
{
    operand_seed opnd_seed;
    char asm_opnd[128];

    init_opnd_seed(&opnd_seed);
    stat_set_opi(opi);
    char *opnd_start = nasm_skip_spaces(get_token_bufptr());

    if (seed == NULL) {  /* pseudo code */
        char *next_opnd;
        char asm_pseudo_opnd[128];
        size_t opnd_len = copy_asm_opnd(opnd_start, asm_pseudo_opnd);

        create_opnd_seed(&opnd_seed, asm_pseudo_opnd);

        if (opnd_seed.is_var) {  /* variable */
            blk_var *var = blk_search_var(stat_get_curr_blk(), asm_pseudo_opnd);
            if (!var->valid) {
                /* generate variable and store it to var->var_val
                 */
                if (!gen_operand_internal(&opnd_seed, asm_opnd))
                    return false;
                var->opndflags = opnd_seed.opndflags;
                var->var_val = nasm_strdup(asm_opnd);
                if (is_class(MEMORY, var->opndflags))
                    var->init_mem_addr = nasm_strdup(stat_get_init_mem_addr());
                var->valid = true;
            }

            nasm_strrplc(opnd_start, opnd_len, var->var_val, strlen(var->var_val));

            /* specify the fundamental data item size for a memory operand
             */
            next_opnd = nasm_skip_a_comma(opnd_start);
            if (opi == 0 && is_class(MEMORY, var->opndflags) &&
                (asm_is_blank(next_opnd) || asm_is_immediate(next_opnd))) {
                preappend_mem_size(opnd_start, var->opndflags & SIZE_MASK);
            }

            init_opnd(asm_opnd, &opnd_seed, var);
        } else if (opnd_seed.is_opnd_type) {  /* pseudo code, operand's type  */
            if (!gen_operand_internal(&opnd_seed, asm_opnd))
                return false;
            nasm_strrplc(opnd_start, opnd_len, asm_opnd, strlen(asm_opnd));

            /* specify the fundamental data item size for a memory operand
             */
            next_opnd = nasm_skip_a_comma(opnd_start);
            if (opi == 0 && is_class(MEMORY, opnd_seed.opndflags) &&
                (asm_is_blank(next_opnd) || asm_is_immediate(next_opnd))) {
                preappend_mem_size(opnd_start, opnd_seed.opdsize);
            }

            init_opnd(asm_opnd, &opnd_seed, NULL);
        } else { /* pseudo code, a valid operand */
            init_opnd(asm_pseudo_opnd, &opnd_seed, NULL);
        }
    } else {  /* instruction seed */
        if (seed->opd[opi] == 0)
            return true;

        opnd_seed.opndflags = seed->opd[opi];
        opnd_seed.srcdestflags = calSrcDestFlags(seed, opi);
        opnd_seed.opdsize = calOperandSize(seed, opi);

        if (opi != 0) {
            gen_comma(opnd_start);
            set_token_bufptr(++opnd_start);
        }

        if (!gen_operand_internal(&opnd_seed, asm_opnd))
            return false;
        nasm_strrplc(opnd_start, 0, asm_opnd, strlen(asm_opnd));

        /* specify the fundamental data item size for a memory operand
         */
        if (opi == 0 && is_class(MEMORY, opnd_seed.opndflags) &&
            (seed->opd[1] == 0 || is_class(IMMEDIATE, seed->opd[1]))) {
            preappend_mem_size(opnd_start, opnd_seed.opdsize);
        }

        if (is_label_consumer(&opnd_seed)) {
            *label_consumer = true;
        }

        init_opnd(asm_opnd, &opnd_seed, NULL);
    }
    return true;
}

void gendata_init(void)
{
    srand((unsigned)time(NULL));
}
