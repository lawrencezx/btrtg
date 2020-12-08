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

static const char fout_head[] = "section .text\n  global _start\n_start:\n";
static const char fout_tail[] = "\n  mov eax,1\n  mov ebx,0\n  int 80h";

bool gen_test_file_MOV(void)
{
    struct output_data data;
    ofmt->init("test_MOV.s");
    dfmt->init("debug_MOV.txt");
    
    generator_init(false);

    data.type = OUTPUT_RAWDATA;
    data.buf = (const void *)fout_head;
    ofmt->output(&data);

//    assign_arr5(MOV_seed.opd, REG_GPR|BITS16,REG_SREG,0,0,0);
    assign_arr5(MOV_seed.opd, REG_GPR|BITS32,MEMORY,0,0,0);

    gsp(&MOV_seed, ofmt);

    data.type = OUTPUT_RAWDATA;
    data.buf = (const void *)fout_tail;
    ofmt->output(&data);
    
    ofmt->cleanup();
    dfmt->cleanup();

    generator_exit();
    return true;
}
