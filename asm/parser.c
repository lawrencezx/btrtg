#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "eval.h"
#include "parser.h"
#include "gendata.h"
#include "buf2token.h"

static struct tokenval tokval;

static inline void init_operand(operand *op)
{
    memset(op, 0, sizeof *op);

    op->basereg  = -1;
    op->indexreg = -1;
    op->segment  = NO_SEG;
    op->wrt      = NO_SEG;
}

bool parse_insn_seed(insn_seed *seed, insn *result)
{
    int opnum = 0;
    char valbuf[20];

    memset(result->prefixes, P_none, sizeof(result->prefixes));
    result->times       = 1;
    result->label       = NULL;
    result->eops        = NULL;
    result->operands    = 0;
    result->evex_rm     = 0;
    result->evex_brerop = -1;

    gen_op(seed->opcode, (char *)valbuf);
    buf2token(valbuf, &tokval);

    result->opcode = tokval.t_integer;
    result->condition = tokval.t_inttwo;

    while (opnum < MAX_OPERANDS && seed->opd[opnum] != 0)
        opnum++;

    if (!sqi_inc(seed, opnum))
        return false;

    for (int i = 0; i < opnum; ++i) {
        expr *value;
        bool mref = false;

        operand *op = &result->oprs[i];

        init_operand(op);

        gen_opnd(seed->opd[i], (char *)valbuf);
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
