#ifndef TEST_INSNLIST_H
#define TEST_INSNLIST_H

typedef struct insnlist insnlist_t;

insnlist_t *insnlist_create(void);
int insnlist_insert(insnlist_t *instlist, const insn* inst);
void insnlist_clear(insnlist_t *instlist);
void insnlist_destroy(insnlist_t *instlist);
void insnlist_output(insnlist_t *instlist, const struct ofmt *ofmt);

#endif
