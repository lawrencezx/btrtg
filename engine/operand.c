#include "compiler.h"

#include "nasm.h"
#include "nasmlib.h"
#include "insns.h"
#include "error.h"
#include "seed.h"
#include "gendata.h"
#include "regdis.h"
#include "x86pg.h"
#include "operand.h"
#include "dfmt.h"
#include "tk.h"
#include "generator.h"
#include "mem.h"

bool create_specific_register(enum reg_enum R_reg, operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create specific register\n");
    if (is_class(REG_CLASS_SREG, opnd_seed->opndflags) && (opnd_seed->srcdestflags & OPDEST))
        return false;

    const char *src = nasm_reg_names[R_reg - EXPR_REG_START];
    sprintf(buffer, "%s", src);
    dfmt->print("    done> new specific register: %s\n", buffer);
    return true;
}

bool create_control_register(operand_seed *opnd_seed, char *buffer)
{
    return false;
    (void)opnd_seed;
    dfmt->print("    try> create creg\n");
    int cregi, cregn;
    enum reg_enum creg;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(REG_CREG);
    cregn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    cregi = nasm_random32(cregn);
    creg = nasm_rd_creg[cregi];
    src = nasm_reg_names[creg - EXPR_REG_START];
    sprintf(buffer, "%s", src);
    dfmt->print("    done> new creg: %s\n", buffer);
    return true;
}

bool create_segment_register(operand_seed *opnd_seed, char *buffer)
{
    (void)opnd_seed;
    dfmt->print("    try> create sreg\n");
    int sregi, sregn;
    enum reg_enum sreg;
    const char *src;
    if (opnd_seed->srcdestflags & OPDEST) {
        return false;
    }

    bseqiflags_t bseqiflags = bseqi_flags(REG_SREG);
    sregn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    sregi = nasm_random32(sregn);
    sreg = nasm_rd_sreg[sregi];
    src = nasm_reg_names[sreg - EXPR_REG_START];
    sprintf(buffer, "%s", src);
    dfmt->print("    done> new sreg: %s\n", buffer);
    return true;
}

bool create_fpu_register(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create fpureg\n");
    int fpuregi, fpuregn;
    enum reg_enum fpureg;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(opnd_seed->opndflags);

    fpuregn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    fpuregi = nasm_random32(fpuregn);
    fpureg = nasm_rd_fpureg[fpuregi];
    src = nasm_reg_names[fpureg - EXPR_REG_START];

    
    sprintf(buffer, " %s", src);
    dfmt->print("    done> new fpureg: %s\n", buffer);
    
    
    return true;
}

bool create_mmx_register(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create mmxreg\n");
    int mmxregi, mmxregn;
    enum reg_enum mmxreg;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(opnd_seed->opndflags);
    mmxregn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    mmxregi = nasm_random32(mmxregn);
    mmxreg = nasm_rd_mmxreg[mmxregi];
    src = nasm_reg_names[mmxreg - EXPR_REG_START];
    sprintf(buffer, " %s", src);
    dfmt->print("    done> new mmxreg: %s\n", buffer);
    return true;
}

bool create_xmm_register(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create xmmreg\n");
    int xmmregi, xmmregn;
    enum reg_enum xmmreg;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(opnd_seed->opndflags);
    xmmregn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    xmmregi = nasm_random32(xmmregn);
    xmmreg = nasm_rd_xmmreg[xmmregi];
    src = nasm_reg_names[xmmreg - EXPR_REG_START];
    sprintf(buffer, " %s", src);
    dfmt->print("    done> new xmmreg: %s\n", buffer);
    return true;
}

bool create_unity(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create unity\n");
    int unity, shiftCount;
    opflags_t opndsize = size_mask(opnd_seed->opndflags);
    
    if (opndsize == BITS8) {
        shiftCount = 8;
    } else if (opndsize == BITS16) {
        shiftCount = 16;
    } else if (opndsize == BITS32) {
        shiftCount = 32;
    }

    unity = nasm_random32(shiftCount + 1);

    sprintf(buffer, "0x%x", unity);
    dfmt->print("    done> new unity: %s\n", buffer);
    return true;
}

bool create_sbyteword(operand_seed *opnd_seed, char *buffer)
{
    (void)opnd_seed;
    dfmt->print("    try> create sbyteword\n");
    
    int sbyteword;

    sbyteword = nasm_random32(RAND_BITS16_BND);

    sprintf(buffer, "0x%x", sbyteword);
    dfmt->print("    done> new sbyteword: %s\n", buffer);
    return true;
}

