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

/* TODO: lots of codes in the file should be adjusted later */

#include "compiler.h"


#include "nasm.h"
#include "insns.h"
#include "test.h"
#include "options.h"

const char *_progname;

static void parse_cmdline(int, char **);
static void usage(void);
static void help(FILE *);

static bool want_usage;

int main(int argc, char *argv[])
{
    _progname = argv[0];
    if (!_progname || !_progname[0])
        _progname = "nasm";

    want_usage = false;

    parse_cmdline(argc, argv);
    if (want_usage)
        usage();

    test_MOV();

    return 0;
}

static bool process_arg(char *p)
{
    if (!p || !p[0])
        return false;

    if (p[0] == '-') {
        switch (p[1]) {
        case 'h':
            help(stdout);
            exit(0);    /* never need usage message here */
            break;

        case 'd':
            set_display_insn(true);
            break;

        default:
            nasm_nonfatalf(ERR_USAGE, "unrecognised option `-%c'", p[1]);
            return false;
        }
    }

    return true;
}

static void parse_cmdline(int argc, char **argv)
{
    /*
     * Initialize all the warnings to their default state, including
     * warning index 0 used for "always on".
     */
    memcpy(warning_state, warning_default, sizeof warning_state);

    /*
     * Now process the actual command line.
     */
    want_usage = (argc > 2) ? true :
                 (argc == 1) ? false :
                 !process_arg(argv[1]); 
}

static void usage(void)
{
    fprintf(stderr, "Type %s -h for help.\n", _progname);
}

static void help(FILE *out)
{
    fprintf(out,
            "Usage: %s [-@ response_file] [options...] [--]\n"
            "       %s -v (or --v)\n",
            _progname, _progname);
    fputs(
        "\n"
        "Options (values in brackets indicate defaults):\n"
        "\n"
        "    -h            show this text and exit (also --help)\n"
        "\n"
        , out);
}
