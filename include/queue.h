#ifndef TEST_QUEUE_H
#define TEST_QUEUE_H

/* 
 * List definitions
 */
#define TLIST_HEAD(name, type)                                          \
struct name {                                                           \
    struct type *lh_first;  /* first element */                         \
}

#define TLIST_HEAD_INITIALIZER(head)                                    \
    { NULL }

#define TLIST_ENTRY(type)                                               \
struct {                                                                \
    struct type *le_next;                                               \
    struct type **le_prev;                                              \
}

/*
 * List operations
 */
#define TLIST_INIT(head) do {                                           \
    (head)->lh_first = NULL;                                            \
} while (/*CONSTCOND*/0)

#define TLIST_INSERT_AFTER(listelm, elm, field) do {                    \
    if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)      \
        (listelm)->field.le_next->field.le_prev = &(elm)->field.le_next;\
    (listelm)->field.le_next = (elm);                                   \
    (elm)->field.le_prev = &(listelm)->field.le_next;                   \
} while (/*CONSTCOND*/0)

#define TLIST_INSERT_BEFORE(listelm, elm, field) do {                   \
    (elm)->field.le_prev = (listelm)->field.le_prev;                    \
    (elm)->field.le_next = (listelm);                                   \
    *(listelm)->field.le_prev = (elm);                                  \
    (listelm)->field.le_prev = &(elm)->field.le_next;                   \
} while (/*CONSTCOND*/0)

#define TLIST_INSERT_HEAD(head, elm, field) do {                        \
    if (((elm)->field.le_next = (head)->lh_first) != NULL)              \
        (head)->lh_first->field.le_prev = &(elm)->field.le_next;        \
    (head)->lh_first = (elm);                                           \
    (elm)->field.le_prev = &(head)->lh_first;                           \
} while (/*CONSTCOND*/0)

#define TLIST_REMOVE(elm, field) do {                                   \
    if ((elm)->field.le_next != NULL)                                   \
        (elm)->field.le_next->field.le_prev = (elm)->field.le_prev;     \
    *(elm)->field.le_prev = (elm)->field.le_next;                       \
    (elm)->field.le_next = NULL;                                        \
    (elm)->field.le_prev = NULL;                                        \
} while (/*CONSTCOND*/0)

#define TLIST_FOREACH(var, head, field)                                 \
    for ((var) = ((head)->lh_first);                                    \
        (var);                                                          \
        (var) = ((var)->field.le_next))

#define QLIST_REMOVE(elm, field) do {                                   \
    if ((elm)->field.le_next != NULL)                                   \
        (elm)->field.le_next->field.le_prev = (elm)->field.le_prev;     \
    *(elm)->field.le_prev = (elm)->field.le_next;                       \
    (elm)->field.le_next = NULL;                                        \
    (elm)->field.le_prev = NULL;                                        \
} while (/*CONSTCOND*/0)
#endif
