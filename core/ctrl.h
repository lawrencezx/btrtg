#ifndef NASM_CTRL_H
#define NASM_CTRL_H

void likely_gen_label(void);
bool gen_control_transfer_insn(const insn_seed *seed, insn *result);
void gen_control_transfer_finish(void);

#endif
