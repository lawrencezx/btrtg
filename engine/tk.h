#ifndef NASM_MODEL_H
#define NASM_MODEL_H

#include "hashtbl.h"
#include "wdtree.h"
#include "tmplt.h"

/* tk_model: testing knowledge model 
 *  a weighted set of instruction scenarios.
 */
struct tk_model {
    GArray *tk_trees;
    //struct wd_root *tk_tree;
};

struct tk_model *tkmodel_create(void);
void tks_free_all(void);
struct const_node *request_val_node(const char *asm_op, int opi);
GArray *request_packed_val_node(const char *asm_op, int opi);
void create_trv_state(char *asm_inst, struct trv_state *trv_state);

extern struct hash_table hash_tks;

#endif
