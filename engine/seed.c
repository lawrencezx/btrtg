#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "seed.h"
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
    opnd_seed->opndflags = 0;
    opnd_seed->srcdestflags = 0;
    opnd_seed->opdsize = 0;
    opnd_seed->explicitmemsize = false;
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

static opflags_t parse_opflags(const char *asm_opnd)
{
    opflags_t opflags = 0;
    char opnd_id[128];
    size_t i = 0, k = 0, opnd_size = 0;

    while (isalpha(asm_opnd[k])) {
        opnd_id[i++] = asm_opnd[k++];
    }
    opnd_id[i++] = '\0';

    struct {
        const char *id;
        opflags_t opndflags;
    } opnd_ids[] = {
        {"reg", REG_GPR},
        {"mem", MEMORY},
        {"memory_offs", MEM_OFFS},
        {"imm", IMMEDIATE},
        {"rm", RM_GPR},
        {"unity", UNITY},
        {"sbyteword", SBYTEWORD},
        {"sbytedword", SBYTEDWORD},
        {"reg_sreg", REG_SREG},
        {"reg_creg", REG_CREG},
        {"reg_dreg", REG_DREG},
    };

    for (i = 0; i < ARRAY_SIZE(opnd_ids); i++) {
        if (strcmp(opnd_ids[i].id, opnd_id) == 0) {
            opflags = opnd_ids[i].opndflags;
            break;
        }
    }
    if (i == ARRAY_SIZE(opnd_ids))
        return opflags;

    while (isdigit(asm_opnd[k])) {
        opnd_size = opnd_size*10 + asm_opnd[k++] - '0';
    }
    switch (opnd_size) {
        case 8:
            opflags |= BITS8;
            break;
        case 16:
            opflags |= BITS16;
            break;
        case 32:
            opflags |= BITS32;
            break;
        case 64:
            opflags |= BITS64;
            break;
        case 80:
            opflags |= BITS80;
            break;
        case 128:
            opflags |= BITS128;
            break;
        case 256:
            opflags |= BITS256;
            break;
        case 512:
            opflags |= BITS512;
            break;
        default:
            break;
    }
    return opflags;
}

void create_opnd_seed(operand_seed *opnd_seed, const char *asm_opnd)
{
    init_opnd_seed(opnd_seed);
    opnd_seed->opndflags = parse_opflags(asm_opnd);
    opnd_seed->opdsize = opnd_seed->opndflags & SIZE_MASK;

}
