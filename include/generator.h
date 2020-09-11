#ifndef NASM_GENERATOR_H
#define NASM_GENERATOR_H

#include "nasm.h"

void generator_init(void);
void generator_exit(void);
void generate(insn *output_ins);

#endif
