#ifndef NASM_REG_H
#define NASM_REG_H

#include "regs.h"

static inline bool is_reg_ax(enum reg_enum reg)
{
    return reg == R_AL || reg == R_AH || reg == R_AX || reg == R_EAX;
}

static inline bool is_reg_bx(enum reg_enum reg)
{
    return reg == R_BL || reg == R_BH || reg == R_BX || reg == R_EBX;
}

static inline bool is_reg_cx(enum reg_enum reg)
{
    return reg == R_CL || reg == R_CH || reg == R_CX || reg == R_ECX;
}

static inline bool is_reg_dx(enum reg_enum reg)
{
    return reg == R_DL || reg == R_DH || reg == R_DX || reg == R_EDX;
}

static inline bool is_reg_si(enum reg_enum reg)
{
    return reg == R_SI || reg == R_ESI;
}

static inline bool is_reg_di(enum reg_enum reg)
{
    return reg == R_DI || reg == R_EDI;
}

#endif
