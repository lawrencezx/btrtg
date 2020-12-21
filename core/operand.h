#ifndef NASM_OPERAND_H
#define NASM_OPERAND_H

void create_specific_register(enum reg_enum R_reg, char *buffer);
void create_control_register(operand_seed *opnd_seed, char *buffer);
void create_segment_register(operand_seed *opnd_seed, char *buffer);
void create_unity(operand_seed *opnd_seed, char *buffer);
void create_gpr_register(operand_seed *opnd_seed, char *buffer);
void create_immediate(operand_seed *opnd_seed, char *buffer);
void create_memory(operand_seed *opnd_seed, char *buffer);
void init_specific_register(enum reg_enum R_reg, bool isSrc);

#endif
