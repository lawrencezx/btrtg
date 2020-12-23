#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

static insn_seed PUSH_seed =
{
    .opcode = I_PUSH
};

bool gen_test_file_PUSH(void)
{
    ofmt->init("test_PUSH.s");
    dfmt->init("debug_PUSH.txt");
    
    gsp_init();

    assign_arr5(PUSH_seed.opd, REG_GPR|BITS16,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, REG_GPR|BITS32,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, RM_GPR|BITS16,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, RM_GPR|BITS32,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, REG_ES,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, REG_CS,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, REG_SS,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, REG_DS,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, REG_FS,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, REG_GS,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, IMMEDIATE|BITS8,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, IMMEDIATE|BITS16,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, IMMEDIATE|BITS32,0,0,0,0);
    gsp(&PUSH_seed);

    assign_arr5(PUSH_seed.opd, IMMEDIATE|BITS32,0,0,0,0);
    gsp(&PUSH_seed);

    gsp_finish();
    
    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
