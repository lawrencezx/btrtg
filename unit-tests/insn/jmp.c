#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

static insn_seed JMP_seed =
{
    .opcode = I_JMP
};

bool gen_test_file_JMP(void)
{
    ofmt->init("test_JMP.s");
    dfmt->init("debug_JMP.txt");
    
    gsp_init();

    assign_arr5(JMP_seed.opd, IMMEDIATE,0,0,0,0);
    gsp(&JMP_seed);

    gsp_finish();
    
    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
