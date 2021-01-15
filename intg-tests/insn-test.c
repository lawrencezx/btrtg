#include "compiler.h"

#include "error.h"
#include "seed.h"
#include "generator.h"
#include "insnlist.h"
#include "ofmt.h"
#include "opflags.h"
#include "insn-test.h"
#include "x86pg.h"

static const char fout_head[] = "  GLOBAL main\nmain:\n  mov edx,0x0\n";
static const char fout_tail[] = "\n  call check_point_end\n  mov eax,1\n  mov ebx,0\n  int 80h";

static char *check_function_names[] =
{
#define DEFINE_CHECK_FUNCTION(name,type,func) #func,
#include "./compile/checkfunctions.h"
    "check_point_end"
#undef DEFINE_CHECK_FUNCTION
};

void gsp_init(void)
{
    struct output_data data;

    data.type = OUTPUT_EXTERN;
    for (size_t i = 0; i < ARRAY_SIZE(check_function_names); i++) {
        data.buf = (const void *)check_function_names[i];
        ofmt->output(&data);
    }

    data.type = OUTPUT_RAWDATA;
    data.buf = "";
    ofmt->output(&data);

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