bool create_sbytedword(operand_seed *opnd_seed, char *buffer)
{
    (void)opnd_seed;
    dfmt->print("    try> create sbytedword\n");
    
    int sbytedword;

    sbytedword = nasm_random64(RAND_BITS32_BND);

    sprintf(buffer, "0x%x", sbytedword);
    dfmt->print("    done> new sbytedword: %s\n", buffer);
    return true;
}

bool create_gpr_register(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create gpr\n");
    int gpri, gprn;
    enum reg_enum gpr;
    const char *src;

    bseqiflags_t bseqiflags = bseqi_flags(opnd_seed->opndflags);

gen_gpr:
    gprn = BSEQIFLAG_INDEXSIZE(bseqiflags);
    gpri = nasm_random32(gprn);

    switch (size_mask(opnd_seed->opndflags)) {
        case BITS8:
            gpr = nasm_rd_reg8[gpri];
            break;
        case BITS16:
            gpr = nasm_rd_reg16[gpri];
            break;
        case BITS32:
            gpr = nasm_rd_reg32[gpri];
            break;
    }
    if (stat_reg_locked(gpr))
        goto gen_gpr;  

    src = nasm_reg_names[gpr - EXPR_REG_START];

    sprintf(buffer, "%s", src);
    dfmt->print("    done> new gpr: %s\n", buffer);
    return true;
}

/* Generate int type immediate.
 * If it's larger than the limmit (8/16-bits imm), the high significant bytes
 * will be wipped away while assembling.
 */
bool create_immediate(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create immediate\n");
    int imm;
    
    long long immn;
    switch (size_mask(opnd_seed->opndflags)) {
        case BITS8:
            immn = RAND_BITS8_BND;
            break;
        case BITS16:
            immn = RAND_BITS16_BND;
            break;
        case BITS32:
            immn = RAND_BITS32_BND;
            break;
        default:
            nasm_fatal("wrong immediate size");
            break;
    }
    imm = (int)nasm_random64(immn);

    sprintf(buffer, "0x%x", imm);
    dfmt->print("    done> new immediate: %s\n", buffer);
    return true;
}

static void create_random_modrm(operand_seed *opnd_seed, char *buffer)
{
    struct random_mem_addr mem_addr;
    enum reg_enum baseregs[6] = {R_EAX, R_ECX, R_EDX, R_EBX, R_ESI, R_EDI};
    enum reg_enum indexregs[6] = {R_EAX, R_ECX, R_EDX, R_EBX, R_ESI, R_EDI};
    enum reg_enum basereg, indexreg;

    /* data section */
    if (true) {
        int mode;
        char *init_mem_addr;
        const char *base_reg_name, *index_reg_name;

        if (is_class(MEM_OFFS, opnd_seed->opndflags))
            mode = SIB_MODE_D;
        else
            mode = nasm_random32(SIB_MODE_NUM);

        random_mem_addr_from_data(&mem_addr, mode);
        /* initialize base register and index register */
        init_mem_addr = stat_get_init_mem_addr();

        *init_mem_addr = '\0';

        if (sib_mode_has_base(mode)) { /* if has base */
            do {
                basereg = baseregs[nasm_random32(6)];
            } while (stat_reg_locked(basereg));
            stat_lock_reg(basereg, LOCK_REG_CASE_MEM);
            base_reg_name = nasm_reg_names[basereg - EXPR_REG_START];
            sprintf(init_mem_addr, "  lea %s, data%d", base_reg_name, mem_addr.base);
        }

        if (sib_mode_has_index(mode)) { /* if has index */
            do {
                indexreg = indexregs[nasm_random32(6)];
            } while (stat_reg_locked(indexreg));
            index_reg_name = nasm_reg_names[indexreg - EXPR_REG_START];
            sprintf(init_mem_addr + strlen(init_mem_addr), "%s  mov %s, 0x%x",
                    sib_mode_has_base(mode) ? "\n" : "", index_reg_name,
                    mem_addr.index);
        }
        stat_unlock_reg(LOCK_REG_CASE_MEM);

        switch (mode) {
            case SIB_MODE_D:   /* disp */
                sprintf(buffer, "[data%d]", mem_addr.base);
                opnd_seed->has_label = true;
                break;
            case SIB_MODE_B:   /* base */
                sprintf(buffer, "[%s]", base_reg_name);
                break;
            case SIB_MODE_BD:   /* base + disp */
                sprintf(buffer, "[%s + 0x%x]", base_reg_name, mem_addr.disp);
                break;
            case SIB_MODE_ISD:   /* index * scale + disp */
                sprintf(buffer, "[%s*%d + data%d]", index_reg_name,
                    mem_addr.scale, mem_addr.base);
                opnd_seed->has_label = true;
                break;
            case SIB_MODE_BID:   /* base + index + disp */
                sprintf(buffer, "[%s + %s + 0x%x]", base_reg_name,
                    index_reg_name, mem_addr.disp);
                break;
            case SIB_MODE_BISD:   /* base + index * scale + disp */
                sprintf(buffer, "[%s + %s*%d + 0x%x]", base_reg_name,
                    index_reg_name, mem_addr.scale, mem_addr.disp);
                break;
            default:
                break;
        }

        if (mode != SIB_MODE_D)
            stat_set_has_mem_opnd(true);
    }
    /* else stack */
}

