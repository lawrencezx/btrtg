#include "compiler.h"

#include "nasm.h"
#include "x86pg.h"
#include "operand.h"
#include "mem.h"

static bool is_sse(enum opcode opcode)
{
    return opcode >= I_ADDPS && opcode <= I_FXSAVE;
}

static bool is_sse2(enum opcode opcode)
{
    return opcode >= I_MASKMOVDQU && opcode <= I_XORPD;
}

#define XMM_MEM_ALIGN_MASK 0xffffff80

void random_mem_addr_from_data(struct random_mem_addr *daddr, enum SIB_MODE mode)
{
    int32_t base, scale, index, disp;
    int data_size, data_size_buf;
    static int max_mem_size = 128;

    base = nasm_random32(SECTION_DATA_NUM);
    scale = (1 << (nasm_random32(3) + 1));
    data_size = stat_get_data_sec()->datasizes[base] - max_mem_size;
    if (is_sse(stat_get_opcode()) || is_sse2(stat_get_opcode())) {
        printf("data_size1:%d\n",data_size);
        while((data_size_buf = nasm_random32(data_size)) == 0)
            ;
        data_size = data_size_buf;
        printf("data_size2:%d\n",data_size);
        disp = nasm_random32(data_size)&XMM_MEM_ALIGN_MASK;
        data_size -= disp;
        index = (data_size/scale)&XMM_MEM_ALIGN_MASK;
    } else {
        index = nasm_random32(data_size / scale);
        disp = data_size - scale * index;
    }

    daddr->base = base;
    daddr->scale = scale;
    daddr->index = index;
    daddr->disp = (mode == SIB_MODE_ISD || mode == SIB_MODE_D) ? base : disp;
}
