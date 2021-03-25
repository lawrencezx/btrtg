#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "tk.h"
#include "buf2token.h"
#include "asmlib.h"

struct hash_table hash_tks;

static int get_implied_operands_num(enum opcode opcode)
{
    switch (opcode) {
        case I_DIV:
        case I_IDIV:
        case I_CMPXCHG:
        case I_CBW:
        case I_CWDE:
        case I_CWD:
        case I_CDQ:
        case I_POPF:
        case I_JCXZ:
        case I_JECXZ:
            return 1;
        default:
            return -1;
    }
}

static int adjust_operands_num(enum opcode opcode)
{
    switch(opcode){
        case I_FADD:
        case I_FADDP:
        case I_FIADD:
        case I_FCMOVB:
        case I_FCMOVBE:
        case I_FCMOVE:
        case I_FCMOVNB:
        case I_FCMOVNBE:
        case I_FCMOVNE:
        case I_FCMOVNU:
        case I_FCMOVU:
        case I_FCOM:
        case I_FCOMP:
        case I_FCOMPP:
        case I_FCOMI:
        case I_FCOMIP:
        case I_FUCOMI:
        case I_FUCOMIP:
        case I_FDIV:
        case I_FDIVP:
        case I_FIDIV:
        case I_FDIVR:
        case I_FDIVRP:
        case I_FIDIVR:
        case I_FMUL:
        case I_FMULP:
        case I_FIMUL:
        case I_FPREM:
        case I_FPREM1:
        case I_FPATAN:
        case I_FSCALE:
        case I_FSUB:
        case I_FSUBP:
        case I_FISUB:
        case I_FSUBR:
        case I_FSUBRP:
        case I_FISUBR:
        case I_FYL2X:
        case I_FYL2XP1:
        case I_FXCH:
        case I_FUCOM:
        case I_FUCOMP:
        case I_FUCOMPP:
        case I_FICOM:
        case I_FICOMP:
            return 2;
        case I_FABS:
        case I_FCHS:
        case I_FCOS:
        case I_FTST:
        case I_FSIN:
        case I_FSINCOS:
        case I_FPTAN:
        case I_FRNDINT:
        case I_FXTRACT:
        case I_FILD:
        case I_F2XM1:
        case I_FSQRT:
        //case I_FIST:
        //case I_FISTP:
            return 1;
        default:
            return -1;
    }
}

struct tk_model *tkmodel_create(void)
{
    struct tk_model *tkm;
    tkm = (struct tk_model *)nasm_malloc(sizeof(struct tk_model));
    tkm->tk_trees = g_array_new(FALSE, FALSE, sizeof(struct wd_root *));
    return tkm;
}

void tks_free_all(void)
{
    struct hash_iterator it;
    const struct hash_node *np;

    hash_for_each(&hash_tks, it, np) {
        if (np->data) {
            struct tk_model *tkm = (struct tk_model *)np->data;
            g_array_free(tkm->tk_trees, true);
            nasm_free(np->data);
        }
        if (np->key)
            nasm_free((void *)np->key);
    }

    hash_free(&hash_tks);
}

static struct tk_model *get_tkm_from_hashtbl(const char *asm_op)
{
    struct hash_insert hi;
    void **tkmpp;
    tkmpp = hash_find(&hash_tks, asm_op, &hi);
    return tkmpp == NULL ? NULL : *(struct tk_model **)tkmpp;
}

void create_trv_state(char *asm_inst, struct trv_state *trv_state)
{
    char asm_opcode[128];
    int i = 0, opi = 0;
    struct tk_model *tkm;
    struct wd_root *tk_tree;
    enum opcode opcode;

    while (asm_inst[i] != ' ' && asm_inst[i] != '\n' && asm_inst[i] != '\0') {
        asm_opcode[i] = asm_inst[i];
        i++;
    }
    asm_opcode[i] = '\0';
    opcode = parse_asm_opcode(asm_opcode);
    tkm = get_tkm_from_hashtbl(asm_opcode);
    if (tkm->tk_trees->len == 1) {
        tk_tree = g_array_index(tkm->tk_trees, struct wd_root *, 0);
        trv_state->tk_trees = g_array_new(FALSE, FALSE, sizeof(struct wd_root *));
        /* first operand */
        if (asm_inst[i] != '\0' && asm_inst[i] != '\n') {
            g_array_append_val(trv_state->tk_trees, tk_tree);
            opi++;
        }
        /* other opi */
        while (asm_inst[i] != '\0' && asm_inst[i] != '\n')
            if (asm_inst[i++] == ',') {
                g_array_append_val(trv_state->tk_trees, tk_tree);
                opi++;
            }
        /* implied opi */
        for(int j = adjust_operands_num(opcode) - opi; j > 0; j--) {
            g_array_append_val(trv_state->tk_trees, tk_tree);
            opi++;
        }
        /* implied opi */
        for(int j = get_implied_operands_num(opcode); j > 0; j--) {
            g_array_append_val(trv_state->tk_trees, tk_tree);
            opi++;
        }
    } else {
        for (guint i = 0; i < tkm->tk_trees->len; i++) {
            trv_state->tk_trees = g_array_copy(tkm->tk_trees);
        }
    }

    trv_state->trv_nodes = g_array_new(FALSE, FALSE, sizeof(GArray *));
}

struct const_node *request_val_node(const char *asm_op, int opi)
{
    struct tk_model *tkm;
    struct wd_root *tk_tree;

    tkm = get_tkm_from_hashtbl(asm_op);
    if (tkm->tk_trees->len == 1)
        opi = 0;
    tk_tree = g_array_index(tkm->tk_trees, struct wd_root *, opi);
    return wdtree_select_leaf_node(tk_tree);
}

GArray *request_packed_val_node(const char *asm_op, int opi)
{
    struct tk_model *tkm;
    struct wd_root *tk_tree;
    GArray *packed_nodes;
    struct const_node *val_node;

    tkm = get_tkm_from_hashtbl(asm_op);
    if (tkm->tk_trees->len == 1)
        opi = 0;
    tk_tree = g_array_index(tkm->tk_trees, struct wd_root *, opi);
    packed_nodes = g_array_new(FALSE, FALSE, sizeof(struct const_node *));
    for (int i = 0; i < tk_tree->packedn; i++) {
        val_node = wdtree_select_leaf_node(tk_tree);
        g_array_append_val(packed_nodes, val_node);
    }
    return packed_nodes;
}
