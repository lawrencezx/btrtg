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
 * 3) xfr_statement: set instruction statements as destination of loopxx/call control transfer instruction
 * 5) rpt_statement: repeat generating instruction statements
 * 5) elem_statement: repeat generating instruction statements
 *
 * element statement types
 * 1) Iset: instructin set/group
 * 2) Print: print test information
 * 3) Insn: a single instruction
 */

/* element structure
 */
typedef enum elem_type {
    ISET_ELEM,
    PRINT_ELEM,
    INSN_ELEM
} elem_type;

typedef enum print_type {
    X86_STATE, /* x86 register information */
    X87_STATE, /* x87 state information */
    ALL_STATE  /* all information */
} print_type;

typedef struct elem_struct {
    elem_type type;
    union {
        WDTree *wdtree;
        print_type printType;
        const char *inst;
    };
} elem_struct;

/* block statement structure
 */
typedef enum blk_type {
    SEQ_BLK,
    SEL_BLK,
    XFR_BLK,
    RPT_BLK,
    ELEM_BLK
} blk_type;

typedef struct blk_struct {
    blk_type type;
    int num;
    const char *xfrName;
    int times;          /* RPT_BLK, XFR_BLK */
    void **blks;
} blk_struct;

/* template
 */
struct tmplt {
    void *blk;
};

void init_blk_struct(blk_struct *blk);
void walk_tmplt(void);

extern struct tmplt tmpltm;

#endif
