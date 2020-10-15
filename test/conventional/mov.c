#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "generator.h"
#include "test.h"

static insn_seed MOV_seed =
{
    .opcode = I_MOV
};

void test_MOV(void)
{
    const char *buf;
    generator_init(false);

//    assign_arr5(MOV_seed.opd, MEMORY,REG_SREG,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS16,REG_SREG,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,REG_SREG,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,REG_SREG,0,0,0);
//    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, RM_GPR|BITS64,REG_SREG,0,0,0);
//    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_SREG,MEMORY,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_SREG,REG_GPR|BITS16,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_SREG,REG_GPR|BITS32,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_SREG,REG_GPR|BITS64,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_SREG,REG_GPR|BITS16,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_SREG,REG_GPR|BITS32,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_SREG,RM_GPR|BITS64,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_AL,MEM_OFFS,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_AX,MEM_OFFS,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_EAX,MEM_OFFS,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_RAX,MEM_OFFS,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, MEM_OFFS,REG_AL,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, MEM_OFFS,REG_AX,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, MEM_OFFS,REG_EAX,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, MEM_OFFS,REG_RAX,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,REG_CREG,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,REG_CREG,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_CREG,REG_GPR|BITS32,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_CREG,REG_GPR|BITS64,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,REG_DREG,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,REG_DREG,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_DREG,REG_GPR|BITS32,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_DREG,REG_GPR|BITS64,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,REG_TREG,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, REG_TREG,REG_GPR|BITS32,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, MEMORY,REG_GPR|BITS8,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS8,REG_GPR|BITS8,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, MEMORY,REG_GPR|BITS16,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS16,REG_GPR|BITS16,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, MEMORY,REG_GPR|BITS32,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,REG_GPR|BITS32,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, MEMORY,REG_GPR|BITS64,0,0,0);
//    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,REG_GPR|BITS64,0,0,0);
//    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS8,MEMORY,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS8,REG_GPR|BITS8,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS16,MEMORY,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS16,REG_GPR|BITS16,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,MEMORY,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,REG_GPR|BITS32,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,MEMORY,0,0,0);
//    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,REG_GPR|BITS64,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS8,IMMEDIATE,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS16,IMMEDIATE,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,IMMEDIATE,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,UDWORD,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,SDWORD,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, REG_GPR|BITS64,IMMEDIATE,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, RM_GPR|BITS8,IMMEDIATE,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, RM_GPR|BITS16,IMMEDIATE,0,0,0);
    generate_bin(&MOV_seed, &buf);

    assign_arr5(MOV_seed.opd, RM_GPR|BITS32,IMMEDIATE,0,0,0);
    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, RM_GPR|BITS64,IMMEDIATE,0,0,0);
//    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, RM_GPR|BITS64,IMMEDIATE|BITS32,0,0,0);
//    generate_bin(&MOV_seed, &buf);

//    assign_arr5(MOV_seed.opd, MEMORY,IMMEDIATE|BITS8,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, MEMORY,IMMEDIATE|BITS16,0,0,0);
//    generate_bin(&MOV_seed, &buf);
//
//    assign_arr5(MOV_seed.opd, MEMORY,IMMEDIATE|BITS32,0,0,0);
//    generate_bin(&MOV_seed, &buf);

    generator_exit();
}
