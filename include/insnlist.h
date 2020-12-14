#ifndef TEST_INSNLIST_H
#define TEST_INSNLIST_H

typedef struct insnlist_entry insnlist_entry_t;
typedef struct insnlist insnlist_t;

insnlist_t *insnlist_create(void);
insnlist_entry_t *insnlist_insert_before(insnlist_t *instlist, insnlist_entry_t *pos, const insn* inst);
insnlist_entry_t *insnlist_insert_after(insnlist_t *instlist, insnlist_entry_t *pos, const insn* inst);
insnlist_entry_t *insnlist_insert_tail(insnlist_t *instlist, const insn* inst);
void insnlist_clear(insnlist_t *instlist);
void insnlist_destroy(insnlist_t *instlist);
void insnlist_output(insnlist_t *instlist, const struct ofmt *ofmt);

#endif
