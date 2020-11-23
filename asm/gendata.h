#ifndef NASM_GENDATA_H
#define NASM_GENDATA_H

void gendata_init(bool set_sequence);
void gen_opcode(enum opcode opcode, char *buffer);
void gen_operand(operand_seed *opnd_seed, char *buffer, bool force_random);
bool sqi_inc(const insn_seed *seed, int opnum);

#endif
