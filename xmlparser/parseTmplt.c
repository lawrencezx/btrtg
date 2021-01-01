#include "compiler.h"

#include <libxml/parser.h>
#include "nasm.h"
#include "nasmlib.h"
#include "parseXML.h"
#include "error.h"
#include "tmplt.h"

static const char *xmlfiles[2] =
{
    /* must put class before tempalte, so the tempalte can find relevant instruction group */
    "insn-class.xml",
    "template-sample.xml"
};
char *tmpltpath = "../xmlmodel/templates";


static int getElemsSize(xmlNodePtr node)
{
    int num = 0;
    for (; node != NULL; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            num++;
        }
    }
    return num;
}

static void parseClasses(xmlNodePtr classesNode)
{
    for (xmlNodePtr classNode = classesNode->children; classNode != NULL; classNode = classNode->next) {
        if (classNode->type != XML_ELEMENT_NODE)
            continue;

        int i = 0;
        WDTree *classTree;
        constVal *classVal;
        const char *key;
        struct hash_insert hi;

        classTree = wdtree_create();
        classTree->isleaf = true;
        classTree->size = getElemsSize(classNode->children);
        classTree->consts = (constVal *)nasm_malloc(classTree->size * sizeof(constVal));
        classVal = classTree->consts;

        for (xmlNodePtr iNode = classNode->children; iNode != NULL; iNode = iNode->next) {
            if (iNode->type != XML_ELEMENT_NODE)
                continue;

            classVal[i].instName = trim((const char *)iNode->children->content);
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
    int *weights;
    const char *key;
    struct hash_insert hi;
    WDTree *selTree;
    WDTree **subtrees;

    selTree = wdtree_create();
    selTree->size = getElemsSize(selNode->children);
    selTree->weights = (int *)nasm_malloc(selTree->size * sizeof(int));
    selTree->children = (WDTree **)nasm_malloc(selTree->size * sizeof(WDTree *));
    weights = selTree->weights;
    subtrees = selTree->children;

    for (xmlNodePtr blkNode = selNode->children; blkNode != NULL; blkNode = blkNode->next) {
        if (blkNode->type != XML_ELEMENT_NODE)
            continue;

        weights[i] = atoi((const char *)xmlGetProp(blkNode, (const unsigned char*)"weight"));
        key = (const char*)trim((const char *)xmlGetProp(blkNode, (const unsigned char*)"type"));
        subtrees[i] = *(WDTree **)hash_find(&hash_wdtrees, key, &hi);
        i++;
    }

    blk->type = SEL_BLK;
    blk->blks = (void **)nasm_malloc(sizeof(void *));
    blk->blks[0] = (void *)selTree;
}

static void parseXfrBlk(xmlNodePtr xfrNode, blk_struct *blk)
{
    blk->type = XFR_BLK;
    blk->times = atoi((const char *)xmlGetProp(xfrNode, (const unsigned char*)"times"));
    blk->xfrName = nasm_strdup(trim((const char *)xmlGetProp(xfrNode, (const unsigned char*)"type")));
    parseBlk(xfrNode->children, blk);
}

static void parseRptBlk(xmlNodePtr rptNode, blk_struct *blk)
{
    blk->type = RPT_BLK;
    blk->times = atoi((const char *)xmlGetProp(rptNode, (const unsigned char*)"times"));
    parseBlk(rptNode->children, blk);
}

static void parseIset(xmlNodePtr IsetNode, blk_struct *blk)
{
    const char *isetType;
    elem_struct *iset_e;
    struct hash_insert hi;

    iset_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    iset_e->type = ISET_ELEM;
    isetType = (const char*)trim((const char *)xmlGetProp(IsetNode, (const unsigned char*)"type"));
    iset_e->wdtree = *(WDTree **)hash_find(&hash_wdtrees, isetType, &hi);

    blk->type = ELEM_BLK;
    blk->blks = (void **)nasm_malloc(sizeof(void *));
    blk->blks[0] = (void *)iset_e;
}

static void parsePrint(xmlNodePtr PrintNode, blk_struct *blk)
{
    const char *printType;
    elem_struct *print_e;

    print_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    print_e->type = PRINT_ELEM;
    printType = (const char*)trim((const char *)xmlGetProp(PrintNode, (const unsigned char*)"type"));
    if (strcmp(printType, "all_state") == 0) {
        print_e->printType = ALL_STATE;
    } else if (strcmp(printType, "x86_state") == 0) {
        print_e->printType = X86_STATE;
    } else if (strcmp(printType, "x87_state") == 0) {
        print_e->printType = X87_STATE;
    } else {
        nasm_fatal("Unsupported print type: %s", printType);
    }

    blk->type = ELEM_BLK;
    blk->blks = (void **)nasm_malloc(sizeof(void *));
    blk->blks[0] = (void *)print_e;
}

static void parseI(xmlNodePtr INode, blk_struct *blk)
{
    const char *iType;
    elem_struct *i_e;

    i_e = (elem_struct *)nasm_malloc(sizeof(elem_struct));
    i_e->type = INSN_ELEM;
    iType = (const char*)trim((const char *)xmlGetProp(INode, (const unsigned char*)"type"));
    i_e->inst = nasm_strdup(iType);

    blk->type = ELEM_BLK;
    blk->blks = (void **)nasm_malloc(sizeof(void *));
    blk->blks[0] = (void *)i_e;
}

static void parseBlk(xmlNodePtr blkNodeStart, blk_struct *blk)
{
    for (xmlNodePtr blkNode = blkNodeStart; blkNode != NULL; blkNode = blkNode->next) {
        if (blkNode->type != XML_ELEMENT_NODE)
            continue;

        blk_struct *subblk;
        const char *blkName;

        blkName = (const char *)blkNode->name;
        subblk = (blk_struct *)nasm_malloc(sizeof(blk_struct));
        init_blk_struct(subblk);
        if (strcmp(blkName, "sequence") == 0) {
            parseSeqBlk(blkNode, subblk);
        } else if (strcmp(blkName, "select") == 0) {
            parseSelBlk(blkNode, subblk);
        } else if (strcmp(blkName, "transfer") == 0) {
            parseXfrBlk(blkNode, subblk);
        } else if (strcmp(blkName, "repeat") == 0) {
            parseRptBlk(blkNode, subblk);
        } else if (strcmp(blkName, "Iset") == 0) {
            parseIset(blkNode, subblk);
        } else if (strcmp(blkName, "Print") == 0) {
            parsePrint(blkNode, subblk);
        } else if (strcmp(blkName, "I") == 0) {
            parseI(blkNode, subblk);
        } else {
            nasm_fatal("Unsupported statement type: %s", blkName);
        }

        if (blk->blks == NULL) {
            blk->blks = (void **)nasm_malloc(sizeof(void *));
        } else {
            blk->blks = (void **)nasm_realloc(blk->blks, (blk->num + 1) * sizeof(void *));
        }
        blk->blks[blk->num] = (void *)subblk;
        blk->num++;
    }
}

static void parseTmplts(xmlNodePtr tmpltsNode)
{
    tmpltm.blk = (blk_struct *)nasm_malloc(sizeof(blk_struct));
    init_blk_struct(tmpltm.blk);
    parseSeqBlk(tmpltsNode, tmpltm.blk);
}

static void parseXML_file(const char *fname)
{
    LIBXML_TEST_VERSION
    xmlDocPtr doc = xmlParseFile(fname);
    if (doc != NULL) {
        xmlNodePtr node = doc->children;
        if (node->type == XML_ELEMENT_NODE) {
            const char *nodeName = (const char *)node->name;
            if (strcmp(nodeName, "InsnClass") == 0) {
                parseClasses(node);
            } else if (strcmp(nodeName, "Template") == 0) {
                parseTmplts(node);
            } else {
                nasm_nonfatal("failed to parse element: %s", nodeName);
            }
        }
    } else {
        nasm_nonfatal("Unable to open %s\n", fname);
    }
    xmlFreeDoc(doc);
}

void parse_tmplts(void)
{
#ifdef LIBXML_READER_ENABLED
    char fname[1024];
    for (size_t i = 0; i < ARRAY_SIZE(xmlfiles); i++) {
        sprintf(fname, "%s/%s", tmpltpath, xmlfiles[i]);
        parseXML_file(fname);
        memset(fname, 0, 1024);
    }
#else
    nasm_fatal("XInclude support not compiled in\n");
#endif
}

