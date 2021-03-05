#ifndef NASM_MEM_H
#define NASM_MEM_H

enum SIB_MODE {
    SIB_MODE_D,
    SIB_MODE_B,
    SIB_MODE_BD,
    SIB_MODE_ISD,
    SIB_MODE_BID,
    SIB_MODE_BISD,
    SIB_MODE_NUM
};

static inline bool sib_mode_has_base(enum SIB_MODE mode)
{
    return  mode == SIB_MODE_B || mode == SIB_MODE_BD || 
            mode == SIB_MODE_BID || mode == SIB_MODE_BISD;
}

static inline bool sib_mode_has_index(enum SIB_MODE mode)
{
    return  mode == SIB_MODE_ISD || mode == SIB_MODE_BID ||
            mode == SIB_MODE_BISD;
}

/* generate random memory address */
void random_mem_addr_from_data(struct random_mem_addr *daddr, enum SIB_MODE mode);
void random_mem_addr_from_stack(struct random_mem_addr *daddr, enum SIB_MODE mode);

#endif
