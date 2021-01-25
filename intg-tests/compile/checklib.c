#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "verification.h"

struct X86StandardRegisters output[] =
{
#include "stdoutput.rst"
    {}
};

static int diffs = 0;
static int point = 0;

#define check_point_gprhi(gprhi) void check_point_##gprhi\
    (struct X87LegacyXSaveArea x87fpstate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    Reg8 std_##gprhi = x86regs.gprhi; \
    Reg8 check_##gprhi = output[point].gprhi; \
    if (std_##gprhi != check_##gprhi) { \
        printf("diff ["#gprhi"]: 0x%x, should be: 0x%x\n", check_##gprhi, std_##gprhi); \
        diff = 1; \
    } \
    printf("check point: %d %s! ["#gprhi"]\n", point + 1, (diff == 1) ? "fail" : "pass"); \
    diffs += diff; \
    point++; \
}

check_point_gprhi(ah)
check_point_gprhi(bh)
check_point_gprhi(ch)
check_point_gprhi(dh)

#define check_point_gprlo(gprlo) void check_point_##gprlo\
    (struct X87LegacyXSaveArea x87fpstate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    Reg8 std_##gprlo = x86regs.gprlo; \
    Reg8 check_##gprlo = output[point].gprlo; \
    if (std_##gprlo != check_##gprlo) { \
        printf("diff ["#gprlo"]: 0x%x, should be: 0x%x\n", check_##gprlo, std_##gprlo); \
        diff = 1; \
    } \
    printf("check point: %d %s! ["#gprlo"]\n", point + 1, (diff == 1) ? "fail" : "pass"); \
    diffs += diff; \
    point++; \
}

check_point_gprlo(al)
check_point_gprlo(bl)
check_point_gprlo(cl)
check_point_gprlo(dl)

#define check_point_reg16(reg16) void check_point_##reg16\
    (struct X87LegacyXSaveArea x87fpstate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    Reg16 std_##reg16 = x86regs.reg16; \
    Reg16 check_##reg16 = output[point].reg16; \
    if (std_##reg16 != check_##reg16) { \
        printf("diff ["#reg16"]: 0x%x, should be: 0x%x\n", check_##reg16, std_##reg16); \
        diff = 1; \
    } \
    printf("check point: %d %s! ["#reg16"]\n", point + 1, (diff == 1) ? "fail" : "pass"); \
    diffs += diff; \
    point++; \
}

check_point_reg16(ax)
check_point_reg16(bx)
check_point_reg16(cx)
check_point_reg16(dx)
check_point_reg16(si)
check_point_reg16(di)
check_point_reg16(sp)
check_point_reg16(bp)

check_point_reg16(cs)
check_point_reg16(ss)
check_point_reg16(ds)
check_point_reg16(es)
check_point_reg16(fs)
check_point_reg16(gs)

#define check_point_reg32(reg32) void check_point_##reg32\
    (struct X87LegacyXSaveArea x87fpstate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    Reg32 std_##reg32 = x86regs.reg32; \
    Reg32 check_##reg32 = output[point].reg32; \
    if (std_##reg32 != check_##reg32) { \
        printf("diff ["#reg32"]: 0x%x, should be: 0x%x\n", check_##reg32, std_##reg32); \
        diff = 1; \
    } \
    printf("check point: %d %s! ["#reg32"]\n", point + 1, (diff == 1) ? "fail" : "pass"); \
    diffs += diff; \
    point++; \
}

check_point_reg32(eax)
check_point_reg32(ebx)
check_point_reg32(ecx)
check_point_reg32(edx)
check_point_reg32(esi)
check_point_reg32(edi)
check_point_reg32(esp)
check_point_reg32(ebp)
check_point_reg32(eflags)

void check_point_x86_state(struct X87LegacyXSaveArea x87fpstate,
     struct X86StandardRegisters x86regs)
{
    int diff = 0;
    if (x86regs.gs != output[point].gs) {
        printf("diff [gs]: %x, should be: 0x%x\n", x86regs.gs, output[point].gs);
        diff = 1;
    }
    if (x86regs.fs != output[point].fs) {
        printf("diff [fs]: %x, should be: 0x%x\n", x86regs.fs, output[point].fs);
        diff = 1;
    }
    if (x86regs.es != output[point].es) {
        printf("diff [es]: %x, should be: 0x%x\n", x86regs.es, output[point].es);
        diff = 1;
    }
    if (x86regs.ds != output[point].ds) {
        printf("diff [ds]: %x, should be: 0x%x\n", x86regs.ds, output[point].ds);
        diff = 1;
    }
    if (x86regs.ss != output[point].ss) {
        printf("diff [ss]: %x, should be: 0x%x\n", x86regs.ss, output[point].ss);
        diff = 1;
    }
    if (x86regs.cs != output[point].cs) {
        printf("diff [cs]: %x, should be: 0x%x\n", x86regs.cs, output[point].cs);
        diff = 1;
    }
    if (x86regs.eflags != output[point].eflags) {
        printf("diff [eflags]: %x, should be: 0x%x\n", x86regs.eflags, output[point].eflags);
        diff = 1;
    }
    if (x86regs.edi != output[point].edi) {
        printf("diff [edi]: %x, should be: 0x%x\n", x86regs.edi, output[point].edi);
        diff = 1;
    }
    if (x86regs.esi != output[point].esi) {
        printf("diff [esi]: %x, should be: 0x%x\n", x86regs.esi, output[point].esi);
        diff = 1;
    }
    if (x86regs.ebx != output[point].ebx) {
        printf("diff [ebx]: %x, should be: 0x%x\n", x86regs.ebx, output[point].ebx);
        diff = 1;
    }
    if (x86regs.edx != output[point].edx) {
        printf("diff [edx]: %x, should be: 0x%x\n", x86regs.edx, output[point].edx);
        diff = 1;
    }
    if (x86regs.ecx != output[point].ecx) {
        printf("diff [ecx]: %x, should be: 0x%x\n", x86regs.ecx, output[point].ecx);
        diff = 1;
    }
    if (x86regs.eax != output[point].eax) {
        printf("diff [eax]: %x, should be: 0x%x\n", x86regs.eax, output[point].eax);
        diff = 1;
    }
    printf("check point: %d %s! [x86_state]\n", point + 1, (diff == 1) ? "fail" : "pass"); \
    diffs += diff;
    point++;
}

void check_point_end(void)
{
    if (diffs != 0) {
        printf("=====[%d] in [%d] check points fail!=====\n", diffs, point);
    } else {
        printf("=====[%d] check points pass!=====\n", point);
    }
    fclose(stdout);
}
