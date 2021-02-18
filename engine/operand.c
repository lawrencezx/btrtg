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

bool create_unity(operand_seed *opnd_seed, char *buffer)
{
    dfmt->print("    try> create unity\n");
    int unity, shiftCount;
    
    if (opnd_seed->opdsize == BITS8) {
        shiftCount = 8;
    } else if (opnd_seed->opdsize == BITS16) {
        shiftCount = 16;
    } else if (opnd_seed->opdsize == BITS32) {
        shiftCount = 32;
    }

    unity = nasm_random32(shiftCount + 1);

    sprintf(buffer, "0x%x", unity);
    dfmt->print("    done> new unity: %s\n", buffer);
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

    switch (opnd_seed->opdsize) {
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
bool create_immediate(operand_seed* opnd_seed, char *buffer)
{
    dfmt->print("    try> create immediate\n");
    int imm;
    
    long long immn;
    switch (opnd_seed->opdsize) {
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

static void create_random_modrm(char *buffer)
{
    struct random_mem_addr mem_addr;
    enum reg_enum baseregs[6] = {R_EAX, R_ECX, R_EDX, R_EBX, R_ESI, R_EDI};
    enum reg_enum indexregs[6] = {R_EAX, R_ECX, R_EDX, R_EBX, R_ESI, R_EDI};
    enum reg_enum basereg, indexreg;

    /* data section */
    if (true) {
        do {
            basereg = baseregs[nasm_random32(6)];
        } while (stat_reg_locked(basereg));
        stat_lock_reg(basereg, LOCK_REG_CASE_MEM);
        do {
            indexreg = indexregs[nasm_random32(6)];
        } while (stat_reg_locked(indexreg));
        stat_unlock_reg(LOCK_REG_CASE_MEM);
        random_mem_addr_from_data(&mem_addr);

        int modes = nasm_random32(4);
        const char *base_reg_name = nasm_reg_names[basereg - EXPR_REG_START];
        const char *index_reg_name = nasm_reg_names[indexreg - EXPR_REG_START];

        /* initialize base register and index register */
        char *init_mem_addr = stat_get_init_mem_addr();
        sprintf(init_mem_addr, "  lea %s, data%d", base_reg_name, mem_addr.base);
        if (modes == 2 || modes == 3)
            sprintf(init_mem_addr + strlen(init_mem_addr), "\n  mov %s, 0x%x",
                    index_reg_name, mem_addr.index);

        switch (modes) {
            case 0:   /* base */
                sprintf(buffer, "[%s]", base_reg_name);
                break;
            case 1:   /* base + disp */
                sprintf(buffer, "[%s + 0x%x]", base_reg_name, mem_addr.disp);
                break;
            case 2:   /* base + index + disp */
                sprintf(buffer, "[%s + %s + 0x%x]", base_reg_name,
                    index_reg_name, mem_addr.disp);
                break;
            case 3:   /* base + index * scale + disp */
                sprintf(buffer, "[%s + %s*%d + 0x%x]", base_reg_name,
                    index_reg_name, mem_addr.scale, mem_addr.disp);
                break;
            default:
                break;
            /* disp */
            /* index * scale + disp */
        }
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
        create_random_modrm(modrm);
    }
    sprintf(buffer, "%s", modrm);
    dfmt->print("    done> new memory: %s\n", buffer);
    return true;
}

bool create_memoffs(operand_seed *opnd_seed, char *buffer)
{
    (void)opnd_seed;
    dfmt->print("    try> create memoffs\n");
    char src[128];
    int datai;
    if (globalbits == 16) {
        nasm_fatal("unsupported 16-bit memory type");
    } else {
        datai = nasm_random32(X86PGState.data_sec.datanum);
        sprintf(src, "[data%d]", datai);
    }
    sprintf(buffer, "%s", src);
    preappend_mem_size(buffer, opnd_seed->opdsize);
    dfmt->print("    done> new memoffs: %s\n", buffer);
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

    if((R_reg >= R_ST0) && (R_reg <= R_ST7)){
        char mem_address[64];
        char inst_init_mem_addr[128];
        strcpy(inst_init_mem_addr, stat_get_init_mem_addr());
        create_memory(NULL, mem_address);
        one_insn_gen_ctrl(stat_get_init_mem_addr(), INSERT_AFTER);

        stat_set_need_init(false);
        sprintf(buffer, "fxch %s", src);
        one_insn_gen_const(buffer);
        stat_set_need_init(true);

        stat_set_need_init(false);
        sprintf(buffer, "fstp st0");    
        //sprintf(asm_fpu_inst, "fincstp");
        one_insn_gen_const(buffer);
        stat_set_need_init(true);

        sprintf(buffer, "mov dword %s, 0x%x", mem_address, val_node->immf[0]);
        one_insn_gen_const(buffer); 

        sprintf(buffer, "fld dword %s",mem_address);
        one_insn_gen_const(buffer);
        
        stat_set_need_init(false);
        sprintf(buffer, "fxch %s", src);
        one_insn_gen_const(buffer);
        stat_set_need_init(true);
        sprintf(stat_get_init_mem_addr(), "%s", inst_init_mem_addr);

    }else{
        sprintf(buffer, "mov %s, 0x%x", src, (val_node == NULL) ?
        (uint32_t)nasm_random64(0x100000000) : val_node->imm32);
        one_insn_gen_const(buffer);
    }
    return true;
}

/* specify the fundamental data item size for a memory operand
 * for example: byte, word, dword, etc.
 */
char *preappend_mem_size(char *asm_mem, opflags_t opdsize)
{
    static const char *memsize[5] = {"byte ", "word ", "dword ", "qword", "tword"};
    int i = opdsize == BITS8 ? 0 :
            opdsize == BITS16 ? 1 : 
            opdsize == BITS32 ? 2 :
            opdsize == BITS64 ? 3 :
            opdsize == BITS80? 4 : 2;
    return nasm_strrplc(asm_mem, 0, memsize[i], strlen(memsize[i]));
}
