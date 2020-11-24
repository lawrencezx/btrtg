#ifndef NASM_GENDATA_H
#define NASM_GENDATA_H

srcdestflags_t calSrcDestFlags(enum opcode op, int opnum, int operands);
void gendata_init(void);
void gen_opcode(enum opcode opcode, char *buffer);
void gen_operand(operand_seed *opnd_seed, char *buffer);

#endif
