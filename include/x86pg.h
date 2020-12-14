#ifndef NASM_X86PG_H
#define NASM_X86PG_H

#include "nasm.h"
#include "insnlist.h"
#include "section.h"
#include "seed.h"


/* big sequence index
 */
#define SEQ_INDEXES_NUM 6
/* big_sequence_index:
 *     A big index structure used to traverse all the operand combinations.
 *     Every field in this structure has a corresponding create_XXX function.
 *     It is used to generate unit test.
 */
typedef struct big_sequence_index {
    bool start;
    int i;
    int num;
    int indexes[SEQ_INDEXES_NUM];
} big_sequence_index;


/* INDEX SIZE | INDEX POS
 */
typedef uint32_t bseqiflags_t;

#define INDEXPOS_SHIFT         (0)
#define INDEXPOS_BITS          (6)
#define INDEXPOS_MASK          (((1 << INDEXPOS_BITS) - 1) << INDEXPOS_SHIFT)
#define GEN_INDEXPOS(bit)      (bit << INDEXPOS_SHIFT)
                            
#define INDEXSIZE_SHIFT       (6)
#define INDEXSIZE_BITS        (5)
#define INDEXSIZE_MASK        (((1 << INDEXSIZE_BITS) - 1) << INDEXSIZE_SHIFT)
#define GEN_INDEXSIZE(bit)    (bit << INDEXSIZE_SHIFT)

/* bits distribution (counted from 0)
 *  *
 *   3         2         1
 * 210987654321098765432109876543210
 * ...........................111111 index position in bseqi.indexes[]
 * ......................11111...... index size of bseqi.indexes[pos]
 */
#define BSEQIFLAG_VALUE(flags,flag)      ((flags & flag##_MASK) >> flag##_SHIFT)
#define BSEQIFLAG_INDEXPOS(flags)        BSEQIFLAG_VALUE(flags,INDEXPOS)
#define BSEQIFLAG_INDEXSIZE(flags)       BSEQIFLAG_VALUE(flags,INDEXSIZE)

void bseqi_init(big_sequence_index *bseqi);
bseqiflags_t bseqi_flags(opflags_t opndflags);
bool bseqi_inc(big_sequence_index *bseqi, const insn_seed *seed, int opnum);


/* global program generator state
 */
struct X86PGState {
    bool seqMode;
    big_sequence_index bseqi;
    bool simpleDataMemMode;

    struct section text_sec;
    struct section data_sec;

    const insn_seed *curr_seed;

    int labeli;
    insnlist_entry_t **labelspos;

    insn *curr_inst;
    insnlist_t *instlist;
    insnlist_entry_t *insertpos;
};

extern struct X86PGState X86PGState;

void init_x86pgstate(void);
void reset_x86pgstate(void);

#endif
