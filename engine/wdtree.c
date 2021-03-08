#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "wdtree.h"

struct hash_table hash_wdtrees;

static struct wd_node **wd_nodes_tempstorage = NULL;
static int wd_nodes_tempsize = 0, wd_nodes_templen = 0;
#define WDTREES_TEMP_DELTA 128

struct wd_root *wdtree_create(void)
{
    struct wd_root *tree;
    tree = (struct wd_root *)nasm_malloc(sizeof(struct wd_root));
    tree->packedn = 1;
    tree->wd_node = NULL;
    return tree;
}

struct wd_node *wdtree_node_create(void)
{
    struct wd_node *wd_node;
    wd_node = (struct wd_node *)nasm_malloc(sizeof(struct wd_node));
    wd_node->isleaf = false;
    wd_node->size = 0;
    wd_node->weights = g_array_new(FALSE, FALSE, sizeof(int));
    wd_node->sub_nodes = g_array_new(FALSE, FALSE, sizeof(struct wd_node *));
    wd_node->const_nodes = g_array_new(FALSE, FALSE, sizeof(struct const_node));

    if (wd_nodes_templen >= wd_nodes_tempsize) {
        wd_nodes_tempsize += WDTREES_TEMP_DELTA;
        wd_nodes_tempstorage = nasm_realloc(wd_nodes_tempstorage,
                wd_nodes_tempsize * sizeof(struct wd_node *));
    }
    wd_nodes_tempstorage[wd_nodes_templen++] = wd_node;

    return wd_node;
}

static void const_node_clear(struct const_node *const_node)
{
    if (const_node->type == CONST_ASM_OP) {
        free(const_node->asm_op);
    }
}

void wdtree_node_clear(struct wd_node *wd_node)
{
    g_array_free(wd_node->weights, true);
    if (wd_node->isleaf == true) {
        for (int i = 0; i < wd_node->size; i++) {
            const_node_clear(&g_array_index(wd_node->const_nodes, struct const_node, i));
        }
        g_array_free(wd_node->const_nodes, true);
    } else {
        g_array_free(wd_node->sub_nodes, true);
    }
}

static void hash_wdtrees_clear(void)
{
    struct hash_iterator it;
    const struct hash_node *np;

    hash_for_each(&hash_wdtrees, it, np) {
        nasm_free((void *)np->key);
    }

    hash_free(&hash_wdtrees);
}

void wdtrees_free_all(void)
{
    for (int i = 0; i < wd_nodes_templen; i++) {
        wdtree_node_clear(wd_nodes_tempstorage[i]);
        free(wd_nodes_tempstorage[i]);
    }
    free(wd_nodes_tempstorage);
    wd_nodes_tempstorage = NULL;

    hash_wdtrees_clear();
}

static int select_subtree(GArray *weights, int len)
{
    if (weights->len == 0) {
        return nasm_random32(len);
    }
    static const int total_weight = 100;
    int subtree, accWeight = 0;
    int random_chi = nasm_random32(total_weight);
    for (subtree = 0; subtree < len; subtree++) {
        accWeight += g_array_index(weights, int, subtree);
        if (random_chi < accWeight)
            break;
    }
    return subtree;
}

static struct const_node *wdtree_select_leaf_node_recursive(struct wd_node *wd_node)
{
    int subtree;
    struct wd_node *subnode;

    if (wd_node == NULL || wd_node->size == 0)
        return NULL;

    subtree = select_subtree(wd_node->weights, wd_node->size);
    if (wd_node->isleaf)
        return &g_array_index(wd_node->const_nodes, struct const_node, subtree);
    subnode = g_array_index(wd_node->sub_nodes, struct wd_node *, subtree);
    return wdtree_select_leaf_node_recursive(subnode);
}

struct const_node *wdtree_select_leaf_node(struct wd_root *tree)
{
    return wdtree_select_leaf_node_recursive(tree->wd_node);
}

