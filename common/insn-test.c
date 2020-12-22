#include "compiler.h"

#include "error.h"
#include "seed.h"
#include "generator.h"
#include "insnlist.h"
#include "ofmt.h"
#include "opflags.h"
#include "insn-test.h"
#include "x86pg.h"

static const char fout_head[] = "  GLOBAL _start\n_start:\n  lea edx,data0\n";
static const char fout_tail[] = "\n  mov eax,1\n  mov ebx,0\n  int 80h";

void gsp_init(void)
{
    struct output_data data;
    data.type = OUTPUT_SECTION;
    data.buf = (const void *)&X86PGState.data_sec;
    ofmt->output(&data);

    data.type = OUTPUT_SECTION;
    data.buf = (const void *)&X86PGState.text_sec;
    ofmt->output(&data);

    data.type = OUTPUT_RAWDATA;
    data.buf = (const void *)fout_head;
    ofmt->output(&data);

    reset_x86pgstate();
}

void gsp_finish(void)
{
    end_insn_gen();

    insnlist_output(X86PGState.instlist, ofmt);
    insnlist_clear(X86PGState.instlist);

    struct output_data data;
    data.type = OUTPUT_RAWDATA;
    data.buf = (const void *)fout_tail;
    ofmt->output(&data);
}

void gsp(const insn_seed *seed)
{
    insn new_inst;

    for (int i = 0; i < 10; i++) {
        one_insn_gen(seed, &new_inst);
    }
}
