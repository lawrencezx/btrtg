/******************************************************************************
* @File name: parseTmplt.c
* @Author: watchnima
* @Version: 0.1
* @Date: 2020-12-15
* @Description: Parse two kinds of template file:
*   1) Instruction grouping template.
*   2) Test generation template.
******************************************************************************/

#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "parseLib.h"
#include "parseXML.h"
#include "error.h"
#include "tmplt.h"
#include "tk.h"

static const char *xmlfiles[2] =
{
    /* must put group before tempalte, so the tempalte can find relevant instruction group */
    "insn-group.xml",
    "flag-pattern-group.xml"
};
char *tmpltpath = "../xmlmodel/templates";

static struct trv_state *trv_state = NULL;

static void parseGroups(xmlNodePtr groupsNode)
{
    for (xmlNodePtr groupNode = groupsNode->children; groupNode != NULL;
            groupNode = groupNode->next) {
        if (groupNode->type != XML_ELEMENT_NODE)
            continue;

        int i = 0;
        const char *key;
        struct hash_insert hi;
        struct wd_node *inst_group_node;
        struct const_node inst_node;

        inst_group_node = wdtree_node_create();
        inst_group_node->isleaf = true;
        inst_group_node->size = getElemsSize(groupNode->children);

        for (xmlNodePtr iNode = groupNode->children; iNode != NULL; iNode = iNode->next) {
            if (iNode->type != XML_ELEMENT_NODE)
                continue;

            inst_node.type = CONST_ASM_OP;
            inst_node.asm_op = nasm_strdup(nasm_trim((char *)iNode->children->content));
            g_array_append_val(inst_group_node->const_nodes, inst_node);
            i++;
        }

        key = (const char *)xmlGetProp(groupNode, (const unsigned char*)"name");
        hash_find(&hash_wdtrees, key, &hi);
        hash_add(&hi, key, (void *)inst_group_node);
    }
}

static void parseBlk(xmlNodePtr blkNodeStart, blk_struct *blk);

static void parseSeqBlk(xmlNodePtr seqNode, blk_struct *blk)
{
    blk->type = SEQ_BLK;
    parseBlk(seqNode->children, blk);
}

static void parseSelBlk(xmlNodePtr selNode, blk_struct *blk)
{
    int i = 0, weight;
    char *key, *prop_weight;
    struct hash_insert hi;
    struct wd_root *selblk_tree;

    selblk_tree = wdtree_create();
    selblk_tree->wd_node = wdtree_node_create();
    selblk_tree->wd_node->size = getElemsSize(selNode->children);
    GArray *weights = selblk_tree->wd_node->weights;
    GArray *sub_nodes = selblk_tree->wd_node->sub_nodes;

    for (xmlNodePtr blkNode = selNode->children; blkNode != NULL;
        blkNode = blkNode->next) {
        if (blkNode->type != XML_ELEMENT_NODE)
            continue;

        prop_weight = (char *)xmlGetProp(blkNode, (const unsigned char*)"weight");
        weight = atoi(prop_weight);
        g_array_append_val(weights, weight);
        key = (char *)xmlGetProp(blkNode, (const unsigned char*)"type");
        g_array_append_val(sub_nodes, *(struct wd_node **)hash_find(&hash_wdtrees, key, &hi));
        i++;

        free(prop_weight);
        free(key);
    }

    blk->type = SEL_BLK;
    g_array_append_val(blk->blks, selblk_tree);
}

static void parseXfrBlk(xmlNodePtr xfrNode, blk_struct *blk)
{
    char *prop_times, *prop_xfr_op;

    prop_times = (char *)xmlGetProp(xfrNode, (const unsigned char*)"times");
    prop_xfr_op = (char *)xmlGetProp(xfrNode, (const unsigned char*)"type");

    blk->type = XFR_BLK;
    blk->times = atoi(prop_times);
    blk->xfr_op = nasm_strdup(nasm_trim(prop_xfr_op));
    parseBlk(xfrNode->children, blk);

    free(prop_times);
    free(prop_xfr_op);
}

