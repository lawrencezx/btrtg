#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

static insn_seed ADD_seed =
{
    .opcode = I_ADD
};

bool gen_test_file_ADD(void)
{
    ofmt->init("test_ADD.s");
    dfmt->init("debug_ADD.txt");
    
    gsp_init();

    assign_arr5(ADD_seed.opd, MEMORY,REG_GPR|BITS8,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS8,REG_GPR|BITS8,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, MEMORY,REG_GPR|BITS16,0,0,0); 
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS16,REG_GPR|BITS16,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, MEMORY,REG_GPR|BITS32,0,0,0); 
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS32,REG_GPR|BITS32,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS8,MEMORY,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS8,REG_GPR|BITS8,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS16,MEMORY,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS16,REG_GPR|BITS16,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS32,MEMORY,0,0,0); 
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_GPR|BITS32,REG_GPR|BITS32,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_AL,IMMEDIATE,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_AX,IMMEDIATE,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, REG_EAX,IMMEDIATE,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, RM_GPR|BITS8,IMMEDIATE,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, RM_GPR|BITS16,IMMEDIATE,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, RM_GPR|BITS32,IMMEDIATE,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, MEMORY,IMMEDIATE|BITS8,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, MEMORY,IMMEDIATE|BITS16,0,0,0);
    gsp(&ADD_seed);

    assign_arr5(ADD_seed.opd, MEMORY,IMMEDIATE|BITS32,0,0,0);
    gsp(&ADD_seed);

    gsp_finish();

    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
