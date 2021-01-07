#ifndef NASM_OPERAND_H
#define NASM_OPERAND_H

bool create_specific_register(enum reg_enum R_reg, operand_seed *opnd_seed, char *buffer);
bool create_control_register(operand_seed *opnd_seed, char *buffer);
bool create_segment_register(operand_seed *opnd_seed, char *buffer);
bool create_unity(operand_seed *opnd_seed, char *buffer);
bool create_gpr_register(operand_seed *opnd_seed, char *buffer);
bool create_immediate(operand_seed *opnd_seed, char *buffer);
bool create_memory(operand_seed *opnd_seed, char *buffer);
bool create_memoffs(operand_seed *opnd_seed, char *buffer);
bool init_specific_register(enum reg_enum R_reg, bool isDest);
char *preappend_mem_size(char *asm_mem, opflags_t opdsize);

#endif
