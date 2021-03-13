#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "generator.h"
#include "tmplt.h"
#include "x86pg.h"
#include "asmlib.h"
#include "ctrl.h"

tmplt_struct tmpltm;

void init_blk_struct(blk_struct *blk)
{
    blk->parent = NULL;
    blk->type = -1;
    blk->ttt_op = NULL;
    blk->times = 0;
    blk->trv_state = NULL;
    blk->blks = g_array_new(FALSE, FALSE, sizeof(void *));
    blk->vars = g_array_new(FALSE, FALSE, sizeof(struct blk_var));
}

void init_blk_var(struct blk_var *var)
{
    var->valid = false;
    var->name = NULL;
    var->opndflags = 0;
    var->var_type = NULL;
    var->var_val = NULL;
    var->init_mem_addr = NULL;
    var->has_label = false;
    var->is_mem_opnd = false;
}

void init_elem_struct(elem_struct *elem)
{
    elem->inip = 0;
    elem->type = NONE_ELEM;
    elem->g_tree = NULL;
    elem->c_type = NULL;
    elem->asm_inst = NULL;
    elem->trv_nodes = NULL;
}

struct blk_var *blk_search_var(blk_struct *blk, const char *var_name)
{
    if (blk == NULL)
        return NULL;
    var_name = nasm_skip_spaces(var_name);
    if (*var_name != '@')
        return NULL;
    for (guint i = 0; i < blk->vars->len; i++) {
        if (strcmp(g_array_index(blk->vars, struct blk_var, i).name, var_name + 1) == 0) {
            return &g_array_index(blk->vars, struct blk_var, i);
        }
    }
    return blk_search_var(blk->parent, var_name);
}

static void blk_invalid_var_all(blk_struct *blk)
{
    if (blk == NULL)
        return;
    for (guint i = 0; i < blk->vars->len; i++) {
        struct blk_var *var = &g_array_index(blk->vars, struct blk_var, i);
        free(var->var_val);
        free(var->init_mem_addr);
        var->valid = false;
        var->var_val = NULL;
        var->init_mem_addr = NULL;
    }
}

static void walkSeqBlk(blk_struct *blk);
static void walkSelBlk(blk_struct *blk);
static void walkTttBlk(blk_struct *blk);
static void walkRptBlk(blk_struct *blk);
static void walkTrvBlk(blk_struct *blk);
static void walkElemBlk(blk_struct *blk);

static void (*walkBlkFuncs[])(blk_struct *) =
{
    walkSeqBlk,
    walkSelBlk,
    walkTttBlk,
    walkRptBlk,
    walkTrvBlk,
    walkElemBlk
};

static void walkSeqBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);
    blk_invalid_var_all(blk);

    for (guint i = 0; i < blk->blks->len; i++) {
        blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
        walkBlkFuncs[subblk->type](subblk);
    }
}

static void walkSelBlk(blk_struct *blk)
{
    struct wd_root *selblk_tree;
    struct const_node *inst_node;
    insn_seed seed;
    insn inst;

    stat_set_curr_blk(blk);
    blk_invalid_var_all(blk);

    selblk_tree = g_array_index(blk->blks, struct wd_root *, 0);
    inst_node = wdtree_select_leaf_node(selblk_tree);
    
    create_insn_seed(&seed, inst_node->asm_op);
    one_insn_gen(&seed, &inst);
}

static void walkTttBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);
    blk_invalid_var_all(blk);

    enum opcode opcode = parse_asm_opcode(blk->ttt_op);
    int next_label = gen_control_transfer_insn(opcode, blk->times);
    for (guint i = 0; i < blk->blks->len; i++) {
        blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
        walkBlkFuncs[subblk->type](subblk);
    }
    update_insert_pos_to_label(next_label);

    stat_unlock_ctrl();
}

static void walkRptBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);

    for (int k = 0; k < blk->times; k++) {
        blk_invalid_var_all(blk);
        for (guint i = 0; i < blk->blks->len; i++) {
            blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
            walkBlkFuncs[subblk->type](subblk);
        }
    }
}

#define EXPECTED_TRV_NUM (8192)
static int trv_step = 0;
static int trv_index = 0;

static int sqrti(int num)
{
    int s;
    for (s=0; num >= (2 * s) + 1; num -= (2 * s++) + 1)
        ;
    return s;
}

