#ifndef NASM_CTRL_H
#define NASM_CTRL_H

void likely_gen_label(void);
int gen_control_transfer_insn(enum opcode opcode, int times);
void gen_control_transfer_finish(void);
void update_insert_pos_to_label(int label);

#endif
