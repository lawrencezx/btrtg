/* ----------------------------------------------------------------------- *
 *
 *   Copyright 1996-2020 The NASM Authors - All Rights Reserved
 *   See the file AUTHORS included with the NASM distribution for
 *   the specific copyright holders.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following
 *   conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----------------------------------------------------------------------- */

/*
 * The Netwide Assembler main program module
 */

#include "compiler.h"


#include "nasm.h"
#include "options.h"
#include "assemble.h"
#include "insns.h"
#include "nctype.h"
#include "generator.h"
#include "parser.h"
#include "gendata.h"
#include "disasm.h"

int globalrel = 0;
int globalbnd = 0;

extern FILE *error_file;               /* Where to write error messages */

static int cmd_sb = 32;    /* by default */

iflag_t cpu;
static iflag_t cmd_cpu;

struct location location;
bool in_absolute;                 /* Flag we are in ABSOLUTE seg */
struct location absolute;         /* Segment/offset inside ABSOLUTE */

char global_codebuf[MAX_INSLEN];
uint8_t global_codebuf_len;

int64_t switch_segment(int32_t segment)
{
    location.segment = segment;
    if (segment == NO_SEG) {
        location.offset = absolute.offset;
        in_absolute = true;
    } else {
        in_absolute = false;
    }
    return location.offset;
}

static void set_curr_offs(int64_t l_off)
{
        if (in_absolute)
            absolute.offset = l_off;
}

static void increment_offset(int64_t delta)
{
    if (unlikely(delta == 0))
        return;

    location.offset += delta;
    set_curr_offs(location.offset);
}

void generator_init(void)
{
  error_file = stderr;

  iflag_set_default_cpu(&cpu);
  iflag_set_default_cpu(&cmd_cpu);

  /* Save away the default state of warnings */
  init_warnings();

  gendata_init();

  nasm_ctype_init();
}

void generator_exit(void)
{
  src_free();
}

static void process_insn(insn *instruction)
{
    int64_t l;

    if (!instruction->times)
        return;                 /* Nothing to do... */

    nasm_assert(instruction->times > 0);

    l = assemble(location.segment, location.offset,
                 globalbits, instruction);
            /* We can't get an invalid instruction here */
    increment_offset(l);
}

uint32_t generate(insn_seed *seed, const char** buf)
{
    insn output_ins;
    switch (cmd_sb) {
    case 16:
        break;
    case 32:
        if (!iflag_cpu_level_ok(&cmd_cpu, IF_386))
            nasm_fatal("command line: 32-bit segment size requires a higher cpu");
        break;
    case 64:
        if (!iflag_cpu_level_ok(&cmd_cpu, IF_X86_64))
            nasm_fatal("command line: 64-bit segment size requires a higher cpu");
        break;
    default:
        panic();
        break;
    }

    global_codebuf_len = 0;

    globalbits = cmd_sb;  /* set 'bits' to command line default */
    cpu = cmd_cpu;

    in_absolute = false;
    location.segment = NO_SEG;
    location.offset  = 0;

    /* Not a directive, or even something that starts with [ */
    parse_insn_seed(seed, &output_ins);
    process_insn(&output_ins);

    reset_warnings();

    if (option_display_insn) {
        char outbuf[256];
        iflag_t prefer;
        disasm((uint8_t *)global_codebuf, (int32_t)global_codebuf_len, (char *)outbuf, sizeof(outbuf),
                globalbits, &prefer);
        printf("%s\n", outbuf);
    }

    *buf = (const char*)global_codebuf;
    return global_codebuf_len;
}
