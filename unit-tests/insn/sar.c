#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

static insn_seed SAR_seed =
{
    .opcode = I_SAR
};

bool gen_test_file_SAR(void)
{
    ofmt->init("test_SAR.s");
    dfmt->init("debug_SAR.txt");
    
    gsp_init();

    assign_arr5(SAR_seed.opd, RM_GPR|BITS8,UNITY,0,0,0);
    gsp(&SAR_seed);

    assign_arr5(SAR_seed.opd, RM_GPR|BITS8,REG_CL,0,0,0);
    gsp(&SAR_seed);

    assign_arr5(SAR_seed.opd, RM_GPR|BITS8,IMMEDIATE|BITS8,0,0,0);
    gsp(&SAR_seed);

    assign_arr5(SAR_seed.opd, RM_GPR|BITS16,UNITY,0,0,0);
    gsp(&SAR_seed);

    assign_arr5(SAR_seed.opd, RM_GPR|BITS16,REG_CL,0,0,0);
    gsp(&SAR_seed);

    assign_arr5(SAR_seed.opd, RM_GPR|BITS16,IMMEDIATE|BITS8,0,0,0);
    gsp(&SAR_seed);

    assign_arr5(SAR_seed.opd, RM_GPR|BITS32,UNITY,0,0,0);
    gsp(&SAR_seed);

    assign_arr5(SAR_seed.opd, RM_GPR|BITS32,REG_CL,0,0,0);
    gsp(&SAR_seed);

    assign_arr5(SAR_seed.opd, RM_GPR|BITS32,IMMEDIATE|BITS8,0,0,0);
    gsp(&SAR_seed);

    gsp_finish();

    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
