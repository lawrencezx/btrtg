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
,0x%08" PRIx32 "\
,0x%08" PRIx32 "},\n"

#define  X87RegsOutputFormat "{\
0x%04" PRIx16 "\
,0x%04" PRIx16 "\
,0x%04" PRIx16 "\
,0x%08" PRIx32 "\
,0x%08" PRIx32 "\
,0x%04" PRIx16 "\
,0x%016" PRIx64 "\
,0x%04" PRIx16 "\
,0x%016" PRIx64 "\
,0x%04" PRIx16 "\
,0x%016" PRIx64 "\
,0x%04" PRIx16 "\
,0x%016" PRIx64 "\
,0x%04" PRIx16 "\
,0x%016" PRIx64 "\
,0x%04" PRIx16 "\
,0x%016" PRIx64 "\
,0x%04" PRIx16 "\
,0x%016" PRIx64 "\
,0x%04" PRIx16 "\
,0x%016" PRIx64 "\
,0x%04" PRIx16 "}"

void parse_argv(void) {}

#define DEFINE_CHECK_FUNCTION(nasm,type,func) void func \
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    printf("{"); \
    printf(X86RegsOutputFormat, \
        x86regs.gs, x86regs.fs, x86regs.es, x86regs.ds, x86regs.ss, x86regs.cs, \
        x86regs.eflags, \
        x86regs.pc, \
        x86regs.edi, x86regs.esi, x86regs.ebp, x86regs.esp, x86regs.ebx, x86regs.edx, x86regs.ecx, x86regs.eax); \
    printf(X87RegsOutputFormat, \
        fsa_get_fcw(&x87fpustate), fsa_get_fsw(&x87fpustate), fsa_get_ftw(&x87fpustate), \
        fsa_get_ffdp(&x87fpustate), fsa_get_ffip(&x87fpustate), fsa_get_ffop(&x87fpustate), \
        fsa_get_st(&x87fpustate, 0).low, fsa_get_st(&x87fpustate, 0).high, \
        fsa_get_st(&x87fpustate, 1).low, fsa_get_st(&x87fpustate, 1).high, \
        fsa_get_st(&x87fpustate, 2).low, fsa_get_st(&x87fpustate, 2).high, \
        fsa_get_st(&x87fpustate, 3).low, fsa_get_st(&x87fpustate, 3).high, \
        fsa_get_st(&x87fpustate, 4).low, fsa_get_st(&x87fpustate, 4).high, \
        fsa_get_st(&x87fpustate, 5).low, fsa_get_st(&x87fpustate, 5).high, \
        fsa_get_st(&x87fpustate, 6).low, fsa_get_st(&x87fpustate, 6).high, \
        fsa_get_st(&x87fpustate, 7).low, fsa_get_st(&x87fpustate, 7).high); \
    printf("},\n"); \
}

#include "checkfunctions.h"
#undef DEFINE_CHECK_FUNCTION

void check_point_end(void)
{
    fclose(stdout);
}

