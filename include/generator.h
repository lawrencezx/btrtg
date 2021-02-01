#ifndef NASM_GENERATOR_H
#define NASM_GENERATOR_H

#include "x86pg.h"
#include "insns.h"
#include "seed.h"

void generator_init(bool set_sequence);
void generator_exit(void);
bool one_insn_gen(const insn_seed *seed, insn *result);
bool one_insn_gen_const(char *asm_buffer);
bool one_insn_gen_ctrl(char *asm_buffer, enum position pos);
void end_insn_gen(void);
void insn_to_bin(insn *instruction, const char** buf);
void insn_to_asm(insn *instruction, const char** buf);

#endif
