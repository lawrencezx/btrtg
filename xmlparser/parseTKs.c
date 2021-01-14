#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "parseLib.h"
#include "parseXML.h"
#include "error.h"
#include "tk.h"

static const char *xmlfiles[2] =
{
    /* must put Consts before TK, so the TK can find relevant constGroup */
    //"fixedpointConsts.xml",
    //"fixedpointTK.xml"
    "floatpointConsts.xml",
    "floatpointTK.xml"
};
char *TKpath = "../xmlmodel/tks";


static void parseCGs(xmlNodePtr cgsNode)
{
    int i = 0, imm;
    char *key;
    struct hash_insert hi;
    struct wd_node *cg_tree_node;
    struct const_node val_node;

    for (xmlNodePtr cgNode = cgsNode->children; cgNode != NULL; cgNode = cgNode->next) {
        if (cgNode->type != XML_ELEMENT_NODE)
            continue;

        cg_tree_node = wdtree_node_create();
        cg_tree_node->isleaf = true;
        cg_tree_node->size = getElemsSize(cgNode->children);

        for (xmlNodePtr cNode = cgNode->children; cNode != NULL; cNode = cNode->next) {
            if (cNode->type != XML_ELEMENT_NODE)
                continue;


            imm = hex2dec((const char *)cNode->children->content);
            const char *cNodeName = (const char *)cNode->name;
            if(strcmp(cNodeName, "Imm64") == 0){
                val_node.type = CONST_IMM64;
                hex2double((const char *)cNode->children->content, val_node.imm64);
                g_array_append_val(cg_tree_node->const_nodes, val_node);
            }else if(strcmp(cNodeName, "Imm80")==0){
                val_node.type = CONST_IMM80;
                hex2ldouble((const char *)cNode->children->content, val_node.imm80);
                g_array_append_val(cg_tree_node->const_nodes, val_node);
            }else{
                val_node.type = CONST_IMM32;
                val_node.imm32 = (uint32_t)imm;
                g_array_append_val(cg_tree_node->const_nodes, val_node);
            }

            //const char *cNodeName = (const char *)cNode->name;
            //if (strcmp(cNodeName, "Imm8") == 0) {
            //    cgVal[i].imm8 = (uint8_t)imm;
            //} else if (strcmp(cNodeName, "Unity") == 0) {
            //    cgVal[i].unity = (uint8_t)imm;
            //} else if (strcmp(cNodeName, "Imm16") == 0) {
            //    cgVal[i].imm16 = (uint16_t)imm;
            //} else if (strcmp(cNodeName, "Imm32") == 0) {
            //} else {
            //    printf("0x%x\n", imm);
            //}
            i++;
        }

        key = (char *)xmlGetProp(cgNode, (const unsigned char*)"name");
        hash_find(&hash_wdtrees, key, &hi);
        hash_add(&hi, key, (void *)cg_tree_node);
    }
}

static struct wd_node *parseTK(xmlNodePtr tkNode)
{
    int i = 0, weight;
    char *propWeight, *propKey;
    struct hash_insert hi;
    struct wd_node *tk_tree_node;

    tk_tree_node = wdtree_node_create();
    tk_tree_node->size = getElemsSize(tkNode->children);
    GArray *weights = tk_tree_node->weights;
    GArray *sub_nodes = tk_tree_node->sub_nodes;

    for (xmlNodePtr nNode = tkNode->children; nNode != NULL; nNode = nNode->next) {
        if (nNode->type != XML_ELEMENT_NODE)
            continue;

        propWeight = (char *)xmlGetProp(nNode, (const unsigned char*)"weight");
        propKey = (char *)xmlGetProp(nNode, (const unsigned char*)"name");
        weight = atoi(propWeight);
        g_array_append_val(weights, weight);
        g_array_append_val(sub_nodes, *(struct wd_node **)hash_find(&hash_wdtrees, propKey, &hi));
        i++;

        free(propWeight);
        free(propKey);
    }

    return tk_tree_node;
}

static void parseTKs(xmlNodePtr tksNode)
{
    char *key;
    struct hash_insert hi;
    struct tk_model *tkm;
    struct wd_node *tkTree;

    for (xmlNodePtr tkNode = tksNode->children; tkNode != NULL; tkNode = tkNode->next) {
        if (tkNode->type != XML_ELEMENT_NODE)
            continue;

        tkm = tkmodel_create();

        tkTree = parseTK(tkNode);
        tkm->tk_tree = wdtree_create();
        tkm->tk_tree->wd_node = tkTree;

        key = (char *)xmlGetProp(tkNode, (const unsigned char*)"inst");
        hash_find(&hash_tks, key, &hi);
        hash_add(&hi, key, (void *)tkm);
    }
}

void parse_tks_file(const char *fname)
{
    LIBXML_TEST_VERSION
    xmlDocPtr doc = xmlParseFile(fname);
    const char *tk_type;

    if (doc != NULL) {
        xmlNodePtr node = doc->children;
        if (node->type == XML_ELEMENT_NODE) {
            tk_type = (const char *)node->name;
            if (strcmp(tk_type, "ConstGroups") == 0) {
                parseCGs(node);
            } else if (strcmp(tk_type, "TestingKnowledges") == 0) {
                parseTKs(node);
            } else {
                nasm_nonfatal("failed to parse element: %s", tk_type);
            }
        }
    } else {
        nasm_nonfatal("Unable to open %s\n", fname);
    }
    xmlFreeDoc(doc);
}

void init_tks(void)
{
#ifdef LIBXML_READER_ENABLED
    char fname[1024];
    for (size_t i = 0; i < ARRAY_SIZE(xmlfiles); i++) {
        sprintf(fname, "%s/%s", TKpath, xmlfiles[i]);
        parse_tks_file(fname);
        memset(fname, 0, 1024);
    }
#else
    nasm_fatal("XInclude support not compiled in\n");
#endif
}

