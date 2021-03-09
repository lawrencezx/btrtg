#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "parseLib.h"
#include "parseXML.h"
#include "error.h"
#include "tk.h"

static const char *xmlfiles[6] =
{
    /* must put Consts before TK, so the TK can find relevant constGroup */
    "fixedpointConsts.xml",
    "fixedpointTK.xml",
    "floatpointConsts.xml",
    "floatpointTK.xml",
    "sseConsts.xml",
    "sseTK.xml"
};

extern char *tk_tmplt_path;


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
            if(0 == strcmp(cNodeName, "Immf")){
                val_node.type = CONST_FLOAT;
                val_node.float32 = strtof((const char *)cNode->children->content, NULL);
                val_node.float64 = strtod((const char *)cNode->children->content, NULL);
                val_node.float80 = strtold((const char *)cNode->children->content, NULL);
                g_array_append_val(cg_tree_node->const_nodes, val_node);
            }else if(0 == strcmp(cNodeName, "Imm64")){
                val_node.type = CONST_IMM64;
                val_node.imm64 = hex2declong((const char *)cNode->children->content);
                g_array_append_val(cg_tree_node->const_nodes, val_node);    
            }else if(0 == strcmp(cNodeName, "Bcd")){
                val_node.type = CONST_BCD;
                str2bcd((const char *)cNode->children->content, val_node.bcd);
                g_array_append_val(cg_tree_node->const_nodes, val_node);
            }else if(0 == strcmp(cNodeName, "X87status")){
                val_node.type = CONST_X87_STATUS;
                uint32_t x87status[7] = {0};
                str2x87_status((const char *)cNode->children->content, x87status);
                memcpy(&(val_node.fcw), x87status, 28);
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

static struct wd_node *parseOpndTK(xmlNodePtr tkNode)
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

static void parseInstTK(xmlNodePtr tkNode, GArray *tk_trees)
{
    char *opnd;
    struct wd_root *tk_tree;

    for (xmlNodePtr opndNode = tkNode->children; opndNode != NULL; opndNode = opndNode->next) {
        if (opndNode->type != XML_ELEMENT_NODE)
            continue;
        
        opnd = (char *)xmlGetProp(opndNode, (const unsigned char*)"opnd");
        if (opnd == NULL)
            goto simple_opnd_tk;

        tk_tree = wdtree_create();
        tk_tree->wd_node = parseOpndTK(opndNode);
        g_array_append_val(tk_trees, tk_tree);
    }
    return;

simple_opnd_tk:
    tk_tree = wdtree_create();
    tk_tree->wd_node = parseOpndTK(tkNode);
    g_array_append_val(tk_trees, tk_tree);
}

static void parseTKs(xmlNodePtr tksNode)
{
    char *key;
    struct hash_insert hi;
    struct tk_model *tkm;

    for (xmlNodePtr tkNode = tksNode->children; tkNode != NULL; tkNode = tkNode->next) {
        if (tkNode->type != XML_ELEMENT_NODE)
            continue;

        tkm = tkmodel_create();

        parseInstTK(tkNode, tkm->tk_trees);

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
        sprintf(fname, "%s/%s", tk_tmplt_path, xmlfiles[i]);
        parse_tks_file(fname);
        memset(fname, 0, 1024);
    }
#else
    nasm_fatal("XInclude support not compiled in\n");
#endif
}

