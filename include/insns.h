/* insns.h   header file for insns.c
 *
 * The Netwide Assembler is copyright (C) 1996 Simon Tatham and
 * Julian Hall. All rights reserved. The software is
 * redistributable under the license given in the file "LICENSE"
 * distributed in the NASM archive.
 */

#ifndef NASM_INSNS_H
#define NASM_INSNS_H

#include "nasm.h"
#include "tokens.h"
#include "iflag.h"

typedef struct insn_seed {
    enum opcode     opcode;
    opflags_t       opd[MAX_OPERANDS];
} insn_seed;

/* if changed, ITEMPLATE_END should be also changed accordingly */
struct itemplate {
    enum opcode     opcode;             /* the token, passed from "parser.c" */
    int             operands;           /* number of operands */
    opflags_t       opd[MAX_OPERANDS];  /* bit flags for operand types */
    decoflags_t     deco[MAX_OPERANDS]; /* bit flags for operand decorators */
    const uint8_t   *code;              /* the code it assembles to */
    uint32_t        iflag_idx;          /* some flags referenced by index */
};

/* Use this helper to test instruction template flags */
static inline bool itemp_has(const struct itemplate *itemp, unsigned int bit)
{
    return iflag_test(&insns_flags[itemp->iflag_idx], bit);
}

/* Disassembler table structure */

/*
 * If n == -1, then p points to another table of 256
 * struct disasm_index, otherwise p points to a list of n
 * struct itemplates to consider.
 */
struct disasm_index {
    const void *p;
    int n;
};

/* Tables for the assembler and disassembler, respectively */
extern const struct itemplate * const nasm_instructions[];
extern const struct disasm_index itable[256];
extern const struct disasm_index * const itable_vex[NASM_VEX_CLASSES][32][4];

/* Common table for the byte codes */
extern const uint8_t nasm_bytecodes[];

/*
 * this define is used to signify the end of an itemplate
 */
#define ITEMPLATE_END {I_none,0,{0,},{0,},NULL,0}

/* Width of Dx and RESx instructions */

/*
 * initialized data bytes length from opcode
 */
static inline int const_func db_bytes(enum opcode opcode)
{
    switch (opcode) {
    case I_none:
        return -1;
    default:
        return 0;
    }
}

/*
 * Uninitialized data bytes length from opcode
 */
static inline int const_func resb_bytes(enum opcode opcode)
{
    switch (opcode) {
    case I_none:
        return -1;
    default:
        return 0;
    }
}

#endif /* NASM_INSNS_H */
