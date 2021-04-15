#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "parseLib.h"
#include "parseXML.h"
#include "error.h"
#include "tk.h"

static const char *xmlfiles[10] =
{
    /* must put Consts before TK, so the TK can find relevant constGroup */
    "fixedpointConsts.xml",
    "fixedpointTK.xml",
    "floatpointConsts.xml",
    "floatpointTK.xml",
    "mmxConsts.xml",
    "mmxTK.xml",
    "sseConsts.xml",
    "sseTK.xml",
    "sse2Consts.xml",
    "sse2TK.xml",
    //"sse2-mmxTK.xml",
    //"sse2-sseTK.xml"
};


extern char *tk_tmplt_path;


static void parseCGs(xmlNodePtr cgsNode)
{
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

            const char *cNodeName = (const char *)cNode->name;
            const char *cNodeContent = (const char *)cNode->children->content;

            if(strcmp(cNodeName, "Immf") == 0){
                val_node.type = CONST_FLOAT;
                val_node.float32 = strtof((const char *)cNode->children->content, NULL);
                val_node.float64 = strtod((const char *)cNode->children->content, NULL);
                long double float80 = strtold((const char *)cNode->children->content, NULL);
                memcpy(&val_node.float80, &float80, 10);
            }else if(strcmp(cNodeName, "Imm64") == 0){
                val_node.type = CONST_IMM64;
                val_node.imm64 = hex2declong(cNodeContent);
            } else if(strcmp(cNodeName, "Mmx") == 0){
                val_node.imm64 = hex2declong(cNodeContent);
            }else if(strcmp(cNodeName, "Bcd") == 0){
                val_node.type = CONST_BCD;
                str2bcd(cNodeContent, val_node.bcd);
            } else if (strcmp(cNodeName, "Float32i") == 0) {
                val_node.type = CONST_FLOAT32I;
                val_node.imm32 = (uint32_t)hex2dec(cNodeContent);
            } else if (strcmp(cNodeName, "Float32f") == 0) {
                val_node.type = CONST_FLOAT32F;
                val_node.float32 = strtof(cNodeContent, NULL);
            } else if (strcmp(cNodeName, "Float64i") == 0) {
                val_node.type = CONST_FLOAT64I;
                val_node.imm64 = (uint32_t)hex2dec(cNodeContent);
            } else if (strcmp(cNodeName, "Float64f") == 0) {
                val_node.type = CONST_FLOAT64F;
                val_node.float64 = strtod(cNodeContent, NULL);
            } else if (strcmp(cNodeName, "X87env") == 0){
                val_node.type = CONST_X87ENV;
                uint32_t x87env[7] = {0};
                str2x87env((const char *)cNode->children->content, x87env);
                memcpy(&(val_node.fcw), x87env, 28);
            } else {
                val_node.type = CONST_IMM32;
                val_node.imm32 = (uint32_t)hex2dec(cNodeContent);
            }
            g_array_append_val(cg_tree_node->const_nodes, val_node);
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
    char *prop_opnd, *prop_packedn;
    struct wd_root *tk_tree;

    for (xmlNodePtr opndNode = tkNode->children; opndNode != NULL; opndNode = opndNode->next) {
        if (opndNode->type != XML_ELEMENT_NODE)
            continue;
        
        prop_packedn = (char *)xmlGetProp(opndNode, (const unsigned char*)"packedn");
        prop_opnd = (char *)xmlGetProp(opndNode, (const unsigned char*)"opnd");
        if (prop_opnd == NULL)
            goto simple_opnd_tk;

        tk_tree = wdtree_create();
        tk_tree->packedn = (prop_packedn == NULL) ? 1 : atoi(prop_packedn);
        tk_tree->wd_node = parseOpndTK(opndNode);
        g_array_append_val(tk_trees, tk_tree);

        free(prop_opnd);
        free(prop_packedn);
    }
    return;

simple_opnd_tk:
    tk_tree = wdtree_create();
    tk_tree->packedn = (prop_packedn == NULL) ? 1 : atoi(prop_packedn);
    tk_tree->wd_node = parseOpndTK(tkNode);
    g_array_append_val(tk_trees, tk_tree);

    free(prop_packedn);
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

