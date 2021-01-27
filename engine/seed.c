#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "seed.h"
#include "tmplt.h"
#include "x86pg.h"
#include "asmlib.h"
#include <ctype.h>

static int itemplate_size(const struct itemplate *itmplt)
{
    int size = 0;
    for (const struct itemplate *temp = itmplt;
        temp->opcode != I_none; temp++, size++)
        ;
    return size;
}

void init_opnd_seed(operand_seed *opnd_seed)
{
    opnd_seed->is_var = false;
    opnd_seed->is_opnd_type = false;
    opnd_seed->opndflags = 0;
    opnd_seed->srcdestflags = 0;
    opnd_seed->opdsize = 0;
}

void create_insn_seed(insn_seed *seed, const char *instname)
{
    struct tokenval tv;
    nasm_token_hash(instname, &tv);
    seed->opcode = tv.t_integer;
    const struct itemplate *itmplt = nasm_instructions[seed->opcode];
    int operandsi = nasm_random32(itemplate_size(itmplt));
    memcpy(seed->opd, itmplt[operandsi].opd, sizeof(itmplt[operandsi].opd));
}

/******************************************************************************
*
* Function name: create_opnd_seed
* Description: make an operand seed from the given pseudo operand string
*              The operand can be:
*              (1) a variable: like @var1 <reg8>, @var2 <imm32>, etc.
*              (2) an operand type: like imm32, reg8, etc.
*              (3) an operand: like al, 0xffff, etc.
* Parameter:
*       @opnd_seed: return, the operand seed
*       @asm_opnd: input, the pseudo operand string
* Return: none
******************************************************************************/
void create_opnd_seed(operand_seed *opnd_seed, const char *asm_opnd)
{
    if (asm_opnd == NULL)
        return;

    if (*asm_opnd == '@') {
        opnd_seed->is_var = true;
        struct blk_var *var = blk_search_var(stat_get_curr_blk(), asm_opnd);
        if (var == NULL)
            nasm_fatal("var: %s not defined\n", asm_opnd);
        if (!var->valid)
            opnd_seed->opndflags = parse_asm_opnd_type_opflags(var->var_type);
    } else {
        opnd_seed->opndflags = parse_asm_opnd_type_opflags(asm_opnd);
        if (opnd_seed->opndflags != 0)
            opnd_seed->is_opnd_type = true;
        else
            opnd_seed->opndflags = parse_asm_opnd_opflags(asm_opnd);
    }
    opnd_seed->opdsize = opnd_seed->opndflags & SIZE_MASK;
}