static int is_prime(int num) {
    if (num <= 3)
        return 1;
    if ((num % 2) == 0)
        return 0;

    int divider_limit = sqrti(num);

    for (int i = 3; i <= divider_limit; i += 2) {
        if ((num % i) == 0)
            return 0;
    }

    return 1;
}

static int greatest_prime(int limit)
{
    int i;
    for (i = limit; i>0; --i) {
        if (is_prime(i))
            return i;
    }
    return i;
}

static void preorder_traverse_tk_trees(blk_struct *blk, struct wd_root *tk_tree,
        struct wd_node *tk_node, GArray *val_nodes, int treei, int packedi)
{
    struct trv_state *trv_state = blk->trv_state;
    struct const_node *val_node;
    GArray *next_val_nodes;
    struct wd_root *next_tk_tree;

    if (tk_node == NULL || tk_node->size == 0)
        return;

    for (int i = 0; i < tk_node->size; i++) {
        if (tk_node->isleaf) {
            val_node = &g_array_index(tk_node->const_nodes, struct const_node, i);
            g_array_append_val(val_nodes, val_node);

            if ((size_t)treei + 1 == trv_state->tk_trees->len &&
                packedi + 1 == tk_tree->packedn) {
                /* got a complete traverse value nodes */
                if ((trv_index++) % trv_step == 0) {
                    blk_invalid_var_all(blk);
                    for (guint i = 0; i < blk->blks->len; i++) {
                        blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
                        walkBlkFuncs[subblk->type](subblk);
                    }
                }
            } else if (packedi + 1 == tk_tree->packedn) {
                /* finished a packed operand's value nodes */
                next_val_nodes = g_array_new(FALSE, FALSE, sizeof(struct const_node *));
                g_array_append_val(trv_state->trv_nodes, next_val_nodes);

                next_tk_tree = g_array_index(trv_state->tk_trees, struct wd_root *, treei + 1);
                preorder_traverse_tk_trees(blk, next_tk_tree, tk_tree->wd_node,
                        next_val_nodes, treei + 1, 0);

                g_array_remove_index(trv_state->trv_nodes, treei + 1);
                g_array_free(next_val_nodes, true);
            } else {
                /* during a packed operand's value nodes */
                preorder_traverse_tk_trees(blk, tk_tree, tk_tree->wd_node,
                        val_nodes, treei, packedi + 1);
            }

            g_array_remove_index(val_nodes, packedi);
        } else {
            preorder_traverse_tk_trees(blk, tk_tree, g_array_index(tk_node->sub_nodes,
                        struct wd_node *, i), val_nodes, treei, packedi);
        }
    }
}

static int get_tk_tree_leaf_num(struct wd_node *tk_node)
{
    if (tk_node == NULL || tk_node->size == 0)
        return 0;
    if (tk_node->isleaf)
        return tk_node->size;
    int num = 0;
    GArray *sub_nodes = tk_node->sub_nodes;
    for (size_t i = 0; i < sub_nodes->len; i++) {
        num += get_tk_tree_leaf_num(g_array_index(sub_nodes, struct wd_node *, i));
    }
    return num;
}

static void walkTrvBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);
    
    struct wd_root *tk_tree;
    struct trv_state *trv_state = blk->trv_state;
    GArray *tk_trees = trv_state->tk_trees;

    int possible_trv_num = 1;
    for (size_t i = 0; i < tk_trees->len; i++) {
        tk_tree = g_array_index(tk_trees, struct wd_root *, 0);
        int tk_tree_leaf_num = get_tk_tree_leaf_num(tk_tree->wd_node);
        for (int j = 0; j < tk_tree->packedn; j++)
            possible_trv_num *= tk_tree_leaf_num;
    }
    if (tk_trees->len != 0) {
        trv_step = 1;
        trv_index = 0;
        if (possible_trv_num / EXPECTED_TRV_NUM > 0)
            trv_step = greatest_prime(possible_trv_num / EXPECTED_TRV_NUM + 1);
        struct wd_root *tk_tree = g_array_index(tk_trees, struct wd_root *, 0);
        GArray *val_nodes = g_array_new(FALSE, FALSE, sizeof(struct const_node *));
        g_array_append_val(trv_state->trv_nodes, val_nodes);
        preorder_traverse_tk_trees(blk, tk_tree, tk_tree->wd_node, val_nodes, 0, 0);
        g_array_remove_index(trv_state->trv_nodes, 0);
        g_array_free(val_nodes, true);
    }
}

/* walk element
 */
