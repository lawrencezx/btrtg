#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "nasmlib.h"
#include "tk.h"
#include "buf2token.h"

struct hash_table hash_tks;

static int get_operands_from_instname(char * inst_name){
    struct tokenval tokval;
    char buffer[128];
    char * old_token_bufptr = get_token_bufptr();
    strcpy(buffer, inst_name);
    set_token_bufptr(buffer);
    get_token(&tokval);
    set_token_bufptr(old_token_bufptr);
    enum opcode opcode = tokval.t_integer;

    switch(opcode){
        case I_FADD:
        case I_FADDP:
        case I_FIADD:
        case I_FCMOVB:
        case I_FCMOVBE:
        case I_FCMOVE:
        case I_FCMOVNB:
        case I_FCMOVNBE:
        case I_FCMOVNE:
        case I_FCMOVNU:
        case I_FCMOVU:
        case I_FCOM:
        case I_FCOMP:
        case I_FCOMPP:
        case I_FCOMI:
        case I_FCOMIP:
        case I_FUCOMI:
        case I_FUCOMIP:
        case I_FDIV:
        case I_FDIVP:
        case I_FIDIV:
        case I_FDIVR:
        case I_FDIVRP:
        case I_FIDIVR:
        case I_FMUL:
        case I_FMULP:
        case I_FIMUL:
            return 2;
        case I_FABS:
        case I_FCHS:
        case I_FCOS:
        case I_FSIN:
        case I_FSINCOS:
        case I_FPTAN:
        case I_FPATAN:
            return 1;
        default:
            return -1;
    }
}
struct tk_model *tkmodel_create(void)
{
    struct tk_model *tkm;
    tkm = (struct tk_model *)nasm_malloc(sizeof(struct tk_model));
    tkm->tk_tree = NULL;
    return tkm;
}

void tks_free_all(void)
{
    hash_free_all(&hash_tks, true);
}

static struct tk_model *get_tkm_from_hashtbl(const char *asm_op)
{
    struct hash_insert hi;
    void **tkmpp;
    tkmpp = hash_find(&hash_tks, asm_op, &hi);
    return tkmpp == NULL ? NULL : *(struct tk_model **)tkmpp;
}

void create_trv_state(char *asm_inst, struct trv_state *trv_state)
{
    char inst_name[128];
    int i = 0;
    int operands = 0;
    while (asm_inst[i] != ' ' && asm_inst[i] != '\n' && asm_inst[i] != '\0') {
        inst_name[i] = asm_inst[i];
        i++;
    }
    inst_name[i] = '\0';
    struct tk_model *tkm = get_tkm_from_hashtbl(inst_name);
    g_array_append_val(trv_state->tk_trees, tkm->tk_tree);
    operands ++;
    while (asm_inst[i] != '\0' && asm_inst[i] != '\n')
        if (asm_inst[i++] == ','){
            g_array_append_val(trv_state->tk_trees, tkm->tk_tree);
            operands++;
        }
    for(int j = get_operands_from_instname(inst_name) - operands; j > 0; j--){
        g_array_append_val(trv_state->tk_trees, tkm->tk_tree);
    }
}

struct const_node *request_val_node(const char *asm_op, bool isDest)
{
    struct const_node *val_node;
    struct tk_model *tkm;
    tkm = get_tkm_from_hashtbl(asm_op);
    val_node = wdtree_select_leaf_node(tkm->tk_tree);
    return val_node;
}
