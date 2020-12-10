#ifndef NASM_MODEL_H
#define NASM_MODEL_H

#include "hashtbl.h"
#include "constVal.h"

/* weight decision tree
 */
typedef struct WDTree {
    bool isleaf;
    int size;
    int *weights;
    struct WDTree **children;
    constVal *consts;
} WDTree;

/* TKmodel: testing knowledge model 
 *  a weighted set of instruction scenarios.
 */
typedef struct TKmodel {
    double initP;
    bool diffSrcDest;
    union {
        WDTree *wdtree;
        struct {
            WDTree *wdsrctree;
            WDTree *wddesttree;
        };
    };
} TKmodel;

TKmodel *tkmodel_create(void);
WDTree *wdtree_create(void);
void wdtree_clear(WDTree *tree);
bool request_initialize(const char *instName);
constVal *request_constVal(const char *instName, bool isSrc);

extern struct hash_table hash_wdtrees;
extern struct hash_table hash_tks;

#endif
