#ifndef NASM_X86PG_H
#define NASM_X86PG_H

#include "nasm.h"
#include "insnlist.h"
#include "section.h"
#include "seed.h"
#include "tmplt.h"


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


enum lock_reg_type {
  LOCK_REG_AX = 0,
  LOCK_REG_BX,
  LOCK_REG_CX,
  LOCK_REG_DX,
  LOCK_REG_SI,
  LOCK_REG_DI,
  LOCK_REG_NUM
};

enum lock_reg_case {
    LOCK_REG_CASE_NULL,
    LOCK_REG_CASE_MEM,
    LOCK_REG_CASE_LOOP
};

/* global program generator state
 */
struct X86PGState {
    bool seqMode;
    big_sequence_index bseqi;

    struct section text_sec;
    struct section data_sec;

    struct {
        enum opcode opcode;
        insn *curr_inst;
        bool need_init;
        int opi;
        GArray *constVals;
    }; /* current instruction */

    blk_struct *curr_blk;

    int labeli;
    insnlist_entry_t **labelspos;
    bool lock_ctrl;
    enum lock_reg_case lock_reg_cases[6];

    insnlist_t *instlist;
    insnlist_entry_t *insertpos;
};

extern struct X86PGState X86PGState;

void init_x86pgstate(void);
void reset_x86pgstate(void);

enum position {
    INSERT_AFTER,
    INSERT_BEFORE,
    INSERT_TAIL
};

void stat_insert_insn(insn *inst, enum position pos);

struct section *stat_get_data_sec(void);
blk_struct *stat_get_curr_blk(void);
void stat_set_curr_blk(blk_struct *blk);
int stat_get_labeli(void);
void stat_inc_labeli(void);
enum opcode stat_get_opcode(void);
void stat_set_opcode(enum opcode opcode);
bool stat_get_need_init(void);
void stat_set_need_init(bool need_init);
int stat_get_opi(void);
void stat_set_opi(int opi);
GArray *stat_get_constVals(void);
void stat_set_constVals(GArray *constVals);

void stat_lock_ctrl(void);
void stat_unlock_ctrl(void);
bool stat_ctrl_locked(void);
void stat_lock_reg(enum reg_enum reg, enum lock_reg_case lr_case);
void stat_unlock_reg(enum lock_reg_case lr_case);
bool stat_reg_locked(enum reg_enum reg);

#endif
