#ifndef NASM_GENERATOR_H
#define NASM_GENERATOR_H

#include "insns.h"

void generator_init(bool set_sequence);
void generator_exit(void);
bool one_insn_gen(const insn_seed *seed, insn *result);
bool one_insn_gen_const(const const_insn_seed *const_seed, insn *result);
void insn_to_bin(insn *instruction, const char** buf);
void insn_to_asm(insn *instruction, const char** buf);

#endif
