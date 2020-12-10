#ifndef NASM_GENDATA_H
#define NASM_GENDATA_H

srcdestflags_t calSrcDestFlags(const insn_seed *seed, int opi);
opflags_t calOperandSize(const insn_seed *seed, int opdi);
void gendata_init(void);
void gen_opcode(enum opcode opcode, char *buffer);
void gen_operand(operand_seed *opnd_seed, char *buffer);
void init_implied_operands(insn_seed *seed);

#endif
