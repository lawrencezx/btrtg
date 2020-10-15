#ifndef NASM_GENERATOR_H
#define NASM_GENERATOR_H

#include "insns.h"

void generator_init(bool set_sequence);
void generator_exit(void);
uint32_t generate_bin(insn_seed *seed, const char** buf);
const char* generate_str(insn_seed *seed);

#endif
