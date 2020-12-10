#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

static insn_seed DIV_seed =
{
    .opcode = I_DIV
};

bool gen_test_file_DIV(void)
{
    ofmt->init("test_DIV.s");
    dfmt->init("debug_DIV.txt");
    
    gsp_init();

    assign_arr5(DIV_seed.opd, REG_GPR|BITS8,0,0,0,0);
    gsp(&DIV_seed, ofmt);

    assign_arr5(DIV_seed.opd, REG_GPR|BITS16,0,0,0,0);
    gsp(&DIV_seed, ofmt);

    assign_arr5(DIV_seed.opd, REG_GPR|BITS32,0,0,0,0);
    gsp(&DIV_seed, ofmt);

    gsp_finish();
    
    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
