#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "verification.h"

#define  X86RegsOutputFormat "{\
0x%04" PRIx16 ",0\
,0x%04" PRIx16 ",0\
,0x%04" PRIx16 ",0\
,0x%04" PRIx16 ",0\
,0x%04" PRIx16 ",0\
,0x%04" PRIx16 ",0\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "},\n"

void print_x86_output(struct X86StandardRegisters x86regs)
{
    printf(X86RegsOutputFormat,
        x86regs.gs, x86regs.fs, x86regs.es, x86regs.ds, x86regs.ss, x86regs.cs,
        x86regs.eflags,
        x86regs.edi, x86regs.esi, x86regs.ebp, x86regs.esp, x86regs.ebx, x86regs.edx, x86regs.ecx, x86regs.eax);
}

void print_all_state(struct X87LegacyXSaveArea x87fpstate, struct X86StandardRegisters x86regs)
{
    print_x86_output(x86regs);
}

void finish_test(void)
{
    fclose(stdout);
}

