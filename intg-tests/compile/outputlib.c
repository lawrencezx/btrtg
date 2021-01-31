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

#define  X87RegsOutputFormat "{\
0x%04" PRIx64 "\
,0x%04" PRIx16 "\
,0x%04" PRIx64 "\
,0x%04" PRIx16 "\
,0x%04" PRIx64 "\
,0x%04" PRIx16 "\
,0x%08" PRIx64 "\
,0x%08" PRIx16 "\
,0x%08" PRIx64 "\
,0x%08" PRIx16 "\
,0x%08" PRIx64 "\
,0x%08" PRIx16 "\
,0x%08" PRIx64 "\
,0x%08" PRIx16 "\
,0x%08" PRIx64 "\
,0x%08" PRIx16 "}"

#define DEFINE_CHECK_FUNCTION(nasm,type,func) void func \
    (struct X87LegacyXSaveArea x87fpstate, \
     struct X86StandardRegisters x86regs) \
{ \
    printf("{"); \
    printf(X86RegsOutputFormat, \
        x86regs.gs, x86regs.fs, x86regs.es, x86regs.ds, x86regs.ss, x86regs.cs, \
        x86regs.eflags, \
        x86regs.edi, x86regs.esi, x86regs.ebp, x86regs.esp, x86regs.ebx, x86regs.edx, x86regs.ecx, x86regs.eax); \
    printf(X87RegsOutputFormat, \
        x87fpstate.fpregs[0].d.low, x87fpstate.fpregs[0].d.high, \
        x87fpstate.fpregs[1].d.low, x87fpstate.fpregs[1].d.high, \
        x87fpstate.fpregs[2].d.low, x87fpstate.fpregs[2].d.high, \
        x87fpstate.fpregs[3].d.low, x87fpstate.fpregs[3].d.high, \
        x87fpstate.fpregs[4].d.low, x87fpstate.fpregs[4].d.high, \
        x87fpstate.fpregs[5].d.low, x87fpstate.fpregs[5].d.high, \
        x87fpstate.fpregs[6].d.low, x87fpstate.fpregs[6].d.high, \
        x87fpstate.fpregs[7].d.low, x87fpstate.fpregs[7].d.high); \
    printf("},\n"); \
}

#include "checkfunctions.h"
#undef DEFINE_CHECK_FUNCTION

void check_point_end(void)
{
    fclose(stdout);
}

