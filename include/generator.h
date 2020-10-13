#ifndef NASM_GENERATOR_H
#define NASM_GENERATOR_H

void generator_init(bool set_sequence);
void generator_exit(void);
uint32_t generate(insn_seed *seed, const char** buf);

#endif
