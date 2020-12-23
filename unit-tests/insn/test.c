#include "compiler.h"

#include "seed.h"
#include "../ut.h"
#include "generator.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

bool gen_test_file_TEST(void)
{
    ofmt->init("test_TEST.s");
    dfmt->init("debug_TEST.txt");
    
    gsp_init();

    one_insn_gen_const("jne 0x1f\n");

    gsp_finish();
    
    ofmt->cleanup();
    dfmt->cleanup();
    return true;
}