static void parseRptBlk(xmlNodePtr rptNode, blk_struct *blk)
{
    char *prop_times;

    prop_times = (char *)xmlGetProp(rptNode, (const unsigned char*)"times");

    blk->type = RPT_BLK;
    blk->times = atoi(prop_times);
    parseBlk(rptNode->children, blk);

    free(prop_times);
}

static void parseTrvBlk(xmlNodePtr trvNode, blk_struct *blk)
{
    blk->type = TRV_BLK;
    parseBlk(trvNode->children, blk);
    blk->trv_state = trv_state;
    trv_state = NULL;
}

/******************************************************************************
*
* Function name: parseV
* Description: parse an <V> tag and append it to blk_struct->vars
*              An <V> tag has two attribute:
*              (1) var: string, name of the variable
*              (2) type: string, type of the variable
* Parameter:
*       @VNode: input, a xml variable node
*       @blk: return, the parent blk_struct
* Return: none
******************************************************************************/
static void parseV(xmlNodePtr VNode, blk_struct *blk)
{
    char *prop_var, *prop_var_type;
    struct blk_var var;

    prop_var = (char *)xmlGetProp(VNode, (const unsigned char*)"var");
    prop_var_type = (char *)xmlGetProp(VNode, (const unsigned char*)"type");

    init_blk_var(&var);
    var.name = nasm_strdup(prop_var);
    var.var_type = nasm_strdup(prop_var_type);
    g_array_append_val(blk->vars, var);

    free(prop_var);
    free(prop_var_type);
}

/******************************************************************************
*
* Function name: parseG
* Description: parse an <G> tag into a blk_struct
*              An <G> tag has two attribute:
*              (1) type: string, instruction group identifier
*              (2) inip: double, the probability to initilize it's operand
* Parameter:
*       @GNode: input, a xml instruction group node
*       @blk: return, the return blk_struct
* Return: none
******************************************************************************/
static void parseG(xmlNodePtr GNode, blk_struct *blk)
{
    char *g_type;
    elem_struct *g_e;
    struct hash_insert hi;
    char *prop_type, *prop_inip;

    prop_type = (char *)xmlGetProp(GNode, (const unsigned char*)"type");
    prop_inip = (char *)xmlGetProp(GNode, (const unsigned char*)"inip");

    g_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    init_elem_struct(g_e);
    g_e->type = G_ELEM;
    g_type = nasm_trim(prop_type);
    g_e->g_tree = wdtree_create();
    g_e->g_tree->wd_node = *(struct wd_node **)hash_find(&hash_wdtrees, g_type, &hi);
    g_e->inip = (prop_inip == NULL) ? 0.0 : atof(prop_inip);

    blk->type = ELEM_BLK;
    g_array_append_val(blk->blks, g_e);

    free(prop_type);
    free(prop_inip);
}

/******************************************************************************
*
* Function name: parseC
* Description: parse an <C> tag into a blk_struct
*              An <C> tag has one attribute:
*              (1) type: string, checking value
* Parameter:
*       @GNode: input, a xml checking node
*       @blk: return, the return blk_struct
* Return: none
******************************************************************************/
static void parseC(xmlNodePtr CNode, blk_struct *blk)
{
    char *c_type;
    elem_struct *c_e;
    char *prop_type;

    prop_type = (char *)xmlGetProp(CNode, (const unsigned char*)"type");

    c_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    init_elem_struct(c_e);
    c_e->type = C_ELEM;
    c_type = nasm_trim(prop_type);
    c_e->c_type = nasm_strdup(c_type);

    blk->type = ELEM_BLK;
    g_array_append_val(blk->blks, c_e);

    free(prop_type);
}

