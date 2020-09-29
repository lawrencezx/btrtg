#ifndef NASM_GENERATOR_H
#define NASM_GENERATOR_H

void generator_init(void);
void generator_exit(void);
uint32_t generate(insn_seed *seed, const char** buf);

#endif
