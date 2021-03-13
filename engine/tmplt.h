#ifndef NASM_TMPLT_H
#define NASM_TMPLT_H

#include "wdtree.h"

/********************************************
 * template 
 *      default: seq_statement
 *
 * instruction block statement types
 * 1) seq_statement: select instruction statements sequentially
 * 2) sel_statement: select instruction statements randomly
 * 3) ttt_statement: wrapper of two target destination instructions
 *                   exp: loopxx, call, jcxz, jecxz
 * 5) rpt_statement: repeat generating instruction statements
 * 5) elem_statement: repeat generating instruction statements
 *
 * element statement types
 * 1) V: a variable
 * 2) G: instructin set/group
 * 3) C: check a value
 * 4) I: a single instruction
 */

/* element structure
 */
enum elem_type {
    NONE_ELEM,
    V_ELEM,
    G_ELEM,
    C_ELEM,
    I_ELEM
};

typedef struct elem_struct {
    double inip;
    enum elem_type type;
    union {
        struct wd_root *g_tree;     /* G_ELEM */
        char *c_type;               /* C_ELEM */
        struct {                    /* I_ELEM */
            char *asm_inst;
            GArray *trv_nodes;
        };
    };
} elem_struct;

/* traversing state of TRV_BLK block
 */
struct trv_state {
    GArray *tk_trees;
    GArray *trv_nodes;
};

/* block variable structure
 */
struct blk_var {
    bool        valid;
    char        *name;
    opflags_t   opndflags;
    char        *var_type;
    char        *var_val;
    /* one or two mov instructions, initiliaze basereg and index */
    bool        is_mem_opnd;
    bool        has_label;
    char        *init_mem_addr;
};

/* block statement structure
 */
enum blk_type {
    SEQ_BLK,
    SEL_BLK,
    TTT_BLK,
    RPT_BLK,
    TRV_BLK,
    ELEM_BLK
};

typedef struct blk_struct {
    struct blk_struct *parent;
    enum blk_type type;
    char *ttt_op;                   /* TTT_BLK */
    int times;                      /* RPT_BLK, TTT_BLK */
    struct trv_state *trv_state;    /* TRV_BLK */
    GArray *vars;
    GArray *blks;
} blk_struct;

/* template
 */
typedef struct tmplt_struct {
    void *blk;
} tmplt_struct;

void init_blk_struct(blk_struct *blk);
void init_blk_var(struct blk_var *var);
void init_elem_struct(elem_struct *elem);
struct blk_var *blk_search_var(blk_struct *blk, const char *var_name);
void walk_tmplt(void);
void tmplt_clear(tmplt_struct *tmpltm);
void tmplt_free(tmplt_struct *tmpltm);

struct const_node *request_trv_node(int opi);
GArray *request_packed_trv_node(int opi);

extern tmplt_struct tmpltm;

#endif
