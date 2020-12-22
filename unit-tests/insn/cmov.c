#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

static insn_seed CMOV_seed =
{
    .opcode = I_CMOV
};

bool gen_test_file_CMOV(void)
{
    ofmt->init("test_CMOV.s");
    dfmt->init("debug_CMOV.txt");
    
    gsp_init();

    assign_arr5(CMOV_seed.opd, MEMORY,REG_SREG,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS16,REG_SREG,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS32,REG_SREG,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_SREG,MEMORY,0,0,0);
    gsp(&CMOV_seed);

//    assign_arr5(CMOV_seed.opd, REG_SREG,REG_GPR|BITS16,0,0,0);
//    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_SREG,REG_GPR|BITS32,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_AL,MEM_OFFS,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_AX,MEM_OFFS,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_EAX,MEM_OFFS,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEM_OFFS,REG_AL,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEM_OFFS,REG_AX,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEM_OFFS,REG_EAX,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS32,REG_CREG,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_CREG,REG_GPR|BITS32,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEMORY,REG_GPR|BITS8,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS8,REG_GPR|BITS8,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEMORY,REG_GPR|BITS16,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS16,REG_GPR|BITS16,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEMORY,REG_GPR|BITS32,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS32,REG_GPR|BITS32,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS8,MEMORY,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS8,REG_GPR|BITS8,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS16,MEMORY,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS16,REG_GPR|BITS16,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS32,MEMORY,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS32,REG_GPR|BITS32,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS8,IMMEDIATE,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS16,IMMEDIATE,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, REG_GPR|BITS32,IMMEDIATE,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, RM_GPR|BITS8,IMMEDIATE,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, RM_GPR|BITS16,IMMEDIATE,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, RM_GPR|BITS32,IMMEDIATE,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEMORY,IMMEDIATE|BITS8,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEMORY,IMMEDIATE|BITS16,0,0,0);
    gsp(&CMOV_seed);

    assign_arr5(CMOV_seed.opd, MEMORY,IMMEDIATE|BITS32,0,0,0);
    gsp(&CMOV_seed);

    gsp_finish();
    
    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
