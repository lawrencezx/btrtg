#ifndef NASM_VERIFICATION_H
#define NASM_VERIFICATION_H

typedef uint32_t float32;
typedef uint64_t float64;

typedef struct {
    uint64_t low;
    uint16_t high;
} floatx80;

typedef struct {
    uint64_t low;
    uint64_t high;
} xmmreg;

struct X87LegacyFPUSaveArea {
    uint16_t fcw;
    uint16_t fcw_padding;
    uint16_t fsw;
    uint16_t fsw_padding;
    uint16_t ftw;
    uint16_t ftw_padding;
    uint16_t ffip_0;
    uint16_t ffip_padding;
    struct {
        uint32_t ffop:12;
        uint32_t ffip_16:16;
        uint32_t ffopfip_padding:4;
    };
    uint16_t ffdp_0;
    uint16_t ffdp_padding;
    struct {
        uint32_t ffdp_padding_pre:12;
        uint32_t ffdp_16:16;
        uint32_t ffdp_padding_after:4;
    };
    uint8_t st80[80];
};

// struct X87LegacyFPUSaveArea {
//     uint16_t fcw;
//     uint16_t fsw;
//     uint8_t ftw;
//     uint8_t reserved0_5;
//     uint16_t fpop;
//     uint32_t fpip;
//     uint16_t fpcs;
//     uint16_t reserved0_14;
//     uint32_t fpdp;
//     uint16_t fpds;
//     uint16_t reserved16_6;
//     uint32_t mxcsr;
//     uint32_t mxcsr_mask;
//     uint8_t st80[128];
//     uint8_t xmm_regs[128];
//     uint8_t reverved[11][16];
//     uint8_t available[3][16];
// };

struct SSEStateSaveArea {
    xmmreg xmm_regs[8];
    uint32_t mxcsr;
};

static inline uint16_t fsa_get_fcw(struct X87LegacyFPUSaveArea *x87fpustate)
{
    return x87fpustate->fcw;
}

static inline uint16_t fsa_get_fsw(struct X87LegacyFPUSaveArea *x87fpustate)
{
    return x87fpustate->fsw;
}

static inline uint16_t fsa_get_ftw(struct X87LegacyFPUSaveArea *x87fpustate)
{
    return x87fpustate->ftw;
}

static inline uint32_t fsa_get_ffdp(struct X87LegacyFPUSaveArea *x87fpustate)
{
    return ((uint32_t)x87fpustate->ffdp_0 | ((uint32_t)x87fpustate->ffdp_16 << 16));
}
// static inline uint32_t fsa_get_ffdp(struct X87LegacyFPUSaveArea *x87fpustate)
// {
//     return x87fpustate->fpdp;
// }

static inline uint32_t fsa_get_ffip(struct X87LegacyFPUSaveArea *x87fpustate)
{
    return ((uint32_t)x87fpustate->ffip_0 | ((uint32_t)x87fpustate->ffip_16 << 16));
}
// static inline uint32_t fsa_get_ffip(struct X87LegacyFPUSaveArea *x87fpustate)
// {
//     return x87fpustate->fpip;
// }

static inline uint16_t fsa_get_ffop(struct X87LegacyFPUSaveArea *x87fpustate)
{
    return x87fpustate->ffop;
}
// static inline uint16_t fsa_get_ffop(struct X87LegacyFPUSaveArea *x87fpustate)
// {
//     return x87fpustate->fpop;
// }

static inline floatx80 fsa_get_st(struct X87LegacyFPUSaveArea *x87fpustate, int i)
{
    return *((floatx80 *)&(x87fpustate->st80[i * 10]));
}

static inline uint32_t fsa_get_mxcsr(struct SSEStateSaveArea *ssestate)
{
    return ssestate->mxcsr;
}
// static inline uint32_t fsa_get_mxcsr(struct X87LegacyFPUSaveArea *x87fpustate)
// {
//     return x87fpustate->mxcsr;
// }

// static inline uint32_t fsa_get_mxcsr_mask(struct X87LegacyFPUSaveArea *x87fpustate)
// {
//     return x87fpustate->mxcsr_mask;
// }

static inline xmmreg fsa_get_xmm(struct SSEStateSaveArea *ssestate, int i)
{
    return *((xmmreg *)&(ssestate->xmm_regs[i]));
}
// static inline xmmreg fsa_get_xmm(struct X87LegacyFPUSaveArea *x87fpustate, int i)
// {
//     return *((xmmreg *)&(x87fpustate->xmm_regs[i * 16]));
// }

typedef uint8_t Reg8;
typedef uint16_t Reg16;
typedef uint32_t Reg32;

struct X86StandardRegisters {
    uint16_t gs;
    uint16_t gs_padding;
    uint16_t fs;
    uint16_t fs_padding;
    uint16_t es;
    uint16_t es_padding;
    uint16_t ds;
    uint16_t ds_padding;
    uint16_t ss;
    uint16_t ss_padding;
    uint16_t cs;
    uint16_t cs_padding;
    uint32_t eflags;
    uint32_t pc;
    union {
        uint32_t edi;
        struct {
            uint16_t di;
            uint16_t di_padding;
        };
    };
    union {
        uint32_t esi;
        struct {
            uint16_t si;
            uint16_t si_padding;
        };
    };
    union {
        uint32_t ebp;
        struct {
            uint16_t bp;
            uint16_t bp_padding;
        };
    };
    union {
        uint32_t esp;
        struct {
            uint16_t sp;
            uint16_t sp_padding;
        };
    };
    union {
        uint32_t ebx;
        struct {
            union {
                uint16_t bx;
                struct {
                    uint8_t bl;
                    uint8_t bh;
                };
            };
            uint16_t bx_padding;
        };
    };
    union {
        uint32_t edx;
        struct {
            union {
                uint16_t dx;
                struct {
                    uint8_t dl;
                    uint8_t dh;
                };
            };
            uint16_t dx_padding;
        };
    };
    union {
        uint32_t ecx;
        struct {
            union {
                uint16_t cx;
                struct {
                    uint8_t cl;
                    uint8_t ch;
                };
            };
            uint16_t cx_padding;
        };
    };
    union {
        uint32_t eax;
        struct {
            union {
                uint16_t ax;
                struct {
                    uint8_t al;
                    uint8_t ah;
                };
            };
            uint16_t ax_padding;
        };
    };
};

#endif
