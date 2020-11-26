#include "compiler.h"


#include "nasm.h"
#include "assemble.h"
#include "insns.h"
#include "nctype.h"
#include "seed.h"
#include "generator.h"
#include "gendata.h"
#include "disasm.h"
#include "buf2token.h"
#include "insns.h"
#include "eval.h"
#include "operand.h"
#include "insnlist.h"
#include "bseqi.h"
#include "x86pg.h"

bool global_sequence;

int globalrel = 0;
int globalbnd = 0;

extern FILE *error_file;            /* Where to write error messages */

static int cmd_sb = 32;             /* by default */

iflag_t cpu;
static iflag_t cmd_cpu;

struct location location;
bool in_absolute;                   /* Flag we are in ABSOLUTE seg */
struct location absolute;           /* Segment/offset inside ABSOLUTE */

insn output_ins;
char global_codebuf[MAX_INSLEN];
uint8_t global_codebuf_len;
static char codestr[256];

static struct tokenval tokval;

int64_t switch_segment(int32_t segment)
{
    location.segment = segment;
    if (segment == NO_SEG) {
        location.offset = absolute.offset;
        in_absolute = true;
    } else {
        in_absolute = false;
    }
    return location.offset;
}

static void set_curr_offs(int64_t l_off)
{
        if (in_absolute)
            absolute.offset = l_off;
}

static void increment_offset(int64_t delta)
{
    if (unlikely(delta == 0))
        return;

    location.offset += delta;
    set_curr_offs(location.offset);
}

void generator_init(bool set_sequence)
{
    error_file = stderr;

    iflag_set_default_cpu(&cpu);
    iflag_set_default_cpu(&cmd_cpu);

    /* Save away the default state of warnings */
    init_warnings();

    init_x86pgstate();
    X86PGState.seqMode = set_sequence;

    gendata_init();

    nasm_ctype_init();

    switch (cmd_sb) {
    case 16:
        break;
    case 32:
        if (!iflag_cpu_level_ok(&cmd_cpu, IF_386))
            nasm_fatal("command line: 32-bit segment size requires a higher cpu");
        break;
    case 64:
        if (!iflag_cpu_level_ok(&cmd_cpu, IF_X86_64))
            nasm_fatal("command line: 64-bit segment size requires a higher cpu");
        break;
    default:
        panic();
        break;
    }

    globalbits = cmd_sb;  /* set 'bits' to command line default */
    cpu = cmd_cpu;

    in_absolute = false;
    location.segment = NO_SEG;
    location.offset  = 0;
}

void generator_exit(void)
{
    src_free();
}

void insn_to_bin(insn *instruction, const char** buf)
{
    int64_t l;

    if (!instruction->times)
        return;                 /* Nothing to do... */

    global_codebuf_len = 0;

    nasm_assert(instruction->times > 0);

    l = assemble(location.segment, location.offset,
                 globalbits, instruction);
    /* We can't get an invalid instruction here */
    increment_offset(l);

    *buf = (const char*)global_codebuf;
}

void insn_to_asm(insn *instruction, const char** buf)
{
    if (instruction == NULL)
        return ;
    const char* binbuf;
    insn_to_bin(instruction, &binbuf);

    iflag_t prefer;
    disasm((uint8_t *)global_codebuf, (int32_t)global_codebuf_len, (char *)codestr, sizeof(codestr),
            globalbits, &prefer);
    *buf = (const char*)codestr;
}

static inline void init_operand(operand *op)
{
    memset(op, 0, sizeof *op);

    op->basereg  = -1;
    op->indexreg = -1;
    op->segment  = NO_SEG;
    op->wrt      = NO_SEG;
}

