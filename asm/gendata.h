#ifndef NASM_GENDATA_H
#define NASM_GENDATA_H

void gendata_init(void);
void gen_op(enum opcode opcode, char *buffer);
void gen_opnd(opflags_t operand, char *buffer);

#endif
