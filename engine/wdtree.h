#ifndef NASM_WDTREE_H
#define NASM_WDTREE_H

#include "hashtbl.h"

enum const_type {
    CONST_IMM8,
    CONST_IMM16,
    CONST_IMM32,
    CONST_IMM64,
    CONST_FLOAT,
    CONST_BCD,
    CONST_UNITY,
    CONST_ASM_OP,
    CONST_X87STATUS
};

struct const_node {
    enum const_type type;
    union {
        uint8_t imm8;
        uint16_t imm16;
        uint32_t imm32;
        uint64_t imm64;
        uint32_t bcd[3];
        uint8_t unity;
        char *asm_op;
    };
    struct {
        float float32;
        double float64;
        long double float80;
    };
    struct {
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
    };
};

/* weight decision tree
 */
struct wd_node {
    bool isleaf;
    int size;
    GArray *weights;
    GArray *sub_nodes;
    GArray *const_nodes;
};

struct wd_root {
    struct wd_node *wd_node;
};

struct wd_root *wdtree_create(void);
struct wd_node *wdtree_node_create(void);
void wdtree_node_clear(struct wd_node *tree);
void wdtrees_free_all(void);
struct const_node *wdtree_select_leaf_node(struct wd_root *tree);

extern struct hash_table hash_wdtrees;

#endif
