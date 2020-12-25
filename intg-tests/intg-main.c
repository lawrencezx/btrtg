#include "compiler.h"

#include "nasm.h"
#include "seed.h"
#include "generator.h"
#include "model.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"

int main(int argc, char *argv[])
{
    generator_init(false);

    ofmt->init("test_intg.s");
    dfmt->init("debug_intg.txt");

    gsp_init();
    
    for (int i = 0; i < tmpltm.instNum; i++) {
        insn_seed seed;
        insn inst;

        create_insn_seed(&seed, select_inst());
        one_insn_gen(&seed, &inst);
    }

    gsp_finish();

    ofmt->cleanup();
    dfmt->cleanup();

    generator_exit();
    return 0;
}
