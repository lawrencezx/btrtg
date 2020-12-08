#include "compiler.h"

#include "nasm.h"
#include "insnlist.h"
#include "queue.h"
#include "ofmt.h"
#include "seed.h"
#include "generator.h"

struct insnlist_entry {
    insn *insn;
    TLIST_ENTRY(insnlist_entry) insn_link;
};

struct insnlist {
    TLIST_HEAD(, insnlist_entry) insn_entries;
    uint32_t insn_count;
    struct insnlist_entry *tail;
};


/*
 * Instruction allocation/free
 */
static insn *nasm_insndup(const insn* s)
{
    insn *p;
    const size_t size = sizeof(*s);

    p = nasm_malloc(size);
    memcpy(p, s, size);
    if (s->label) {
        p->label = nasm_strdup(s->label);
    }
    if (s->eops) {
        /* TODO: p->eops = nasm_eopsdup(s->eops); */
    }
    return p;
}

static void nasm_insnfree(insn* inst)
{
    if (inst == NULL)
        return;
    free(inst->label);
    free(inst);
}

/*
 * Instruction list operations.
 */
insnlist_t *insnlist_create(void)
{
    insnlist_t *instlist;

    instlist = nasm_zalloc(sizeof(*instlist));
    TLIST_INIT(&instlist->insn_entries);
    instlist->tail = NULL;
    instlist->insn_count = 0;

    return (instlist);
}

int insnlist_insert(insnlist_t *instlist, const insn* inst)
{
    if (inst == NULL)
        return -1;
    struct insnlist_entry *entry;
    entry = nasm_zalloc(sizeof(*entry));
    entry->insn = nasm_insndup(inst);
    if (instlist->tail == NULL) {
        TLIST_INSERT_HEAD(&instlist->insn_entries, entry, insn_link);
        instlist->tail = instlist->insn_entries.lh_first;
    } else {
        TLIST_INSERT_AFTER(instlist->tail, entry, insn_link);
        instlist->tail = entry;
    }
    instlist->insn_count++;
    return 0;
}

void insnlist_clear(insnlist_t *instlist)
{
    struct insnlist_entry *entry;
    while (instlist->insn_entries.lh_first != NULL) {
        entry = instlist->insn_entries.lh_first;
        TLIST_REMOVE(entry, insn_link);

        nasm_insnfree(entry->insn);
        nasm_free(entry);
    }
    instlist->tail = NULL;
    instlist->insn_count = 0;
}

void insnlist_destroy(insnlist_t *instlist)
{
    nasm_free(instlist);
}

void insnlist_output(insnlist_t* instlist, const struct ofmt *ofmt)
{
    struct insnlist_entry *entry;
    const char *buf;
    struct output_data data;
    TLIST_FOREACH(entry, &instlist->insn_entries, insn_link) {
        insn_to_asm(entry->insn, &buf);
        data.type = OUTPUT_INSN;
        data.buf = (const void *)buf;
        ofmt->output(&data);
    }
}
