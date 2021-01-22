#ifndef NASM_WDTREE_H
#define NASM_WDTREE_H

#include "hashtbl.h"

typedef enum constType {
    CONST_IMM8,
    CONST_IMM16,
    CONST_IMM32,
    CONST_UNITY,
    CONST_INSN  
} constType;

typedef struct constVal {
    constType type;
    union {
        int8_t imm8;
        int16_t imm16;
        int32_t imm32;
        int8_t unity;
        char *instName;
    };
} constVal;

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
constVal *wdtree_select_constval(WDTree *tree);

extern struct hash_table hash_wdtrees;

#endif
