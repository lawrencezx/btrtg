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

static const char *xmlfiles[1] =
{
    /* must put class before tempalte, so the tempalte can find relevant instruction group */
    "insn-group.xml",
};
char *tmpltpath = "../xmlmodel/templates";


static void parseClasses(xmlNodePtr classesNode)
{
    for (xmlNodePtr classNode = classesNode->children; classNode != NULL; classNode = classNode->next) {
        if (classNode->type != XML_ELEMENT_NODE)
            continue;

        int i = 0;
        WDTree *classTree;
        const char *key;
        struct hash_insert hi;

        classTree = wdtree_create();
        classTree->isleaf = true;
        classTree->size = getElemsSize(classNode->children);

        for (xmlNodePtr iNode = classNode->children; iNode != NULL; iNode = iNode->next) {
            if (iNode->type != XML_ELEMENT_NODE)
                continue;

            constVal classVal;
            classVal.type = CONST_INSN;
            classVal.instName = nasm_strdup(nasm_trim((char *)iNode->children->content));
            g_array_append_val(classTree->consts, classVal);
            i++;
        }

        key = (const char *)xmlGetProp(classNode, (const unsigned char*)"name");
        hash_find(&hash_wdtrees, key, &hi);
        hash_add(&hi, key, (void *)classTree);
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
    int i = 0;
    char *key;
    struct hash_insert hi;
    WDTree *selTree;

    selTree = wdtree_create();
    selTree->size = getElemsSize(selNode->children);
    GArray *weights = selTree->weights;
    GArray *subtrees = selTree->subtrees;

    for (xmlNodePtr blkNode = selNode->children; blkNode != NULL;
        blkNode = blkNode->next) {
        if (blkNode->type != XML_ELEMENT_NODE)
            continue;

        char *prop_weight;

        prop_weight = (char *)xmlGetProp(blkNode, (const unsigned char*)"weight");
        int weight = atoi(prop_weight);
        g_array_append_val(weights, weight);
        key = (char *)xmlGetProp(blkNode, (const unsigned char*)"type");
        g_array_append_val(subtrees, *(WDTree **)hash_find(&hash_wdtrees, key, &hi));
        i++;

        free(prop_weight);
        free(key);
    }

    blk->type = SEL_BLK;
    g_array_append_val(blk->blks, selTree);
}

static void parseXfrBlk(xmlNodePtr xfrNode, blk_struct *blk)
{
    char *prop_times, *propXfrName;

    prop_times = (char *)xmlGetProp(xfrNode, (const unsigned char*)"times");
    propXfrName = (char *)xmlGetProp(xfrNode, (const unsigned char*)"type");

    blk->type = XFR_BLK;
    blk->times = atoi(prop_times);
    blk->xfrName = nasm_strdup(nasm_trim(propXfrName));
    parseBlk(xfrNode->children, blk);

    free(prop_times);
    free(propXfrName);
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
    char *prop_var, *prop_var_type;

    prop_var = (char *)xmlGetProp(trvNode, (const unsigned char*)"var");
    prop_var_type = (char *)xmlGetProp(trvNode, (const unsigned char*)"type");

    blk_var var;
    init_blk_var(&var);
    var.name = nasm_strdup(prop_var);
    var.asm_var = nasm_strdup(prop_var_type);
    g_array_append_val(blk->vars, var);

    blk->type = TRV_BLK;
    parseBlk(trvNode->children, blk);

    free(prop_var);
    free(prop_var_type);
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

    prop_var = (char *)xmlGetProp(VNode, (const unsigned char*)"var");
    prop_var_type = (char *)xmlGetProp(VNode, (const unsigned char*)"type");

    blk_var var;
    init_blk_var(&var);
    var.name = nasm_strdup(prop_var);
    var.asm_var = nasm_strdup(prop_var_type);
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
    char *gType;
    elem_struct *g_e;
    struct hash_insert hi;
    char *prop_type, *prop_inip;

    prop_type = (char *)xmlGetProp(GNode, (const unsigned char*)"type");
    prop_inip = (char *)xmlGetProp(GNode, (const unsigned char*)"inip");

    g_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    g_e->type = G_ELEM;
    gType = nasm_trim(prop_type);
    g_e->wdtree = *(WDTree **)hash_find(&hash_wdtrees, gType, &hi);
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
    char *checkType;
    elem_struct *c_e;
    char *prop_type;

    prop_type = (char *)xmlGetProp(CNode, (const unsigned char*)"type");

    c_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    c_e->type = C_ELEM;
    checkType = nasm_trim(prop_type);
    c_e->checkType = nasm_strdup(checkType);

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
    char *prop_type, *prop_inip;

    prop_type = (char *)xmlGetProp(INode, (const unsigned char*)"type");
    prop_inip = (char *)xmlGetProp(INode, (const unsigned char*)"inip");

    i_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    i_e->type = I_ELEM;
    i_e->inst = nasm_strdup(nasm_trim(prop_type));
    i_e->inip = (prop_inip == NULL) ? 0.0 : atof(prop_inip);

    blk->type = ELEM_BLK;
    g_array_append_val(blk->blks, i_e);

    free(prop_type);
    free(prop_inip);
}

static void parseBlk(xmlNodePtr blkNodeStart, blk_struct *blk)
{
    for (xmlNodePtr blkNode = blkNodeStart; blkNode != NULL;
        blkNode = blkNode->next) {
        if (blkNode->type != XML_ELEMENT_NODE)
            continue;

        blk_struct *subblk;
        const char *blkName;

        blkName = (const char *)blkNode->name;
        subblk = (blk_struct *)nasm_malloc(sizeof(blk_struct));
        init_blk_struct(subblk);
        subblk->parent = blk;
        if (strcmp(blkName, "sequence") == 0) {
            parseSeqBlk(blkNode, subblk);
        } else if (strcmp(blkName, "select") == 0) {
            parseSelBlk(blkNode, subblk);
        } else if (strcmp(blkName, "transfer") == 0) {
            parseXfrBlk(blkNode, subblk);
        } else if (strcmp(blkName, "repeat") == 0) {
            parseRptBlk(blkNode, subblk);
        } else if (strcmp(blkName, "traverse") == 0) {
            parseTrvBlk(blkNode, subblk);
        } else if (strcmp(blkName, "V") == 0) {
            parseV(blkNode, blk);
            nasm_free(subblk);
            continue;
        } else if (strcmp(blkName, "G") == 0) {
            parseG(blkNode, subblk);
        } else if (strcmp(blkName, "C") == 0) {
            parseC(blkNode, subblk);
        } else if (strcmp(blkName, "I") == 0) {
            parseI(blkNode, subblk);
        } else {
            nasm_fatal("Unsupported statement type: %s", blkName);
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
    if (doc != NULL) {
        xmlNodePtr node = doc->children;
        if (node->type == XML_ELEMENT_NODE) {
            const char *nodeName = (const char *)node->name;
            if (strcmp(nodeName, "InsnGroups") == 0) {
                parseClasses(node);
            } else if (strcmp(nodeName, "Template") == 0) {
                parseTmplts(node);
            } else {
                nasm_fatal("failed to parse element: %s", nodeName);
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

