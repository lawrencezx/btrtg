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
#include "xmlparser/parseXML.h"
#include "x86pg.h"
#include "dfmt.h"
#include "tk.h"
#include "tmplt.h"
#include "ctrl.h"

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

    nasm_ctype_init();

    /* Save away the default state of warnings */
    init_warnings();

    init_x86pgstate();
    X86PGState.seqMode = set_sequence;

    init_tks();

    init_tmplts();

    gendata_init();

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
    tks_free_all();
    wdtrees_free_all();
    tmplt_clear(&tmpltm);
    token_cleanup();
}

void insn_to_bin(insn *instruction, const char** buf)
{
    int64_t l;

    if (!instruction->times)
        return;                 /* Nothing to do... */

    global_codebuf_len = 0;
    memset(global_codebuf, 0, sizeof(global_codebuf));

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

static int parse_mref(operand *op, const expr *e)
{
    int b, i, s;        /* basereg, indexreg, scale */
    int64_t o;          /* offset */

    b = op->basereg;
    i = op->indexreg;
    s = op->scale;
    o = op->offset;

    for (; e->type; e++) {
        if (e->type <= EXPR_REG_END) {
            bool is_gpr = is_class(REG_GPR,nasm_reg_flags[e->type]);

            if (is_gpr && e->value == 1 && b == -1) {
                /* It can be basereg */
                b = e->type;
            } else if (i == -1) {
                /* Must be index register */
                i = e->type;
                s = e->value;
            } else {
                if (b == -1)
                    nasm_nonfatal("invalid effective address: two index registers");
                else if (!is_gpr)
                    nasm_nonfatal("invalid effective address: impossible register");
                else
                    nasm_nonfatal("invalid effective address: too many registers");
                return -1;
            }
        } else if (e->type == EXPR_UNKNOWN) {
            op->opflags |= OPFLAG_UNKNOWN;
        } else if (e->type == EXPR_SIMPLE) {
            o += e->value;
        } else if  (e->type == EXPR_WRT) {
            op->wrt = e->value;
        } else if (e->type >= EXPR_SEGBASE) {
            if (e->value == 1) {
                if (op->segment != NO_SEG) {
                    nasm_nonfatal("invalid effective address: multiple base segments");
                    return -1;
                }
                op->segment = e->type - EXPR_SEGBASE;
            } else if (e->value == -1 &&
                       e->type == location.segment + EXPR_SEGBASE &&
                       !(op->opflags & OPFLAG_RELATIVE)) {
                op->opflags |= OPFLAG_RELATIVE;
            } else {
                nasm_nonfatal("invalid effective address: impossible segment base multiplier");
                return -1;
            }
        } else {
            nasm_nonfatal("invalid effective address: bad subexpression type");
            return -1;
        }
   }

    op->basereg  = b;
    op->indexreg = i;
    op->scale    = s;
    op->offset   = o;
    return 0;
}

static void mref_set_optype(operand *op)
{
    int b = op->basereg;
    int i = op->indexreg;
    int s = op->scale;

    /* It is memory, but it can match any r/m operand */
    op->type |= MEMORY_ANY;

    if (b == -1 && (i == -1 || s == 0)) {
        int is_rel = globalbits == 64 &&
            !(op->eaflags & EAF_ABS) &&
            ((globalrel &&
              !(op->eaflags & EAF_FSGS)) ||
             (op->eaflags & EAF_REL));

        op->type |= is_rel ? IP_REL : MEM_OFFS;
    }

    if (i != -1) {
        opflags_t iclass = nasm_reg_flags[i];

        if (is_class(XMMREG,iclass))
            op->type |= XMEM;
        else if (is_class(YMMREG,iclass))
            op->type |= YMEM;
        else if (is_class(ZMMREG,iclass))
            op->type |= ZMEM;
    }
}

bool one_insn_gen(const insn_seed *seed, insn *result)
{
    if (seed != NULL)
        dfmt->print("\033[35m Gen inst: %s \033[m\n", nasm_insn_names[seed->opcode]);
    else
        dfmt->print("\033[31m Gen const inst\033[m: %s\n", get_token_bufptr());

    token_reset();

    if (gen_control_transfer_insn(seed))
        return true;
    likely_gen_label();

    int i;
    int opi = 0;
    struct eval_hints hints;
    bool has_label_operand = false;

    memset(result->prefixes, P_none, sizeof(result->prefixes));
    result->times       = 1;
    result->ctrl        = NULL;
    result->eops        = NULL;
    result->operands    = 0;
    result->evex_rm     = 0;
    result->evex_brerop = -1;

    if (!gen_opcode(seed))
        return false;
    i = get_token(&tokval);

    if (stat_get_opcode() == I_none)
        stat_set_opcode(tokval.t_integer);

    result->opcode = tokval.t_integer;
    result->condition = tokval.t_inttwo;

//    if (seed != NULL && X86PGState.seqMode && !bseqi_inc(&X86PGState.bseqi, seed, opnum))
//        return false;

    while (true) {
        expr *value;
        bool mref = false;
        int bracket = 0;
        bool mib;
        int setsize = 0;
        operand *op;

        op = &result->oprs[opi];
        init_operand(op);

        stat_set_opi(opi);
        if (!gen_operand(seed, &has_label_operand))
            return false;
        i = get_token(&tokval);
        if (i == TOKEN_EOS)
            break;              /* end of operands: get out of here */

        op->type = 0;
        /* size specifiers */
        while (i == TOKEN_SPECIAL || i == TOKEN_SIZE) {
            switch (tokval.t_integer) {
            case S_BYTE:
                if (!setsize)   /* we want to use only the first */
                    op->type |= BITS8;
                setsize = 1;
                break;
            case S_WORD:
                if (!setsize)
                    op->type |= BITS16;
                setsize = 1;
                break;
            case S_DWORD:
            case S_LONG:
                if (!setsize)
                    op->type |= BITS32;
                setsize = 1;
                break;
            case S_QWORD:
                if (!setsize)
                    op->type |= BITS64;
                setsize = 1;
                break;
            case S_TWORD:
                if (!setsize)
                    op->type |= BITS80;
                setsize = 1;
                break;
            case S_OWORD:
                if (!setsize)
                    op->type |= BITS128;
                setsize = 1;
                break;
            case S_YWORD:
                if (!setsize)
                    op->type |= BITS256;
                setsize = 1;
                break;
            case S_ZWORD:
                if (!setsize)
                    op->type |= BITS512;
                setsize = 1;
                break;
            case S_TO:
                op->type |= TO;
                break;
            case S_STRICT:
                op->type |= STRICT;
                break;
            case S_FAR:
                op->type |= FAR;
                break;
            case S_NEAR:
                op->type |= NEAR;
                break;
            case S_SHORT:
                op->type |= SHORT;
                break;
            default:
                nasm_nonfatal("invalid operand size specification");
            }
            i = get_token(&tokval);
        }

        if (i == '[' || i == TOKEN_MASM_PTR || i == '&') {
            /* memory reference */
            mref = true;
            bracket += (i == '[');
            i = get_token(&tokval);
        }

        /* mref_more: */
        if (mref) {
          /* TODO */
        }

        value = evaluate(get_token, NULL, &tokval,
                         &op->opflags, false, &hints);
        i = tokval.t_type;

        mib = false;

        if (mref) {
            if (bracket == 1) {
                if (i == ']') {
                    bracket--;
                    i = get_token(&tokval);
                } else {
                    nasm_nonfatal("expecting ] at end of memory operand");
                }
            }
          /* TODO */
        }

        if (mref) {             /* it's a memory reference */
            /* A mib reference was fully parsed already */
            if (!mib) {
                if (parse_mref(op, value))
                    goto fail;
                op->hintbase = hints.base;
                op->hinttype = hints.type;
            }
            mref_set_optype(op);
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
        opi++;
    }

    result->operands = opi; /* set operand count */

    /* clear remaining operands */
    while (opi < MAX_OPERANDS)
        result->oprs[opi++].type = 0;
    if (has_label_operand) {
        result->ctrl = get_token_buf();
    }

    if (stat_get_has_mem_opnd())
        one_insn_gen_ctrl(stat_get_init_mem_addr(), INSERT_AFTER);

    stat_insert_insn(result, INSERT_AFTER);

    return true;

fail:
    nasm_fatal("fail to generate instruction");
    /* TODO */
    return true;
}

bool one_insn_gen_const(char *asm_buffer)
{
    bool sucess;
    insn const_inst;
    char buffer[128];
    char *old_bufptr;
    /* save the current contents in token_buf */
    old_bufptr = get_token_bufptr();
    strcpy(buffer, get_token_buf());

    strcpy(get_token_cbufptr(), asm_buffer);
    sucess = one_insn_gen(NULL, &const_inst);

    /* restore the contents in token_buf */
    strcpy(get_token_buf(), buffer);
    set_token_bufptr(old_bufptr);

    return sucess;
}

bool one_insn_gen_ctrl(char *asm_buffer, enum position pos)
{
    insn ctrl_inst;
    ctrl_inst.ctrl = asm_buffer;
    stat_insert_insn(&ctrl_inst, pos);
    return true;
}

void end_insn_gen(void)
{
    gen_control_transfer_finish();
}
