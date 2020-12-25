#include "compiler.h"

#include "regs.h"
#include "check.h"
#include "x86pg.h"

bool check_reg_valid(enum reg_enum gpr)
{
    bool valid = true;
    if (stat_edx_locked()) {
        valid = valid && (gpr != R_DL && gpr != R_DH && gpr != R_DX && gpr != R_EDX);
    }
    if (stat_ebx_locked()) {
        valid = valid && (gpr != R_BL && gpr != R_BH && gpr != R_BX && gpr != R_EBX);
    }
    if (stat_ecx_locked()) {
        valid = valid && (gpr != R_CL && gpr != R_CH && gpr != R_CX && gpr != R_ECX);
    }
    return valid;
}
