#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

static insn_seed DEC_seed =
{
    .opcode = I_DEC
};

bool gen_test_file_DEC(void)
{
    ofmt->init("test_DEC.s");
    dfmt->init("debug_DEC.txt");
    
    gsp_init();

    assign_arr5(DEC_seed.opd, REG_GPR|BITS16,0,0,0,0);
    gsp(&DEC_seed);

    assign_arr5(DEC_seed.opd, REG_GPR|BITS32,0,0,0,0);
    gsp(&DEC_seed);

    assign_arr5(DEC_seed.opd, RM_GPR|BITS8,0,0,0,0);
    gsp(&DEC_seed);

    assign_arr5(DEC_seed.opd, RM_GPR|BITS16,0,0,0,0);
    gsp(&DEC_seed);

    assign_arr5(DEC_seed.opd, RM_GPR|BITS32,0,0,0,0);
    gsp(&DEC_seed);

    gsp_finish();

    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
