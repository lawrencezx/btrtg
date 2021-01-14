#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "verification.h"

#define  X86RegsFormat "==========X86 REGISTERS==========\n\
[eax]       0x%08" PRIx32 "\n\
[ecx]       0x%08" PRIx32 "\n\
[edx]       0x%08" PRIx32 "\n\
[ebx]       0x%08" PRIx32 "\n\
[esp]       0x%08" PRIx32 "\n\
[ebp]       0x%08" PRIx32 "\n\
[esi]       0x%08" PRIx32 "\n\
[edi]       0x%08" PRIx32 "\n\
[eflags]    0x%08" PRIx32 "\n\
[cs]        0x%04" PRIx16 "\n\
[ss]        0x%04" PRIx16 "\n\
[ds]        0x%04" PRIx16 "\n\
[es]        0x%04" PRIx16 "\n\
[fs]        0x%04" PRIx16 "\n\
[gs]        0x%04" PRIx16 "\n"

#define X87FPFormat "==========X87 FP STATE==========\n\
[ FCW | FSW  | FTW|Rsvd |  FOP  |     FIP     |  FCS  | Rsvd  ]  \n\
0x%04" PRIx16 " 0x%04" PRIx16 " 0x%02" PRIx8 "  -    0x%04" PRIx16 "   0x%08" PRIx32 "    0x%04" PRIx16 "   -- \n\
[    FDP     |   FDS    | Rsvd  |    MXCSR    |    MXCSR_MASK ] \n\
  0x%08" PRIx32 "    0x%04" PRIx16 "     --     0x%08" PRIx32 "      0x%08" PRIx32 "\n\
[ST0] 0x%04" PRIx16 "%016" PRIx64 "\n\
[ST1] 0x%04" PRIx16 "%016" PRIx64 "\n\
[ST2] 0x%04" PRIx16 "%016" PRIx64 "\n\
[ST3] 0x%04" PRIx16 "%016" PRIx64 "\n\
[ST4] 0x%04" PRIx16 "%016" PRIx64 "\n\
[ST5] 0x%04" PRIx16 "%016" PRIx64 "\n\
[ST6] 0x%04" PRIx16 "%016" PRIx64 "\n\
[ST7] 0x%04" PRIx16 "%016" PRIx64 "\n\
[XMM0] 0x%016" PRIx64 "%016" PRIx64 "\n\
[XMM1] 0x%016" PRIx64 "%016" PRIx64 "\n\
[XMM2] 0x%016" PRIx64 "%016" PRIx64 "\n\
[XMM3] 0x%016" PRIx64 "%016" PRIx64 "\n\
[XMM4] 0x%016" PRIx64 "%016" PRIx64 "\n\
[XMM5] 0x%016" PRIx64 "%016" PRIx64 "\n\
[XMM6] 0x%016" PRIx64 "%016" PRIx64 "\n\
[XMM7] 0x%016" PRIx64 "%016" PRIx64 "\n"

void print_all_state(struct X87LegacyXSaveArea x87fpstate, struct X86StandardRegisters x86regs)
{
    printf(X86RegsFormat,
        x86regs.eax, x86regs.ecx, x86regs.edx, x86regs.ebx, x86regs.esp, x86regs.ebp, x86regs.esi, x86regs.edi,
        x86regs.eflags,
        x86regs.cs, x86regs.ss, x86regs.ds, x86regs.es, x86regs.fs, x86regs.gs);
    printf(X87FPFormat,
        x87fpstate.fcw, x87fpstate.fsw, x87fpstate.ftw, x87fpstate.fpop, x87fpstate.fpip, x87fpstate.fpcs,
        x87fpstate.fpdp, x87fpstate.fpds, x87fpstate.mxcsr, x87fpstate.mxcsr_mask,
        x87fpstate.fpregs[0].d.high, x87fpstate.fpregs[0].d.low,
        x87fpstate.fpregs[1].d.high, x87fpstate.fpregs[1].d.low,
        x87fpstate.fpregs[2].d.high, x87fpstate.fpregs[2].d.low,
        x87fpstate.fpregs[3].d.high, x87fpstate.fpregs[3].d.low,
        x87fpstate.fpregs[4].d.high, x87fpstate.fpregs[4].d.low,
        x87fpstate.fpregs[5].d.high, x87fpstate.fpregs[5].d.low,
        x87fpstate.fpregs[6].d.high, x87fpstate.fpregs[6].d.low,
        x87fpstate.fpregs[7].d.high, x87fpstate.fpregs[7].d.low,
        *(uint64_t *)&x87fpstate.xmm_regs[0][8], *(uint64_t *)&x87fpstate.xmm_regs[0][0], 
        *(uint64_t *)&x87fpstate.xmm_regs[1][8], *(uint64_t *)&x87fpstate.xmm_regs[1][0], 
        *(uint64_t *)&x87fpstate.xmm_regs[2][8], *(uint64_t *)&x87fpstate.xmm_regs[2][0], 
        *(uint64_t *)&x87fpstate.xmm_regs[3][8], *(uint64_t *)&x87fpstate.xmm_regs[3][0], 
        *(uint64_t *)&x87fpstate.xmm_regs[4][8], *(uint64_t *)&x87fpstate.xmm_regs[4][0], 
        *(uint64_t *)&x87fpstate.xmm_regs[5][8], *(uint64_t *)&x87fpstate.xmm_regs[5][0], 
        *(uint64_t *)&x87fpstate.xmm_regs[6][8], *(uint64_t *)&x87fpstate.xmm_regs[6][0], 
        *(uint64_t *)&x87fpstate.xmm_regs[7][8], *(uint64_t *)&x87fpstate.xmm_regs[7][0]);
    printf("\n");
}

void finish_test(void)
{
    fclose(stdout);
}

