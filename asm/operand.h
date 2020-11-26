#ifndef NASM_OPERAND_H
#define NASM_OPERAND_H

void create_specific_register(char *buffer, enum reg_enum R_reg);
void create_control_register(char *buffer);
void create_segment_register(char *buffer);
void create_unity(char *buffer, operand_seed *opnd_seed);
void create_gpr_register(char *buffer, operand_seed *opnd_seed);
void create_immediate(char *buffer, operand_seed *opnd_seed);

#endif