/******************************************************************************
*
* Function name: parseI
* Description: parse an <I> tag into a blk_struct
*              An <I> tag has two attribute:
*              (1) type: string, pseudo assembler code
*              (2) inip: double, the probability to initilize it's operand
* Parameter:
*       @INode: input, a xml instruction node
*       @blk: return, the return blk_struct
* Return: none
******************************************************************************/
static void parseI(xmlNodePtr INode, blk_struct *blk)
{
    elem_struct *i_e;
    char *prop_type, *prop_inip, *prop_trv;

    prop_type = (char *)xmlGetProp(INode, (const unsigned char*)"type");
    prop_inip = (char *)xmlGetProp(INode, (const unsigned char*)"inip");
    prop_trv = (char *)xmlGetProp(INode, (const unsigned char*)"trv");

    i_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    init_elem_struct(i_e);
    i_e->type = I_ELEM;
    i_e->asm_inst = nasm_strdup(nasm_trim(prop_type));
    i_e->inip = (prop_inip == NULL) ? 0.0 : atof(prop_inip);
    if (prop_trv != NULL && strcmp(nasm_trim(prop_trv), "true") == 0) {
        trv_state = (struct trv_state *)nasm_malloc(sizeof(struct trv_state));
        create_trv_state(i_e->asm_inst, trv_state);
        i_e->val_nodes = trv_state->val_nodes;
        i_e->inip = 1.0;
    }

    blk->type = ELEM_BLK;
    g_array_append_val(blk->blks, i_e);

    free(prop_type);
    free(prop_inip);
    free(prop_trv);
}

static void parseBlk(xmlNodePtr blkNodeStart, blk_struct *blk)
{
    blk_struct *subblk;
    const char *blk_name;

    for (xmlNodePtr blkNode = blkNodeStart; blkNode != NULL;
        blkNode = blkNode->next) {
        if (blkNode->type != XML_ELEMENT_NODE)
            continue;

        blk_name = (const char *)blkNode->name;
        subblk = (blk_struct *)nasm_malloc(sizeof(blk_struct));
        init_blk_struct(subblk);
        subblk->parent = blk;
        if (strcmp(blk_name, "sequence") == 0) {
            parseSeqBlk(blkNode, subblk);
        } else if (strcmp(blk_name, "select") == 0) {
            parseSelBlk(blkNode, subblk);
        } else if (strcmp(blk_name, "transfer") == 0) {
            parseXfrBlk(blkNode, subblk);
        } else if (strcmp(blk_name, "repeat") == 0) {
            parseRptBlk(blkNode, subblk);
        } else if (strcmp(blk_name, "traverse") == 0) {
            parseTrvBlk(blkNode, subblk);
        } else if (strcmp(blk_name, "V") == 0) {
            parseV(blkNode, blk);
            nasm_free(subblk);
            continue;
        } else if (strcmp(blk_name, "G") == 0) {
            parseG(blkNode, subblk);
        } else if (strcmp(blk_name, "C") == 0) {
            parseC(blkNode, subblk);
        } else if (strcmp(blk_name, "I") == 0) {
            parseI(blkNode, subblk);
        } else {
            nasm_fatal("Unsupported statement type: %s", blk_name);
        }

        g_array_append_val(blk->blks, subblk);
    }
}

static void parseTmplts(xmlNodePtr tmpltsNode)
{
    tmpltm.blk = (blk_struct *)nasm_malloc(sizeof(blk_struct));
    init_blk_struct(tmpltm.blk);
    parseSeqBlk(tmpltsNode, tmpltm.blk);
}

void parse_tmplts_file(const char *fname)
{
    LIBXML_TEST_VERSION
    xmlDocPtr doc = xmlParseFile(fname);
    const char *tmplt_type;
    if (doc != NULL) {
        xmlNodePtr node = doc->children;
        while (node != NULL && node->type != XML_ELEMENT_NODE)
            node = node->next;
        if (node == NULL)
            return;
        if (node->type == XML_ELEMENT_NODE) {
            tmplt_type = (const char *)node->name;
            if (strcmp(tmplt_type, "InsnGroups") == 0) {
                parseGroups(node);
            } else if (strcmp(tmplt_type, "Template") == 0) {
                parseTmplts(node);
            } else {
                nasm_fatal("failed to parse tempalte type: %s", tmplt_type);
            }
        }
    } else {
        nasm_fatal("Unable to open %s\n", fname);
    }
    xmlFreeDoc(doc);
}

void init_tmplts(void)
{
#ifdef LIBXML_READER_ENABLED
    char fname[1024];
    for (size_t i = 0; i < ARRAY_SIZE(xmlfiles); i++) {
        sprintf(fname, "%s/%s", tmpltpath, xmlfiles[i]);
        parse_tmplts_file(fname);
        memset(fname, 0, 1024);
    }
#else
    nasm_fatal("XInclude support not compiled in\n");
#endif
}

