#ifndef NASM_GENERATOR_H
#define NASM_GENERATOR_H

void generator_init(void);
void generator_exit(void);
void generate(insn_seed *seed, insn *output_ins);

#endif
