#include "compiler.h"

#include "nasm.h"
#include "insns.h"
#include "seed.h"
#include "x86pg.h"

struct X86PGState X86PGState;

void init_x86pgstate(void)
{
    X86PGState.seqMode = false;
    bseqi_init(&X86PGState.bseqi);
    X86PGState.simpleDataMemMode = true;
    init_text_sec(&X86PGState.text_sec);
    init_data_sec(&X86PGState.data_sec);
    X86PGState.curr_seed = NULL;
    X86PGState.curr_inst = NULL;
    X86PGState.instlist = insnlist_create();
}
