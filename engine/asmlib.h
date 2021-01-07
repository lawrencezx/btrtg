#ifndef NASM_ASMLIB_H
#define NASM_ASMLIB_H

opflags_t asm_parse_opflags(const char *asm_opnd);
bool asm_is_blank(const char *asm_opnd);
bool asm_is_immediate(const char *asm_opnd);

#endif
