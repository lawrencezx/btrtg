#ifndef NASM_WDTREE_H
#define NASM_WDTREE_H

#include "hashtbl.h"

enum const_type {
    CONST_IMM8,
    CONST_IMM16,
    CONST_IMM32,
    CONST_UNITY,
    CONST_INSN  
};

struct const_node {
    enum const_type type;
    union {
        int8_t imm8;
        int16_t imm16;
        int32_t imm32;
        int8_t unity;
        char *instName;
    };
};

/* weight decision tree
 */
typedef struct WDTree {
    bool isleaf;
    int size;
    GArray *weights;
    GArray *subtrees;
    GArray *consts;
} WDTree;

typedef struct Tmpltmodel {
    int instNum;
    WDTree *wdtree;
} Tmpltmodel;

WDTree *wdtree_create(void);
void wdtree_clear(WDTree *tree);
void wdtrees_free_all(void);
struct const_node *wdtree_select_leaf_node(WDTree *tree);

extern struct hash_table hash_wdtrees;

#endif