bool one_insn_gen(const insn_seed *seed, insn *result)
{
#ifdef DEBUG_MODE
    fprintf(stderr, "Gen inst: %s\n", nasm_insn_names[seed->opcode]);
#endif
    int opnum = 0;
    char valbuf[20];

    memset(result->prefixes, P_none, sizeof(result->prefixes));
    result->times       = 1;
    result->label       = NULL;
    result->eops        = NULL;
    result->operands    = 0;
    result->evex_rm     = 0;
    result->evex_brerop = -1;

    gen_opcode(seed->opcode, (char *)valbuf);
    buf2token(valbuf, &tokval);

    result->opcode = tokval.t_integer;
    result->condition = tokval.t_inttwo;

    while (opnum < MAX_OPERANDS && seed->opd[opnum] != 0)
        opnum++;

    if (X86PGState.seqMode && !bseqi_inc(&X86PGState.bseqi, seed, opnum))
        return false;

    for (int i = 0; i < opnum; ++i) {
        expr *value;
        bool mref = false;
        operand *op;
        operand_seed opnd_seed;

        op = &result->oprs[i];
        init_operand(op);

        opnd_seed.opcode = seed->opcode;
        opnd_seed.opndflags = seed->opd[i];
        opnd_seed.srcdestflags = calSrcDestFlags(seed->opcode, i, opnum);
        opnd_seed.opdsize = calOperandSize(seed, i);
        gen_operand(&opnd_seed, (char *)valbuf);
        buf2token(valbuf, &tokval);

        op->type = 0;

        /* mref_more: */
        if (mref) {
          /* TODO */
        }

        value = evaluate(NULL, NULL, &tokval,
                         &op->opflags, false, NULL);

        if (mref) {
          /* TODO */
        } else {
            if (is_reloc(value)) {          /* it's immediate */
                uint64_t n = reloc_value(value);

                op->type       |= IMMEDIATE;
                op->offset     = n;
                op->segment    = reloc_seg(value);
                op->wrt        = reloc_wrt(value);
                op->opflags    |= is_self_relative(value) ? OPFLAG_RELATIVE : 0;
            } else {                        /* it's a register */
                if (value->type >= EXPR_SIMPLE || value->value != 1) {
                    nasm_nonfatal("invalid operand type");
                    goto fail;
                }

                op->type       &= TO;
                op->type       |= REGISTER;
                op->type       |= nasm_reg_flags[value->type];
                op->basereg    = value->type;
            }
        }
    }

    result->operands = opnum; /* set operand count */

    /* clear remaining operands */
    while (opnum < MAX_OPERANDS)
        result->oprs[opnum++].type = 0;

    return true;

fail:
    /* TODO */
    return true;
}

bool one_insn_gen_const(const const_insn_seed *const_seed, insn *result)
{
#ifdef DEBUG_MODE
    fprintf(stderr, "Gen const inst: %s\n", nasm_insn_names[const_seed->insn_seed.opcode]);
#endif
    int i, opnum = 0;
    char valbuf[20];
    const insn_seed* seed;

    memset(result->prefixes, P_none, sizeof(result->prefixes));
    result->times       = 1;
    result->label       = NULL;
    result->eops        = NULL;
    result->operands    = 0;
    result->evex_rm     = 0;
    result->evex_brerop = -1;

    seed = &const_seed->insn_seed;

    gen_opcode(seed->opcode, (char *)valbuf);
    buf2token(valbuf, &tokval);

    result->opcode = tokval.t_integer;
    result->condition = tokval.t_inttwo;

    while (opnum < MAX_OPERANDS && seed->opd[opnum] != 0)
        opnum++;

    for (i = 0; i < opnum; ++i) {
        if (const_seed->oprs_random[i] == false) {
            result->oprs[i] = const_seed->oprs[i];
            continue;
        }

        expr *value;
        bool mref = false;
        operand *op;
        operand_seed opnd_seed;

        op = &result->oprs[i];
        init_operand(op);

        opnd_seed.opcode = seed->opcode;
        opnd_seed.opndflags = seed->opd[i];
        opnd_seed.srcdestflags = calSrcDestFlags(seed->opcode, i, opnum);
        opnd_seed.opdsize = calOperandSize(seed, i);
        gen_operand(&opnd_seed, (char *)valbuf);
        buf2token(valbuf, &tokval);

        op->type = 0;

        /* mref_more: */
        if (mref) {
          /* TODO */
        }

        value = evaluate(NULL, NULL, &tokval,
                         &op->opflags, false, NULL);

        if (mref) {
          /* TODO */
        } else {
            if (is_reloc(value)) {          /* it's immediate */
                uint64_t n = reloc_value(value);

                op->type       |= IMMEDIATE;
                op->offset     = n;
                op->segment    = reloc_seg(value);
                op->wrt        = reloc_wrt(value);
                op->opflags    |= is_self_relative(value) ? OPFLAG_RELATIVE : 0;
            } else {                        /* it's a register */
                if (value->type >= EXPR_SIMPLE || value->value != 1) {
                    nasm_nonfatal("invalid operand type");
                    goto fail;
                }

                op->type       &= TO;
                op->type       |= REGISTER;
                op->type       |= nasm_reg_flags[value->type];
                op->basereg    = value->type;
            }
        }
    }

    result->operands = opnum; /* set operand count */

    /* clear remaining operands */
    while (i < MAX_OPERANDS)
        result->oprs[i++].type = 0;

    return true;

fail:
    /* TODO */
    return true;
}
