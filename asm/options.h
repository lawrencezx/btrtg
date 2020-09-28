#ifndef NASM_OPTIONS_H
#define NASM_OPTIONS_H

#include <stdbool.h>

extern bool option_display_insn;

static inline void options_init(void)
{
    option_display_insn = false;
}

static inline void set_display_insn(bool val)
{
    option_display_insn = val;
}

#endif
