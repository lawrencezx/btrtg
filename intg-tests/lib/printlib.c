#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

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
[cs]        0x%08" PRIx32 "\n\
[ss]        0x%08" PRIx32 "\n\
[ds]        0x%08" PRIx32 "\n\
[es]        0x%08" PRIx32 "\n\
[fs]        0x%08" PRIx32 "\n\
[gs]        0x%08" PRIx32 "\n"

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

typedef uint32_t float32;
typedef uint64_t float64;

#define MMREG_UNION(n, bits)        \
    union n {                       \
        uint8_t  _b_##n[(bits)/8];  \
        uint16_t _w_##n[(bits)/16]; \
        uint32_t _l_##n[(bits)/32]; \
        uint64_t _q_##n[(bits)/64]; \
        float32  _s_##n[(bits)/32]; \
        float64  _d_##n[(bits)/64]; \
    }

typedef MMREG_UNION(MMXReg, 64)  MMXReg;

typedef struct {
    uint64_t low;
    uint16_t high;
} floatx80;


typedef union {
    floatx80 d __attribute__((aligned(16)));
    MMXReg mmx;
} FPReg;

struct X87LegacyXSaveArea {
    uint16_t fcw;
    uint16_t fsw;
    uint8_t ftw;
    uint8_t reserved0_5;
    uint16_t fpop;
    uint32_t fpip;
    uint16_t fpcs;
    uint16_t reserved0_14;
    uint32_t fpdp;
    uint16_t fpds;
    uint16_t reserved16_6;
    uint32_t mxcsr;
    uint32_t mxcsr_mask;
    FPReg fpregs[8];
    uint8_t xmm_regs[8][16];
    uint8_t reverved[11][16];
    uint8_t available[3][16];
};

struct X86StandardRegisters {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t ss;
    uint32_t cs;
    uint32_t eflags;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
};

void print_x86_state(struct X86StandardRegisters x86regs)
{
    printf(X86RegsFormat,
        x86regs.eax, x86regs.ecx, x86regs.edx, x86regs.ebx, x86regs.esp, x86regs.ebp, x86regs.esi, x86regs.edi,
        x86regs.eflags,
        x86regs.cs, x86regs.ss, x86regs.ds, x86regs.es, x86regs.fs, x86regs.gs);
}

void print_x87_state(struct X87LegacyXSaveArea x87fpstate)
{
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
}

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
}
