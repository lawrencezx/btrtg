#include "compiler.h"

#include "nasm.h"
#include "x86pg.h"
#include "operand.h"

void random_mem_addr_from_data(struct random_mem_addr *daddr)
{
    int32_t base, scale, index, disp;
    int data_size;
    static int max_mem_size = 512;

    base = nasm_random32(SECTION_DATA_NUM);
    scale = (1 << (nasm_random32(3) + 1));
    data_size = stat_get_data_sec()->datasizes[base] - max_mem_size;
    index = nasm_random32(data_size / scale);
    disp = nasm_random32(data_size - scale * index);

    daddr->base = base;
    daddr->scale = scale;
    daddr->index = index;
    daddr->disp = disp;
}
