#ifndef NASM_ASMLIB_H
#define NASM_ASMLIB_H

opflags_t parse_asm_opnd_type_opflags(const char *asm_opnd);
opflags_t parse_asm_opnd_opflags(const char *asm_opnd);
bool asm_is_blank(const char *asm_opnd);
bool asm_is_immediate(const char *asm_opnd);
size_t copy_asm_opnd(const char *src, char *dst);
enum opcode parse_asm_opcode(char *asm_opcode);

#endif
