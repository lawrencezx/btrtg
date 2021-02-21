#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "generator.h"
#include "tmplt.h"
#include "x86pg.h"

tmplt_struct tmpltm;

void init_blk_struct(blk_struct *blk)
{
    blk->parent = NULL;
    blk->type = -1;
    blk->xfr_op = NULL;
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
}

void init_elem_struct(elem_struct *elem)
{
    elem->inip = 0;
    elem->type = NONE_ELEM;
    elem->g_tree = NULL;
    elem->c_type = NULL;
    elem->asm_inst = NULL;
    elem->val_nodes = NULL;
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
static void walkXfrBlk(blk_struct *blk);
static void walkRptBlk(blk_struct *blk);
static void walkTrvBlk(blk_struct *blk);
static void walkElemBlk(blk_struct *blk);

static void (*walkBlkFuncs[])(blk_struct *) =
{
    walkSeqBlk,
    walkSelBlk,
    walkXfrBlk,
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

static void walkXfrBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);
    blk_invalid_var_all(blk);
    /* TODO */
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

static void preorder_traverse_trv_state(blk_struct *blk, struct wd_node *tk_node, int num)
{
    struct trv_state *trv_state = blk->trv_state;
    struct const_node *val_node;
    struct wd_root *tk_tree;

    if (tk_node == NULL || tk_node->size == 0)
        return;

    for (int i = 0; i < tk_node->size; i++) {
        if (tk_node->isleaf) {
            val_node = &g_array_index(tk_node->const_nodes, struct const_node, i);
            g_array_append_val(trv_state->val_nodes, val_node);
            if ((size_t)num + 1 == trv_state->tk_trees->len) {
                blk_invalid_var_all(blk);
                for (guint i = 0; i < blk->blks->len; i++) {
                    blk_struct *subblk = g_array_index(blk->blks, blk_struct *, i);
                    walkBlkFuncs[subblk->type](subblk);
                }
            } else {
                tk_tree = g_array_index(trv_state->tk_trees, struct wd_root *, num + 1);
                preorder_traverse_trv_state(blk, tk_tree->wd_node, num + 1);
            }
            g_array_remove_index(trv_state->val_nodes, num);
        } else {
            preorder_traverse_trv_state(blk, g_array_index(tk_node->sub_nodes,
                        struct wd_node *, i), num);
        }
    }
}

static void walkTrvBlk(blk_struct *blk)
{
    stat_set_curr_blk(blk);
    
    GArray *tk_trees = blk->trv_state->tk_trees;
    if (tk_trees->len != 0) {
        struct wd_root *tk_tree = g_array_index(tk_trees, struct wd_root *, 0);
        preorder_traverse_trv_state(blk, tk_tree->wd_node, 0);
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
    stat_set_val_nodes(i_e->val_nodes);

    one_insn_gen_const(i_e->asm_inst);

    stat_set_need_init(false);
    stat_set_val_nodes(NULL);
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
    if (elem->type == I_ELEM)
        free(elem->asm_inst);
    free(elem);
}

static void blk_free(blk_struct *blk)
{
    if (blk == NULL)
        return;
    for (guint i = 0; i < blk->vars->len; i++) {
        struct blk_var *var = &g_array_index(blk->vars, struct blk_var, i);
        free(var->name);
        free(var->var_type);
        free(var->var_val);
        free(var->init_mem_addr);
    }
    if (blk->type == SEL_BLK) {
    } else if (blk->type == ELEM_BLK) {
        elem_free(g_array_index(blk->blks, elem_struct *, 0));
    } else {
        for (guint i = 0; i < blk->blks->len; i++) {
            blk_free(g_array_index(blk->blks, void *, i));
        }
    }
    free(blk->xfr_op);
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
