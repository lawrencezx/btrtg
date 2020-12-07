#include "compiler.h"

#include "error.h"
#include "seed.h"
#include "generator.h"
#include "insnlist.h"
#include "ofmt.h"
#include "opflags.h"
#include "insn-test.h"
#include "x86pg.h"

void gsp(const insn_seed *seed, const struct ofmt *ofmt)
{
    insn new_inst;

    for (int i = 0; i < 100; i++) {
        one_insn_gen(seed, &new_inst);
        insnlist_insert(X86PGState.instlist, &new_inst);
    }
    insnlist_output(X86PGState.instlist, ofmt);
    insnlist_clear(X86PGState.instlist);
}
