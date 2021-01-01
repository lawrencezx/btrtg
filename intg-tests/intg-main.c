#include "compiler.h"

#include "nasm.h"
#include "seed.h"
#include "generator.h"
#include "tmplt.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

int main(int argc, char *argv[])
{
    generator_init(false);

    ofmt->init("test_intg.s");
    dfmt->init("debug_intg.txt");

    gsp_init();
    
    walk_tmplt();

    gsp_finish();

    ofmt->cleanup();
    dfmt->cleanup();

    generator_exit();
    return 0;
}
