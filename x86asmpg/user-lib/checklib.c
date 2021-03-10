#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "verification.h"

struct X87Regs{
    uint16_t fcw;
    uint16_t fsw;
    uint16_t ftw;
    uint32_t ffdp;
    uint32_t ffip;
    uint16_t ffop;
    floatx80 fpregs[8];
};
struct output{
    struct X86StandardRegisters X86;
    struct X87Regs X87;
};
struct output  output[] = 
{
#include "stdoutput.rst"
    {}
};
// struct X86StandardRegisters output[] =
// {
// #include "stdoutput.rst"
//     {}
// };

static int diffs = 0;
static int point = 0;

static int verbose = 0;

void parse_argv(void *main_addr, int argc, char const *argv[])
{
    if (argc == 2 &&
        *argv[1] != '\0' && *argv[1] == '-' &&
        *(argv[1] + 1) != '\0' && *(argv[1] + 1) == 'v')
        verbose = 1;
}

#define check_point_gprhi(gprhi) void check_point_##gprhi\
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    Reg8 check_##gprhi = x86regs.gprhi; \
    Reg8 std_##gprhi = output[point].X86.gprhi; \
    if (std_##gprhi != check_##gprhi) { \
        printf("diff ["#gprhi"]: 0x%x, should be: 0x%x\n", check_##gprhi, std_##gprhi); \
        diff = 1; \
    } \
    if (diff == 1) \
        printf("check point: %d fail! ["#gprhi"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#gprhi"]\n", point + 1); \
    diffs += diff; \
    point++; \
}

check_point_gprhi(ah)
check_point_gprhi(bh)
check_point_gprhi(ch)
check_point_gprhi(dh)

#define check_point_gprlo(gprlo) void check_point_##gprlo\
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    Reg8 check_##gprlo = x86regs.gprlo; \
    Reg8 std_##gprlo = output[point].X86.gprlo; \
    if (std_##gprlo != check_##gprlo) { \
        printf("diff ["#gprlo"]: 0x%x, should be: 0x%x\n", check_##gprlo, std_##gprlo); \
        diff = 1; \
    } \
    if (diff == 1) \
        printf("check point: %d fail! ["#gprlo"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#gprlo"]\n", point + 1); \
    diffs += diff; \
    point++; \
}

check_point_gprlo(al)
check_point_gprlo(bl)
check_point_gprlo(cl)
check_point_gprlo(dl)

#define check_point_reg16(reg16) void check_point_##reg16\
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    Reg16 check_##reg16 = x86regs.reg16; \
    Reg16 std_##reg16 = output[point].X86.reg16; \
    if (std_##reg16 != check_##reg16) { \
        printf("diff ["#reg16"]: 0x%x, should be: 0x%x\n", check_##reg16, std_##reg16); \
        diff = 1; \
    } \
    if (diff == 1) \
        printf("check point: %d fail! ["#reg16"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#reg16"]\n", point + 1); \
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
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    Reg32 check_##reg32 = x86regs.reg32; \
    Reg32 std_##reg32 = output[point].X86.reg32; \
    if (std_##reg32 != check_##reg32) { \
        printf("diff ["#reg32"]: 0x%x, should be: 0x%x\n", check_##reg32, std_##reg32); \
        diff = 1; \
    } \
    if (diff == 1) \
        printf("check point: %d fail! ["#reg32"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#reg32"]\n", point + 1); \
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

#define check_point_fpureg(fpureg, index) void check_point_##fpureg\
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    floatx80 check_##fpureg = fsa_get_st(&x87fpustate, index); \
    floatx80 std_##fpureg = output[point].X87.fpregs[index]; \
    if (std_##fpureg.low != check_##fpureg.low || std_##fpureg.high != check_##fpureg.high) { \
        printf("diff ["#fpureg"]: 0x%x, should be: 0x%x\n", check_##fpureg, std_##fpureg); \
        diff = 1; \
    } \
    if (diff == 1) \
        printf("check point: %d fail! ["#fpureg"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#fpureg"]\n", point + 1); \
    diffs += diff; \
    point++; \
}

check_point_fpureg(st0, 0)
check_point_fpureg(st1, 1)
check_point_fpureg(st2, 2)
check_point_fpureg(st3, 3)
check_point_fpureg(st4, 4)
check_point_fpureg(st5, 5)
check_point_fpureg(st6, 6)
check_point_fpureg(st7, 7)

#define check_point_mmxreg(mmxreg, index) void check_point_##mmxreg\
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    floatx80 check_##mmxreg = fsa_get_st(&x87fpustate, index); \
    floatx80 std_##mmxreg = output[point].X87.fpregs[index]; \
    if (std_##mmxreg.low != check_##mmxreg.low || std_##mmxreg.high != check_##mmxreg.high) { \
        printf("diff ["#mmxreg"]: 0x%x, should be: 0x%x\n", check_##mmxreg, std_##mmxreg); \
        diff = 1; \
    } \
    if (diff == 1) \
        printf("check point: %d fail! ["#mmxreg"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#mmxreg"]\n", point + 1); \
    diffs += diff; \
    point++; \
}

check_point_mmxreg(mm0, 0)
check_point_mmxreg(mm1, 1)
check_point_mmxreg(mm2, 2)
check_point_mmxreg(mm3, 3)
check_point_mmxreg(mm4, 4)
check_point_mmxreg(mm5, 5)
check_point_mmxreg(mm6, 6)
check_point_mmxreg(mm7, 7)

#define check_point_x87status32(x87status) void check_point_##x87status\
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    uint32_t check_##x87status = fsa_get_##x87status(&x87fpustate); \
    uint32_t std_##x87status = output[point].X87.x87status; \
    if (std_##x87status != check_##x87status) { \
        printf("diff ["#x87status"]: 0x%x, should be: 0x%x\n", check_##x87status, std_##x87status); \
        diff = 1; \
    } \
    if (diff == 1) \
        printf("check point: %d fail! ["#x87status"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#x87status"]\n", point + 1); \
    diffs += diff; \
    point++; \
}

check_point_x87status32(ffdp)
check_point_x87status32(ffip)

#define check_point_x87status16(x87status) void check_point_##x87status\
    (struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    uint16_t check_##x87status = fsa_get_##x87status(&x87fpustate); \
    uint16_t std_##x87status = output[point].X87.x87status; \
    if (std_##x87status != check_##x87status) { \
        printf("diff ["#x87status"]: 0x%x, should be: 0x%x\n", check_##x87status, std_##x87status); \
        diff = 1; \
    } \
    if (diff == 1) \
        printf("check point: %d fail! ["#x87status"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#x87status"]\n", point + 1); \
    diffs += diff; \
    point++; \
}

check_point_x87status16(fcw)
check_point_x87status16(fsw)
check_point_x87status16(ftw)
check_point_x87status16(ffop)

void check_point_x86_state(struct X87LegacyFPUSaveArea x87fpustate,
     struct X86StandardRegisters x86regs)
{
    int diff = 0;
    //if (x86regs.gs != output[point].X86.gs) {
    //    printf("diff [gs]: %x, should be: 0x%x\n", x86regs.gs, output[point].X86.gs);
    //    diff = 1;
    //}
    if (x86regs.fs != output[point].X86.fs) {
        printf("diff [fs]: %x, should be: 0x%x\n", x86regs.fs, output[point].X86.fs);
        diff = 1;
    }
    if (x86regs.es != output[point].X86.es) {
        printf("diff [es]: %x, should be: 0x%x\n", x86regs.es, output[point].X86.es);
        diff = 1;
    }
    if (x86regs.ds != output[point].X86.ds) {
        printf("diff [ds]: %x, should be: 0x%x\n", x86regs.ds, output[point].X86.ds);
        diff = 1;
    }
    if (x86regs.ss != output[point].X86.ss) {
        printf("diff [ss]: %x, should be: 0x%x\n", x86regs.ss, output[point].X86.ss);
        diff = 1;
    }
    if (x86regs.cs != output[point].X86.cs) {
        printf("diff [cs]: %x, should be: 0x%x\n", x86regs.cs, output[point].X86.cs);
        diff = 1;
    }
    if (x86regs.eflags != output[point].X86.eflags) {
        printf("diff [eflags]: %x, should be: 0x%x\n", x86regs.eflags, output[point].X86.eflags);
        diff = 1;
    }
    if (x86regs.edi != output[point].X86.edi) {
        printf("diff [edi]: %x, should be: 0x%x\n", x86regs.edi, output[point].X86.edi);
        diff = 1;
    }
    if (x86regs.esi != output[point].X86.esi) {
        printf("diff [esi]: %x, should be: 0x%x\n", x86regs.esi, output[point].X86.esi);
        diff = 1;
    }
    if (x86regs.ebx != output[point].X86.ebx) {
        printf("diff [ebx]: %x, should be: 0x%x\n", x86regs.ebx, output[point].X86.ebx);
        diff = 1;
    }
    if (x86regs.edx != output[point].X86.edx) {
        printf("diff [edx]: %x, should be: 0x%x\n", x86regs.edx, output[point].X86.edx);
        diff = 1;
    }
    if (x86regs.ecx != output[point].X86.ecx) {
        printf("diff [ecx]: %x, should be: 0x%x\n", x86regs.ecx, output[point].X86.ecx);
        diff = 1;
    }
    if (x86regs.eax != output[point].X86.eax) {
        printf("diff [eax]: %x, should be: 0x%x\n", x86regs.eax, output[point].X86.eax);
        diff = 1;
    }
    if (diff == 1)
        printf("check point: %d fail! [x86_state][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc);
    else if (verbose == 1)
        printf("check point: %d pass! [x86_state]\n", point + 1);
    diffs += diff;
    point++;
}

void check_point_x87_env(struct X87LegacyFPUSaveArea x87fpustate,
     struct X86StandardRegisters x86regs)
{
    int diff = 0;
    if (fsa_get_fcw(&x87fpustate) != output[point].X87.fcw) {
        printf("diff [fcw]: %x, should be: 0x%x\n", fsa_get_fcw(&x87fpustate), output[point].X87.fcw);
        diff = 1;
    }
    if (fsa_get_fsw(&x87fpustate) != output[point].X87.fsw) {
        printf("diff [fsw]: %x, should be: 0x%x\n", fsa_get_fsw(&x87fpustate), output[point].X87.fsw);
        diff = 1;
    }
    if (fsa_get_ftw(&x87fpustate) != output[point].X87.ftw) {
        printf("diff [ftw]: %x, should be: 0x%x\n", fsa_get_ftw(&x87fpustate), output[point].X87.ftw);
        diff = 1;
    }
    if (fsa_get_ffdp(&x87fpustate) != output[point].X87.ffdp) {
        printf("diff [ffdp]: %x, should be: 0x%x\n", fsa_get_ffdp(&x87fpustate), output[point].X87.ffdp);
        diff = 1;
    }
    if (fsa_get_ffip(&x87fpustate) != output[point].X87.ffip) {
        printf("diff [ffip]: %x, should be: 0x%x\n", fsa_get_ffip(&x87fpustate), output[point].X87.ffip);
        diff = 1;
    }
    if (fsa_get_ffop(&x87fpustate) != output[point].X87.ffop) {
        printf("diff [ffop]: %x, should be: 0x%x\n", fsa_get_ffop(&x87fpustate), output[point].X87.ffop);
        diff = 1;
    }
    if (diff == 1)
        printf("check point: %d fail! [x87_env][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc);
    else if (verbose == 1)
        printf("check point: %d pass! [x87_env]\n", point + 1);
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
