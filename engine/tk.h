#ifndef NASM_MODEL_H
#define NASM_MODEL_H

#include "hashtbl.h"
#include "wdtree.h"
#include "tmplt.h"

/* TKmodel: testing knowledge model 
 *  a weighted set of instruction scenarios.
 */
typedef struct TKmodel {
    bool diffSrcDest;
    union {
        struct wd_root *tk_tree;
        struct {
            struct wd_root *tk_src_tree;
            struct wd_root *tk_dest_tree;
        };
    };
} TKmodel;

TKmodel *tkmodel_create(void);
void tks_free_all(void);
struct const_node *request_val_node(const char *instName, bool isDest);
void create_trv_state(char *asm_inst, struct trv_state *trv_state);

extern struct hash_table hash_tks;

#endif
