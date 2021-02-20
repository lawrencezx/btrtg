#ifndef NASM_CTRL_H
#define NASM_CTRL_H

void likely_gen_label(void);
bool gen_control_transfer_insn(enum opcode opcode);
void gen_control_transfer_finish(void);

#endif
