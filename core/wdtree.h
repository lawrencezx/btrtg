#ifndef NASM_WDTREE_H
#define NASM_WDTREE_H

#include "hashtbl.h"

typedef struct constVal {
    opflags_t type;
    union {
        int8_t imm8;
        int8_t unity;
        int16_t imm16;
        int32_t imm32;
        char *instName;
    };
} constVal;

/* weight decision tree
 */
typedef struct WDTree {
    bool isleaf;
    int size;
    int *weights;
    struct WDTree **children;
    constVal *consts;
} WDTree;

typedef struct Tmpltmodel {
    int instNum;
    WDTree *wdtree;
} Tmpltmodel;

WDTree *wdtree_create(void);
void wdtree_clear(WDTree *tree);
constVal *wdtree_select_constval(WDTree *tree);

extern struct hash_table hash_wdtrees;

#endif
