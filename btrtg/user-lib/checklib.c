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
    struct floatx80 fpregs[8];
};
struct SSERegs{
    uint32_t mxcsr;
    uint32_t mxcsr_mask;
    xmmreg xmmregs[8];
};

struct output{
    struct X86StandardRegisters X86;
    struct X87Regs X87;
    struct SSERegs SSE;
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
#define ERROR "1.08420217248550443400745280086994171e-19"
//#define ERROR "1.19209289550781250000000000000000000e-7F"
#define PRINT_ERROR_INFO 0

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
    (struct SSEStateSaveArea* ssestate, \
     struct X87LegacyFPUSaveArea x87fpustate, \
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
    (struct SSEStateSaveArea* ssestate, \
     struct X87LegacyFPUSaveArea x87fpustate, \
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
    (struct SSEStateSaveArea* ssestate, \
     struct X87LegacyFPUSaveArea x87fpustate, \
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


extern char * fxstate;


#define check_point_reg32(reg32) void check_point_##reg32\
    (struct SSEStateSaveArea* ssestate, \
     struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    extern unsigned int phone_num;\
    int diff = 0; \
    Reg32 check_##reg32 = x86regs.reg32; \
    Reg32 std_##reg32 = output[point].X86.reg32; \
    if (std_##reg32 != check_##reg32) { \
        if(PRINT_ERROR_INFO) \
            printf("diff ["#reg32"]: 0x%x, should be: 0x%x\n", check_##reg32, std_##reg32); \
            diff = 1; \
    } \
    if (diff == 1 && PRINT_ERROR_INFO) \
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

void display_fpustack(struct X87LegacyFPUSaveArea *x87fpustate)
{
    for(int i = 0; i < 8; i++){
        struct floatx80 check_fpureg = fsa_get_st(x87fpustate, i);
        struct floatx80 std_fpureg = output[point].X87.fpregs[i];
        printf("diff [st%d]: 0x%08x %08x %04x, should be: 0x%08x %08x %04x\n", i,
        ((int *)&check_fpureg)[0], ((int *)&check_fpureg)[1], check_fpureg.high,
        ((int *)&std_fpureg)[0], ((int *)&std_fpureg)[1], std_fpureg.high);
    }
    printf("\n");
}
#define check_point_fpureg(fpureg, index) void check_point_##fpureg\
    (struct SSEStateSaveArea* ssestate, \
     struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    struct floatx80 check_##fpureg = fsa_get_st(&x87fpustate, index); \
    struct floatx80 std_##fpureg = output[point].X87.fpregs[index]; \
    long double check_float_num = *(long double*)&check_##fpureg; \
    long double std_float_num = *(long double*)&std_##fpureg; \
    /*if (abs(check_float_num - std_float_num) > strtold( ERROR, NULL)) { */\
    if (std_##fpureg.low != check_##fpureg.low || std_##fpureg.high != check_##fpureg.high) { \
        printf("diff ["#fpureg"]: high[0x%04x] low[0x%016lx], should be: high[0x%04x] low[0x%016lx]\n", \
                check_##fpureg.high, check_##fpureg.low, \
                std_##fpureg.high, std_##fpureg.low); \
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
    (struct SSEStateSaveArea* ssestate, \
     struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    struct floatx80 check_##mmxreg = fsa_get_st(&x87fpustate, index); \
    struct floatx80 std_##mmxreg = output[point].X87.fpregs[index]; \
    if (std_##mmxreg.low != check_##mmxreg.low) { \
        printf("diff ["#mmxreg"]: 0x%08x %08x %04x, should be: 0x%08x %08x %04x\n", \
        ((int *)&check_##mmxreg)[0], ((int *)&check_##mmxreg)[1], check_##mmxreg.high, \
        ((int*)&std_##mmxreg)[0], ((int*)&std_##mmxreg)[1], std_##mmxreg.high); \
        diff = 1; \
    } \
    if (diff == 1 && PRINT_ERROR_INFO) \
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
    (struct SSEStateSaveArea* ssestate, \
     struct X87LegacyFPUSaveArea x87fpustate, \
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

#define check_point_x87status16(x87status, mask) void check_point_##x87status\
    (struct SSEStateSaveArea* ssestate, \
     struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    uint16_t check_##x87status = fsa_get_##x87status(&x87fpustate); \
    uint16_t std_##x87status = output[point].X87.x87status; \
    if (std_##x87status & mask != check_##x87status & mask) { \
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

check_point_x87status16(fcw, 0x1f3f)
check_point_x87status16(fsw, 0xffff)
check_point_x87status16(ftw, 0xffff)
check_point_x87status16(ffop, 0xffff)

void check_point_ccode(struct SSEStateSaveArea* ssestate, 
                           struct X87LegacyFPUSaveArea x87fpustate,
                           struct X86StandardRegisters x86regs)
{
    int diff = 0; 
    uint32_t check_fsw = fsa_get_fsw(&x87fpustate); 
    uint32_t std_fsw = output[point].X87.fsw;
    uint32_t check_ccode = (check_fsw >> 8) & 0x0007;
    uint32_t std_ccode = (check_fsw >> 8) & 0x0007;
    if (check_ccode != std_ccode) { 
        printf("diff [ccode]: 0x%x, should be: 0x%x\n", check_ccode, std_ccode); 
        diff = 1; 
    } 
    if (diff == 1) 
        printf("check point: %d fail! [ccode][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); 
    else if (verbose == 1) 
        printf("check point: %d pass! [ccode]\n", point + 1); 
    diffs += diff; 
    point++; 
}

#define check_point_xmmreg(one_xmmreg, index) void check_point_##one_xmmreg\
    (struct SSEStateSaveArea* ssestate,\
     struct X87LegacyFPUSaveArea x87fpustate, \
     struct X86StandardRegisters x86regs) \
{ \
    int diff = 0; \
    xmmreg check_##one_xmmreg = fsa_get_xmm(ssestate, index); \
    xmmreg std_##one_xmmreg = output[point].SSE.xmmregs[index]; \
    if (std_##one_xmmreg.low != check_##one_xmmreg.low || std_##one_xmmreg.high != check_##one_xmmreg.high) { \
        if(PRINT_ERROR_INFO) \
            printf("diff ["#one_xmmreg"]: 0x%08x %08x %08x %08x, \nshould be: 0x%08x %08x %08x %08x\n", \
                ((int *)&check_##one_xmmreg)[0], ((int *)&check_##one_xmmreg)[1], \
                ((int *)&check_##one_xmmreg)[2], ((int *)&check_##one_xmmreg)[3], \
                ((int *)&std_##one_xmmreg)[0], ((int *)&std_##one_xmmreg)[1], \
                ((int *)&std_##one_xmmreg)[2], ((int *)&std_##one_xmmreg)[3]);\
        diff = 1; \
    } \
    if (diff == 1 && PRINT_ERROR_INFO) \
        printf("check point: %d fail! ["#one_xmmreg"][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); \
    else if (verbose == 1) \
        printf("check point: %d pass! ["#one_xmmreg"]\n", point + 1); \
    diffs += diff; \
    point++; \
}

check_point_xmmreg(xmm0, 0)
check_point_xmmreg(xmm1, 1)
check_point_xmmreg(xmm2, 2)
check_point_xmmreg(xmm3, 3)
check_point_xmmreg(xmm4, 4)
check_point_xmmreg(xmm5, 5)
check_point_xmmreg(xmm6, 6)
check_point_xmmreg(xmm7, 7)

void check_point_mxcsr(struct SSEStateSaveArea* ssestate, 
                           struct X87LegacyFPUSaveArea x87fpustate,
                           struct X86StandardRegisters x86regs)
{
    int diff = 0; 
    uint32_t check_mxcsr = fsa_get_mxcsr(ssestate); 
    uint32_t std_mxcsr = output[point].SSE.mxcsr; 
    if (check_mxcsr != std_mxcsr) { 
        printf("diff [mxcsr]: 0x%x, should be: 0x%x\n", check_mxcsr, std_mxcsr); 
        diff = 1; 
    } 
    if (diff == 1) 
        printf("check point: %d fail! [mxcsr][fuzzy_pc:0x%x]\n", point + 1, x86regs.pc); 
    else if (verbose == 1) 
        printf("check point: %d pass! [mxcsr]\n", point + 1); 
    diffs += diff; 
    point++; 
}

void check_point_x86_state(struct SSEStateSaveArea* ssestate, 
                           struct X87LegacyFPUSaveArea x87fpustate,
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

void check_point_x87_env(struct SSEStateSaveArea* ssestate, \
                         struct X87LegacyFPUSaveArea x87fpustate,
                         struct X86StandardRegisters x86regs)
{
    int diff = 0;
    if (fsa_get_fcw(&x87fpustate) & 0x1f3f != output[point].X87.fcw & 0x1f3f) {
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