bool create_memory(operand_seed *opnd_seed, char *buffer)
{
    (void)opnd_seed;
    dfmt->print("    try> create memory\n");
    char modrm[64];

    if (globalbits == 16) {
        nasm_fatal("unsupported 16-bit memory type");
    } else {
        create_random_modrm(opnd_seed, modrm);
    }
    sprintf(buffer, "%s", modrm);
    dfmt->print("    done> new memory: %s\n", buffer);
    return true;
}

bool init_specific_register(enum reg_enum R_reg)
{
    char buffer[128];
    const char *src;
    src = nasm_reg_names[R_reg - EXPR_REG_START];
    struct const_node *val_node;
    GArray *val_nodes = stat_get_val_nodes();
    if (val_nodes == NULL) {
        const char *asm_op = nasm_insn_names[stat_get_opcode()];
        val_node = request_val_node(asm_op, stat_get_opi());
    } else {
        val_node = g_array_index(val_nodes, struct const_node *, stat_get_opi());
    }

    bool has_mem_opnd = stat_get_has_mem_opnd();
    stat_set_has_mem_opnd(false);

    if((R_reg >= R_ST0) && (R_reg <= R_ST7)){
        char mem_address[64] = "[data0]";
        char *mem_address_end = mem_address + strlen(mem_address);
        sprintf(buffer, "fxch %s", src);
        one_insn_gen_const(buffer);

        sprintf(buffer, "fstp st0");    
        //sprintf(asm_fpu_inst, "fincstp");
        one_insn_gen_const(buffer);

        sprintf(buffer, "  mov dword %s, 0x%x", mem_address, val_node->immf[1]);
        one_insn_gen_ctrl(buffer, INSERT_AFTER); 

        sprintf(mem_address_end - 1, " + 0x4]");

        sprintf(buffer, "  mov dword %s, 0x%x", mem_address, val_node->immf[2]);
        one_insn_gen_ctrl(buffer, INSERT_AFTER); 

        sprintf(mem_address_end - 1 , "]");
        sprintf(buffer, "  fld qword %s",mem_address);
        one_insn_gen_ctrl(  buffer, INSERT_AFTER);

        sprintf(buffer, "fxch %s", src);
        one_insn_gen_const(buffer);
    }else{
        sprintf(buffer, "mov %s, 0x%x", src, (val_node == NULL) ?
        (uint32_t)nasm_random64(0x100000000) : val_node->imm32);
        one_insn_gen_const(buffer);
    }
    stat_set_has_mem_opnd(has_mem_opnd);
    return true;
}

bool init_specific_mem(enum reg_enum R_reg)
{
    char buffer[128];
    if (R_reg == R_ESI)
        sprintf(buffer, "  lea esi,data1");
    else if (R_reg == R_EDI)
        sprintf(buffer, "  lea edi,data2");
    else
        nasm_fatal("Unsupported specific memory register");
    one_insn_gen_ctrl(buffer, INSERT_AFTER);
    return true;
}

bool init_popf(void)
{
    char buffer[128];
    struct const_node *val_node;
    GArray *val_nodes = stat_get_val_nodes();
    if (val_nodes == NULL) {
        const char *asm_op = nasm_insn_names[stat_get_opcode()];
        val_node = request_val_node(asm_op, stat_get_opi());
    } else {
        val_node = g_array_index(val_nodes, struct const_node *, stat_get_opi());
    }
    sprintf(buffer, "  push 0x%x", val_node->imm32);
    one_insn_gen_ctrl(buffer, INSERT_AFTER);
    return true;
}

/* specify the fundamental data item size for a memory operand
 * for example: byte, word, dword, etc.
 */
char *preappend_mem_size(char *asm_mem, opflags_t opndsize)
{
    static const char *memsize[5] = {"byte ", "word ", "dword ", "qword", "tword"};
    int i = opndsize == BITS8 ? 0 :
            opndsize == BITS16 ? 1 : 
            opndsize == BITS32 ? 2 :
            opndsize == BITS64 ? 3 :
            opndsize == BITS80? 4 : 2;
    return nasm_strrplc(asm_mem, 0, memsize[i], strlen(memsize[i]));
}
