#ifndef NASM_BSEQI_H
#define NASM_BSEQI_H

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


typedef uint32_t bseqiflags_t;


#define INDEX_SHIFT         (0)
#define INDEX_BITS          (6)
#define INDEX_MASK          (((1 << INDEX_BITS) - 1) << INDEX_SHIFT)
#define GEN_INDEX(bit)      (bit << INDEX_SHIFT)
                            
#define SEQINUM_SHIFT       (6)
#define SEQINUM_BITS        (5)
#define SEQINUM_MASK        (((1 << SEQINUM_BITS) - 1) << SEQINUM_SHIFT)
#define GEN_SEQINUM(bit)    (bit << SEQINUM_SHIFT)

/* bits distribution (counted from 0)
 *  *
 *   3         2         1
 * 210987654321098765432109876543210
 * ...........................111111 index of bseqi.indexes
 * ......................11111...... sequence index numbers
 */
#define SEQIFLAG_VALUE(flags,flag)    ((flags & flag##_MASK) >> flag##_SHIFT)
#define SEQIFLAG_INDEX(flags)         SEQIFLAG_VALUE(flags,INDEX)
#define SEQIFLAG_SEQINUM(flags)       SEQIFLAG_VALUE(flags,SEQINUM)

void bseqi_init(big_sequence_index *bseqi);
bseqiflags_t bseqi_flags(opflags_t opndflags);
bool bseqi_inc(big_sequence_index *bseqi, const insn_seed *seed, int opnum);

#endif
