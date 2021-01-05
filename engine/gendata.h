#ifndef NASM_GENDATA_H
#define NASM_GENDATA_H

void gendata_init(void);
bool gen_opcode(const insn_seed *seed);
bool gen_operand(const insn_seed *seed, int opi, bool *label_consumer);

#endif
