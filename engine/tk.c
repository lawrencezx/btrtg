#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "tk.h"
#include "buf2token.h"

struct hash_table hash_tks;

static int get_implied_operands_from_instname(char *inst_name)
{
    struct tokenval tv;
    nasm_token_hash(inst_name, &tv);
    enum opcode opcode = tv.t_integer;

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

static int get_operands_from_instname(char * inst_name){
    struct tokenval tv;
    nasm_token_hash(inst_name, &tv);
    enum opcode opcode = tv.t_integer;

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
            return 2;
        case I_FABS:
        case I_FCHS:
        case I_FCOS:
        case I_FSIN:
        case I_FSINCOS:
        case I_FPTAN:
        case I_FRNDINT:
        case I_FXTRACT:
        case I_FILD:
        case I_F2XM1:
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
    hash_free_all(&hash_tks, true);
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
    char inst_name[128];
    int i = 0, operands = 0;
    struct tk_model *tkm;
    struct wd_root *tk_tree;

    while (asm_inst[i] != ' ' && asm_inst[i] != '\n' && asm_inst[i] != '\0') {
        inst_name[i] = asm_inst[i];
        i++;
    }
    inst_name[i] = '\0';
    tkm = get_tkm_from_hashtbl(inst_name);
    if (tkm->tk_trees->len == 1) {
        trv_state->tk_trees = g_array_new(FALSE, FALSE, sizeof(struct wd_root *));
        tk_tree = g_array_index(tkm->tk_trees, struct wd_root *, 0);
        /* first operand */
        if (asm_inst[i] != '\0' && asm_inst[i] != '\n') {
            g_array_append_val(trv_state->tk_trees, tk_tree);
            operands++;
        }
        /* other operands */
        while (asm_inst[i] != '\0' && asm_inst[i] != '\n')
            if (asm_inst[i++] == ',') {
                g_array_append_val(trv_state->tk_trees, tk_tree);
                operands++;
            }
        /* implied operands */
        for(int j = get_operands_from_instname(inst_name) - operands; j > 0; j--) {
            g_array_append_val(trv_state->tk_trees, tk_tree);
        }
        /* implied operands */
        for(int j = get_implied_operands_from_instname(inst_name); j > 0; j--) {
            g_array_append_val(trv_state->tk_trees, tk_tree);
        }
    } else {
        for (guint i = 0; i < tkm->tk_trees->len; i++) {
            trv_state->tk_trees = g_array_copy(tkm->tk_trees);
        }
    }

    trv_state->val_nodes = g_array_new(FALSE, FALSE, sizeof(struct const_node *));
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
