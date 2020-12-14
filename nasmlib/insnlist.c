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
    if (s->ctrl) {
        p->ctrl = nasm_strdup(s->ctrl);
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
    free(inst->ctrl);
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

insnlist_entry_t *insnlist_insert_before(insnlist_t *instlist, insnlist_entry_t *pos, const insn* inst)
{
    if (instlist == NULL || inst == NULL ||
        (pos == NULL && instlist->tail != NULL))
        return NULL;
    struct insnlist_entry *entry;
    entry = nasm_zalloc(sizeof(*entry));
    entry->insn = nasm_insndup(inst);
    if (pos == NULL) {
        TLIST_INSERT_HEAD(&instlist->insn_entries, entry, insn_link);
        instlist->tail = instlist->insn_entries.lh_first;
    } else {
        TLIST_INSERT_BEFORE(pos, entry, insn_link);
    }
    instlist->insn_count++;
    return entry;
}

insnlist_entry_t *insnlist_insert_after(insnlist_t *instlist, insnlist_entry_t *pos, const insn* inst)
{
    if (instlist == NULL || inst == NULL ||
        (pos == NULL && instlist->tail != NULL))
        return NULL;
    struct insnlist_entry *entry;
    entry = nasm_zalloc(sizeof(*entry));
    entry->insn = nasm_insndup(inst);
    if (pos == NULL) {
        TLIST_INSERT_HEAD(&instlist->insn_entries, entry, insn_link);
        instlist->tail = instlist->insn_entries.lh_first;
    } else {
        TLIST_INSERT_AFTER(pos, entry, insn_link);
        if (pos == instlist->tail)
            instlist->tail = entry;
    }
    instlist->insn_count++;
    return entry;
}

insnlist_entry_t *insnlist_insert_tail(insnlist_t *instlist, const insn* inst)
{
    if (instlist == NULL || inst == NULL)
        return NULL;
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
    return entry;
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
    insnlist_clear(instlist);
    nasm_free(instlist);
}

void insnlist_output(insnlist_t* instlist, const struct ofmt *ofmt)
{
    struct insnlist_entry *entry;
    const char *buf;
    struct output_data data;
    TLIST_FOREACH(entry, &instlist->insn_entries, insn_link) {
        char *ctrl = entry->insn->ctrl;
        if (ctrl) {
            data.type = OUTPUT_RAWDATA;
            data.buf = (const void *)ctrl;
            ofmt->output(&data);
        } else {
            insn_to_asm(entry->insn, &buf);
            data.type = OUTPUT_INSN;
            data.buf = (const void *)buf;
            ofmt->output(&data);
        }
    }
}
