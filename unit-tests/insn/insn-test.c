#include "compiler.h"

#include "error.h"
#include "seed.h"
#include "generator.h"
#include "insnlist.h"
#include "ofmt.h"
#include "opflags.h"
#include "insn-test.h"
#include "x86pg.h"

static const char data_content[] = "  data:  times 256 db 0\n";
static const char fout_head[] = "  GLOBAL _start\n_start:\n";
static const char fout_tail[] = "\n  mov eax,1\n  mov ebx,0\n  int 80h";

void gsp_init(void)
{
    struct output_data data;
    data.type = OUTPUT_SECTION;
    data.buf = (const void *)&X86PGState.data_sec;
    ofmt->output(&data);

    data.type = OUTPUT_RAWDATA;
    data.buf = (const void *)data_content;
    ofmt->output(&data);

    data.type = OUTPUT_SECTION;
    data.buf = (const void *)&X86PGState.text_sec;
    ofmt->output(&data);

    data.type = OUTPUT_RAWDATA;
    data.buf = (const void *)fout_head;
    ofmt->output(&data);
}

void gsp_finish(void)
{
    struct output_data data;
    data.type = OUTPUT_RAWDATA;
    data.buf = (const void *)fout_tail;
    ofmt->output(&data);
}

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
