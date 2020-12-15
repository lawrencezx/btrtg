#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

static insn_seed MOV_seed =
{
    .opcode = I_MOV
};

bool gen_test_file_MOV(void)
{
    ofmt->init("test_MOV.s");
    dfmt->init("debug_MOV.txt");
    
    gsp_init();

    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,MEMORY,0,0,0);
    gsp(&MOV_seed);

    gsp_finish();
    
    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
