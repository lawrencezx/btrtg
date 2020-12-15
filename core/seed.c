#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "seed.h"

static int itemplate_size(const struct itemplate *itmplt)
{
    int size = 0;
    for (const struct itemplate *temp = itmplt;
        temp->opcode != I_none; temp++, size++)
        ;
    return size;
}

void create_insn_seed(insn_seed *seed, const char *instname)
{
    struct tokenval tv;
    nasm_token_hash(instname, &tv);
    seed->opcode = tv.t_integer;
    seed->instname = nasm_strdup(instname);
    const struct itemplate *itmplt = nasm_instructions[seed->opcode];
    int operandsi = nasm_random32(itemplate_size(itmplt));
    memcpy(seed->opd, itmplt[operandsi].opd, sizeof(itmplt[operandsi].opd));
}