static void walkGElem(elem_struct *g_e);
static void walkCElem(elem_struct *p_e);
static void walkIElem(elem_struct *i_e);

static void (*walkElemFuncs[])(elem_struct *elem) =
{
    NULL, /* none element */
    NULL, /* variable element */
    walkGElem,
    walkCElem,
    walkIElem
};

static void walkGElem(elem_struct *g_e)
{
    struct const_node *inst_node;
    insn_seed seed;
    insn inst;

    stat_set_need_init(likely_happen_p(g_e->inip));
    stat_set_has_mem_opnd(false);

    inst_node = wdtree_select_leaf_node(g_e->g_tree);
    
    create_insn_seed(&seed, inst_node->asm_op);
    one_insn_gen(&seed, &inst);
    stat_set_need_init(false);
}

static void walkCElem(elem_struct *c_e)
{
    char call_check_function[128];
    char *c_type = c_e->c_type;

    if (c_type == NULL)
        nasm_fatal("no check point\n");
    if (c_type[0] == '@') {
        struct blk_var *var = blk_search_var(stat_get_curr_blk(), c_type);
        if (!var->valid)
            nasm_fatal("checking value: %s has not been initialized", c_type);
        c_type = var->var_val;
    }
    sprintf(call_check_function, "  check %s", c_type);
    one_insn_gen_ctrl(call_check_function, INSERT_AFTER);
}

static void walkIElem(elem_struct *i_e)
{
    stat_set_need_init(likely_happen_p(i_e->inip));
    stat_set_has_mem_opnd(false);
    stat_set_trv_nodes(i_e->trv_nodes);

    one_insn_gen_const(i_e->asm_inst);

    stat_set_need_init(false);
    stat_set_trv_nodes(NULL);
}

static void walkElemBlk(blk_struct *blk)
{
    stat_set_opcode(I_none);
    elem_struct *elem = g_array_index(blk->blks, elem_struct *, 0);
    walkElemFuncs[elem->type](elem);
}

void walk_tmplt(void)
{
    blk_struct *blk = (blk_struct *)tmpltm.blk;
    walkBlkFuncs[blk->type](blk);
}


static void elem_free(elem_struct *elem)
{
    if (elem->type == I_ELEM) {
        free(elem->asm_inst);
    } else if (elem->type == C_ELEM) {
        free(elem->c_type);
    }
    free(elem);
}

static void trv_state_free(struct trv_state *trv_state)
{
    g_array_free(trv_state->tk_trees, true);
    g_array_free(trv_state->trv_nodes, true);
    free(trv_state);
}

static void blk_free(blk_struct *blk)
{
    if (blk == NULL)
        return;
    /* free variables */
    for (guint i = 0; i < blk->vars->len; i++) {
        struct blk_var *var = &g_array_index(blk->vars, struct blk_var, i);
        free(var->name);
        free(var->var_type);
        free(var->var_val);
        free(var->init_mem_addr);
    }
    /* free statement specific items */
    if (blk->type == TTT_BLK)
        free(blk->ttt_op);
    else if (blk->type == TRV_BLK)
        trv_state_free(blk->trv_state);
    /* free subblks */
    if (blk->type == SEL_BLK) {
    } else if (blk->type == ELEM_BLK) {
        elem_free(g_array_index(blk->blks, elem_struct *, 0));
    } else {
        for (guint i = 0; i < blk->blks->len; i++) {
            blk_free(g_array_index(blk->blks, void *, i));
        }
    }
    g_array_free(blk->vars, true);
    g_array_free(blk->blks, true);
    free(blk);
}

void tmplt_clear(tmplt_struct *tmpltm)
{
    blk_free((blk_struct *)tmpltm->blk);
}

void tmplt_free(tmplt_struct *tmpltm)
{
    tmplt_clear(tmpltm);
    free(tmpltm);
}

struct const_node *request_trv_node(int opi)
{
    GArray *trv_nodes, *val_nodes;

    trv_nodes = stat_get_trv_nodes();
    if (trv_nodes == NULL)
        return NULL;
    val_nodes = g_array_index(trv_nodes, GArray *, opi);
    return g_array_index(val_nodes, struct const_node *, 0);
}

GArray *request_packed_trv_node(int opi)
{
    GArray *trv_nodes;

    trv_nodes = stat_get_trv_nodes();
    if (trv_nodes == NULL)
        return NULL;
    return g_array_index(trv_nodes, GArray *, opi);
}
