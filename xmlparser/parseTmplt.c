#include "compiler.h"

#include <libxml/parser.h>
#include "nasm.h"
#include "nasmlib.h"
#include "parseXML.h"
#include "error.h"
#include "model.h"

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

static void parseTmplts(xmlNodePtr tmpltsNode)
{
    xmlNodePtr tmpltNode;

    for (tmpltNode = tmpltsNode->children; tmpltNode != NULL; tmpltNode = tmpltNode->next) {
        if (tmpltNode->type != XML_ELEMENT_NODE)
            continue;

        if (strcmp((const char *)tmpltNode->name, "TestLength") == 0) {
            tmpltm.instNum = atoi((const char *)xmlGetProp(tmpltNode, (const unsigned char*)"value"));
            tmpltNode = tmpltNode->next;
            break;
        }
    }

    int i = 0;
    int *weights;
    const char *key;
    struct hash_insert hi;
    WDTree *tmpltTree;
    WDTree **subtrees;

    tmpltTree = wdtree_create();
    tmpltTree->size = getElemsSize(tmpltNode);
    tmpltTree->weights = (int *)nasm_malloc(tmpltTree->size * sizeof(int));
    tmpltTree->children = (WDTree **)nasm_malloc(tmpltTree->size * sizeof(WDTree *));
    weights = tmpltTree->weights;
    subtrees = tmpltTree->children;

    for (; tmpltNode != NULL; tmpltNode = tmpltNode->next) {
        if (tmpltNode->type != XML_ELEMENT_NODE)
            continue;

        weights[i] = atoi((const char *)xmlGetProp(tmpltNode, (const unsigned char*)"weight"));
        key = (const char*)trim((const char *)xmlGetProp(tmpltNode, (const unsigned char*)"name"));
        subtrees[i] = *(WDTree **)hash_find(&hash_wdtrees, key, &hi);
        i++;
    }
    
    tmpltm.wdtree = tmpltTree;
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

