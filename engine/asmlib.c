#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "opflags.h"
#include "nctype.h"
#include "asmlib.h"

opflags_t parse_asm_opnd_type_opflags(const char *asm_opnd)
{
    opflags_t opflags = 0;
    char opnd_id[128];
    size_t i = 0, k = 0, opnd_size = 0;

    while (asm_opnd[k] && !isdigit(asm_opnd[k])) {
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
        {"fpureg", FPUREG},
        {"fpu", FPU0},
        {"mmxreg", MMXREG},
        {"mmxmem", MEMORY|REG_CLASS_RM_MMX},
        {"mmxrm", RM_MMX},
        {"xmmreg", XMMREG},
        {"xmmmem", MEMORY|REG_CLASS_RM_XMM}
        /* TODO */
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

opflags_t parse_asm_opnd_opflags(const char *asm_opnd)
{
/* TODO: only register type supported */
    opflags_t opflags = 0;
    struct tokenval tv;
    nasm_token_hash(asm_opnd, &tv);
    if (tv.t_type == TOKEN_REG)
        opflags = nasm_reg_flags[tv.t_integer];
    return opflags;
}

bool asm_is_blank(const char *asm_opnd)
{
    if (asm_opnd) {
        asm_opnd = nasm_skip_spaces(asm_opnd);
        if (*asm_opnd == '\0' || *asm_opnd == '\n')
            return true;
    }
    return false;
}

bool asm_is_immediate(const char *asm_opnd)
{
    if (asm_opnd == NULL)
        return false;
    asm_opnd = nasm_skip_spaces(asm_opnd);
    if (nasm_isnumstart(*asm_opnd))
        return true;
    if (is_class(IMMEDIATE, parse_asm_opnd_type_opflags(asm_opnd)))
        return true;
    return false;
}

size_t copy_asm_opnd(const char *src, char *dst)
{
    size_t cnt = 0;
    while (*src != '\0' && *src != '\n' && *src != ',') {
        *dst++ = *src++;
        cnt++;
    }
    *dst = '\0';
    return cnt;
}

enum opcode parse_asm_opcode(char *asm_opcode)
{
    struct tokenval tv;
    nasm_token_hash(asm_opcode, &tv);
    return tv.t_integer;
}
