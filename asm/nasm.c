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
#include "nasmlib.h"
#include "nctype.h"
#include "error.h"
#include "floats.h"
#include "stdscan.h"
#include "insns.h"
#include "preproc.h"
#include "parser.h"
#include "eval.h"
#include "assemble.h"
#include "labels.h"
#include "outform.h"
#include "listing.h"
#include "iflag.h"
#include "quote.h"
#include "ver.h"

/*
 * This is the maximum number of optimization passes to do.  If we ever
 * find a case where the optimizer doesn't naturally converge, we might
 * have to drop this value so the assembler doesn't appear to just hang.
 */
#define MAX_OPTIMIZE (INT_MAX >> 1)

struct forwrefinfo {            /* info held on forward refs. */
    int lineno;
    int operand;
};

const char *_progname;

static void parse_cmdline(int, char **, int);
static void assemble_file(void);
static bool skip_this_pass(errflags severity);
static void usage(void);
static void help(FILE *);

struct error_format {
    const char *beforeline;     /* Before line number, if present */
    const char *afterline;      /* After line number, if present */
    const char *beforemsg;      /* Before actual message */
};

static const struct error_format errfmt_gnu  = { ":", "",  ": "  };
static const struct error_format errfmt_msvc = { "(", ")", " : " };
static const struct error_format *errfmt = &errfmt_gnu;
static struct strlist *warn_list;
static struct nasm_errhold *errhold_stack;

unsigned int debug_nasm;        /* Debugging messages? */

static bool using_debug_info, opt_verbose_info;
static const char *debug_format;

#ifndef ABORT_ON_PANIC
# define ABORT_ON_PANIC 0
#endif
static bool abort_on_panic = ABORT_ON_PANIC;
static bool keep_all;

bool tasm_compatible_mode = false;
int globalrel = 0;
int globalbnd = 0;

const char *inname;
const char *outname;
static const char *listname;
static const char *errname;

const struct ofmt_alias *ofmt_alias = NULL;
const struct dfmt *dfmt;

FILE *error_file;               /* Where to write error messages */

FILE *ofile = NULL;
struct optimization optimizing =
    { MAX_OPTIMIZE, OPTIM_ALL_ENABLED }; /* number of optimization passes to take */
static int cmd_sb = 32;    /* by default */

iflag_t cpu;
static iflag_t cmd_cpu;

struct location location;
bool in_absolute;                 /* Flag we are in ABSOLUTE seg */
struct location absolute;         /* Segment/offset inside ABSOLUTE */

static enum preproc_opt ppopt;

#define OP_NORMAL           (1U << 0)
#define OP_PREPROCESS       (1U << 1)
#define OP_DEPEND           (1U << 2)

/* Dependency flags */
static bool depend_emit_phony = false;
static const char *depend_target = NULL;
static const char *depend_file = NULL;

static bool want_usage;
static bool terminate_after_phase;
bool user_nolist = false;

static char *quote_for_pmake(const char *str);
static char *quote_for_wmake(const char *str);
static char *(*quote_for_make)(const char *) = quote_for_pmake;

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

int main(int argc, char *argv[])
{
  error_file = stderr;
    _progname = argv[0];
  if (!_progname || !_progname[0])
      _progname = "nasm";

  iflag_set_default_cpu(&cpu);
  iflag_set_default_cpu(&cmd_cpu);

  parse_cmdline(argc, argv, 1);
  if (terminate_after_phase) {
      if (want_usage)
          usage();
      return 1;
  }

  /* At this point we have ofmt and the name of the desired debug format */
  if (!using_debug_info) {
      /* No debug info, redirect to the null backend (empty stubs) */
      dfmt = &null_debug_form;
  }

  parse_cmdline(argc, argv, 2);
  if (terminate_after_phase) {
      if (want_usage)
          usage();
      return 1;
  }

  /* Save away the default state of warnings */
  init_warnings();

  dfmt->init();

  assemble_file();

  if (want_usage)
      usage();

  eval_cleanup();
  stdscan_cleanup();
  src_free();

  return terminate_after_phase;
}

/*
 * Get a parameter for a command line option.
 * First arg must be in the form of e.g. -f...
 */
static char *get_param(char *p, char *q, bool *advance)
{
    *advance = false;
    if (p[2]) /* the parameter's in the option */
        return nasm_skip_spaces(p + 2);
    if (q && q[0]) {
        *advance = true;
        return q;
    }
    nasm_nonfatalf(ERR_USAGE, "option `-%c' requires an argument", p[1]);
    return NULL;
}

/*
 * Copy a filename
 */
static void copy_filename(const char **dst, const char *src, const char *what)
{
    if (*dst)
        nasm_fatal("more than one %s file specified: %s\n", what, src);

    *dst = nasm_strdup(src);
}

/*
 * Convert a string to a POSIX make-safe form
 */
static char *quote_for_pmake(const char *str)
{
    const char *p;
    char *os, *q;

    size_t n = 1; /* Terminating zero */
    size_t nbs = 0;

    if (!str)
        return NULL;

    for (p = str; *p; p++) {
        switch (*p) {
        case ' ':
        case '\t':
            /* Convert N backslashes + ws -> 2N+1 backslashes + ws */
            n += nbs + 2;
            nbs = 0;
            break;
        case '$':
        case '#':
            nbs = 0;
            n += 2;
            break;
        case '\\':
            nbs++;
            n++;
            break;
        default:
            nbs = 0;
            n++;
            break;
        }
    }

    /* Convert N backslashes at the end of filename to 2N backslashes */
    if (nbs)
        n += nbs;

    os = q = nasm_malloc(n);

    nbs = 0;
    for (p = str; *p; p++) {
        switch (*p) {
        case ' ':
        case '\t':
            while (nbs--)
                *q++ = '\\';
            *q++ = '\\';
            *q++ = *p;
            break;
        case '$':
            *q++ = *p;
            *q++ = *p;
            nbs = 0;
            break;
        case '#':
            *q++ = '\\';
            *q++ = *p;
            nbs = 0;
            break;
        case '\\':
            *q++ = *p;
            nbs++;
            break;
        default:
            *q++ = *p;
            nbs = 0;
            break;
        }
    }
    while (nbs--)
        *q++ = '\\';

    *q = '\0';

    return os;
}

/*
 * Convert a string to a Watcom make-safe form
 */
static char *quote_for_wmake(const char *str)
{
    const char *p;
    char *os, *q;
    bool quote = false;

    size_t n = 1; /* Terminating zero */

    if (!str)
        return NULL;

    for (p = str; *p; p++) {
        switch (*p) {
        case ' ':
        case '\t':
        case '&':
            quote = true;
            n++;
            break;
        case '\"':
            quote = true;
            n += 2;
            break;
        case '$':
        case '#':
            n += 2;
            break;
        default:
            n++;
            break;
        }
    }

    if (quote)
        n += 2;

    os = q = nasm_malloc(n);

    if (quote)
        *q++ = '\"';

    for (p = str; *p; p++) {
        switch (*p) {
        case '$':
        case '#':
            *q++ = '$';
            *q++ = *p;
            break;
        case '\"':
            *q++ = *p;
            *q++ = *p;
            break;
        default:
            *q++ = *p;
            break;
        }
    }

    if (quote)
        *q++ = '\"';

    *q = '\0';

    return os;
}

enum text_options {
    OPT_BOGUS,
    OPT_VERSION,
    OPT_HELP,
    OPT_ABORT_ON_PANIC,
    OPT_MANGLE,
    OPT_INCLUDE,
    OPT_PRAGMA,
    OPT_BEFORE,
    OPT_KEEP_ALL,
    OPT_NO_LINE,
    OPT_DEBUG,
    OPT_REPRODUCIBLE
};
enum need_arg {
    ARG_NO,
    ARG_YES,
    ARG_MAYBE
};

struct textargs {
    const char *label;
    enum text_options opt;
    enum need_arg need_arg;
    int pvt;
};
static const struct textargs textopts[] = {
    {"v", OPT_VERSION, ARG_NO, 0},
    {"version", OPT_VERSION, ARG_NO, 0},
    {"help",     OPT_HELP,  ARG_NO, 0},
    {"abort-on-panic", OPT_ABORT_ON_PANIC, ARG_NO, 0},
    {"prefix",   OPT_MANGLE, ARG_YES, LM_GPREFIX},
    {"postfix",  OPT_MANGLE, ARG_YES, LM_GSUFFIX},
    {"gprefix",  OPT_MANGLE, ARG_YES, LM_GPREFIX},
    {"gpostfix", OPT_MANGLE, ARG_YES, LM_GSUFFIX},
    {"lprefix",  OPT_MANGLE, ARG_YES, LM_LPREFIX},
    {"lpostfix", OPT_MANGLE, ARG_YES, LM_LSUFFIX},
    {"include",  OPT_INCLUDE, ARG_YES, 0},
    {"pragma",   OPT_PRAGMA,  ARG_YES, 0},
    {"before",   OPT_BEFORE,  ARG_YES, 0},
    {"keep-all", OPT_KEEP_ALL, ARG_NO, 0},
    {"no-line",  OPT_NO_LINE, ARG_NO, 0},
    {"debug",    OPT_DEBUG, ARG_MAYBE, 0},
    {"reproducible", OPT_REPRODUCIBLE, ARG_NO, 0},
    {NULL, OPT_BOGUS, ARG_NO, 0}
};

static void show_version(void)
{
    printf("NASM version %s compiled on %s%s\n",
           nasm_version, nasm_date, nasm_compile_options);
    exit(0);
}

static bool stopoptions = false;
static bool process_arg(char *p, char *q, int pass)
{
    char *param;
    bool advance = false;

    if (!p || !p[0])
        return false;

    if (p[0] == '-' && !stopoptions) {
        if (strchr("oOfpPdDiIlLFXuUZwW", p[1])) {
            /* These parameters take values */
            if (!(param = get_param(p, q, &advance)))
                return advance;
        }

        switch (p[1]) {
        case 's':
            if (pass == 1)
                error_file = stdout;
            break;

        case 'o':       /* output file */
            if (pass == 2)
                copy_filename(&outname, param, "output");
            break;

        case 'O':       /* Optimization level */
            if (pass == 1) {
                int opt;

                if (!*param) {
                    /* Naked -O == -Ox */
                    optimizing.level = MAX_OPTIMIZE;
                } else {
                    while (*param) {
                        switch (*param) {
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7': case '8': case '9':
                            opt = strtoul(param, &param, 10);

                            /* -O0 -> optimizing.level == -1, 0.98 behaviour */
                            /* -O1 -> optimizing.level == 0, 0.98.09 behaviour */
                            if (opt < 2)
                                optimizing.level = opt - 1;
                            else
                                optimizing.level = opt;
                            break;

                        case 'v':
                        case '+':
                        param++;
                        opt_verbose_info = true;
                        break;

                        case 'x':
                            param++;
                            optimizing.level = MAX_OPTIMIZE;
                            break;

                        default:
                            nasm_fatal("unknown optimization option -O%c\n",
                                       *param);
                            break;
                        }
                    }
                    if (optimizing.level > MAX_OPTIMIZE)
                        optimizing.level = MAX_OPTIMIZE;
                }
            }
            break;

        case 'l':       /* listing file */
            if (pass == 2)
                copy_filename(&listname, param, "listing");
            break;

        case 'L':        /* listing options */
            if (pass == 2) {
                while (*param)
                    list_options |= list_option_mask(*param++);
            }
            break;

        case 'Z':       /* error messages file */
            if (pass == 1)
                copy_filename(&errname, param, "error");
            break;

        case 'F':       /* specify debug format */
            if (pass == 1) {
                using_debug_info = true;
                debug_format = param;
            }
            break;

        case 'X':       /* specify error reporting format */
            if (pass == 1) {
                if (!nasm_stricmp("vc", param) || !nasm_stricmp("msvc", param) || !nasm_stricmp("ms", param))
                    errfmt = &errfmt_msvc;
                else if (!nasm_stricmp("gnu", param) || !nasm_stricmp("gcc", param))
                    errfmt = &errfmt_gnu;
                else
                    nasm_fatalf(ERR_USAGE, "unrecognized error reporting format `%s'", param);
            }
            break;

        case 'g':
            if (pass == 1) {
                using_debug_info = true;
                if (p[2])
                    debug_format = nasm_skip_spaces(p + 2);
            }
            break;

        case 'h':
            help(stdout);
            exit(0);    /* never need usage message here */
            break;

        case 'y':
            /* legacy option */
            dfmt_list(stdout);
            exit(0);
            break;

        case 't':
            if (pass == 1)
                tasm_compatible_mode = true;
            break;

        case 'v':
            show_version();
            break;

        case 'a':       /* assemble only - don't preprocess */
            if (pass == 1)
                ppopt |= PP_TRIVIAL;
            break;

        case 'w':
        case 'W':
            if (pass == 2)
                set_warning_status(param);
        break;

        case 'M':
            if (pass == 1) {
                switch (p[2]) {
                case 'W':
                    quote_for_make = quote_for_wmake;
                    break;
                case 'D':
                case 'F':
                case 'T':
                case 'Q':
                    advance = true;
                    break;
                default:
                    break;
                }
            } else {
                switch (p[2]) {
                case 'P':
                    depend_emit_phony = true;
                    break;
                case 'F':
                    depend_file = q;
                    advance = true;
                    break;
                case 'T':
                    depend_target = q;
                    advance = true;
                    break;
                case 'Q':
                    depend_target = quote_for_make(q);
                    advance = true;
                    break;
                case 'W':
                    /* handled in pass 1 */
                    break;
                default:
                    nasm_nonfatalf(ERR_USAGE, "unknown dependency option `-M%c'", p[2]);
                    break;
                }
            }
            if (advance && (!q || !q[0])) {
                nasm_nonfatalf(ERR_USAGE, "option `-M%c' requires a parameter", p[2]);
                break;
            }
            break;

        case '-':
            {
                const struct textargs *tx;
                size_t olen, plen;
                char *eqsave;
                enum text_options opt;

                p += 2;

                if (!*p) {        /* -- => stop processing options */
                    stopoptions = true;
                    break;
                }

                olen = 0;       /* Placate gcc at lower optimization levels */
                plen = strlen(p);
                for (tx = textopts; tx->label; tx++) {
                    olen = strlen(tx->label);

                    if (olen > plen)
                        continue;

                    if (nasm_memicmp(p, tx->label, olen))
                        continue;

                    if (tx->label[olen-1] == '-')
                        break;  /* Incomplete option */

                    if (!p[olen] || p[olen] == '=')
                        break;  /* Complete option */
                }

                if (!tx->label) {
                    nasm_nonfatalf(ERR_USAGE, "unrecognized option `--%s'", p);
                }

                opt = tx->opt;

                eqsave = param = strchr(p+olen, '=');
                if (param)
                    *param++ = '\0';

                switch (tx->need_arg) {
                case ARG_YES:   /* Argument required, and may be standalone */
                    if (!param) {
                        param = q;
                        advance = true;
                    }

                    /* Note: a null string is a valid parameter */
                    if (!param) {
                        nasm_nonfatalf(ERR_USAGE, "option `--%s' requires an argument", p);
                        opt = OPT_BOGUS;
                    }
                    break;

                case ARG_NO:    /* Argument prohibited */
                    if (param) {
                        nasm_nonfatalf(ERR_USAGE, "option `--%s' does not take an argument", p);
                        opt = OPT_BOGUS;
                    }
                    break;

                case ARG_MAYBE: /* Argument permitted, but must be attached with = */
                    break;
                }

                switch (opt) {
                case OPT_BOGUS:
                    break;      /* We have already errored out */
                case OPT_VERSION:
                    show_version();
                    break;
                case OPT_ABORT_ON_PANIC:
                    abort_on_panic = true;
                    break;
                case OPT_MANGLE:
                    if (pass == 2)
                        set_label_mangle(tx->pvt, param);
                    break;
                case OPT_KEEP_ALL:
                    keep_all = true;
                    break;
                case OPT_NO_LINE:
                    ppopt |= PP_NOLINE;
                    break;
                case OPT_DEBUG:
                    debug_nasm = param ? strtoul(param, NULL, 10) : debug_nasm+1;
                    break;
                case OPT_REPRODUCIBLE:
                    reproducible = true;
                    break;
                case OPT_HELP:
                    help(stdout);
                    exit(0);
                default:
                    panic();
                }

                if (eqsave)
                    *eqsave = '='; /* Restore = argument separator */

                break;
            }

        default:
            nasm_nonfatalf(ERR_USAGE, "unrecognised option `-%c'", p[1]);
            break;
        }
    } else if (pass == 2) {
        /* In theory we could allow multiple input files... */
        copy_filename(&inname, p, "input");
    }

    return advance;
}

#define ARG_BUF_DELTA 128

static void process_respfile(FILE * rfile, int pass)
{
    char *buffer, *p, *q, *prevarg;
    int bufsize, prevargsize;

    bufsize = prevargsize = ARG_BUF_DELTA;
    buffer = nasm_malloc(ARG_BUF_DELTA);
    prevarg = nasm_malloc(ARG_BUF_DELTA);
    prevarg[0] = '\0';

    while (1) {                 /* Loop to handle all lines in file */
        p = buffer;
        while (1) {             /* Loop to handle long lines */
            q = fgets(p, bufsize - (p - buffer), rfile);
            if (!q)
                break;
            p += strlen(p);
            if (p > buffer && p[-1] == '\n')
                break;
            if (p - buffer > bufsize - 10) {
                int offset;
                offset = p - buffer;
                bufsize += ARG_BUF_DELTA;
                buffer = nasm_realloc(buffer, bufsize);
                p = buffer + offset;
            }
        }

        if (!q && p == buffer) {
            if (prevarg[0])
                process_arg(prevarg, NULL, pass);
            nasm_free(buffer);
            nasm_free(prevarg);
            return;
        }

        /*
         * Play safe: remove CRs, LFs and any spurious ^Zs, if any of
         * them are present at the end of the line.
         */
        *(p = &buffer[strcspn(buffer, "\r\n\032")]) = '\0';

        while (p > buffer && nasm_isspace(p[-1]))
            *--p = '\0';

        p = nasm_skip_spaces(buffer);

        if (process_arg(prevarg, p, pass))
            *p = '\0';

        if ((int) strlen(p) > prevargsize - 10) {
            prevargsize += ARG_BUF_DELTA;
            prevarg = nasm_realloc(prevarg, prevargsize);
        }
        strncpy(prevarg, p, prevargsize);
    }
}

/* Function to process args from a string of args, rather than the
 * argv array. Used by the environment variable and response file
 * processing.
 */
static void process_args(char *args, int pass)
{
    char *p, *q, *arg, *prevarg;
    char separator = ' ';

    p = args;
    if (*p && *p != '-')
        separator = *p++;
    arg = NULL;
    while (*p) {
        q = p;
        while (*p && *p != separator)
            p++;
        while (*p == separator)
            *p++ = '\0';
        prevarg = arg;
        arg = q;
        if (process_arg(prevarg, arg, pass))
            arg = NULL;
    }
    if (arg)
        process_arg(arg, NULL, pass);
}

static void process_response_file(const char *file, int pass)
{
    char str[2048];
    FILE *f = nasm_open_read(file, NF_TEXT);
    if (!f) {
        perror(file);
        exit(-1);
    }
    while (fgets(str, sizeof str, f)) {
        process_args(str, pass);
    }
    fclose(f);
}

static void parse_cmdline(int argc, char **argv, int pass)
{
    FILE *rfile;
    char *envreal, *envcopy = NULL, *p;

    /*
     * Initialize all the warnings to their default state, including
     * warning index 0 used for "always on".
     */
    memcpy(warning_state, warning_default, sizeof warning_state);

    /*
     * First, process the NASMENV environment variable.
     */
    envreal = getenv("NASMENV");
    if (envreal) {
        envcopy = nasm_strdup(envreal);
        process_args(envcopy, pass);
        nasm_free(envcopy);
    }

    /*
     * Now process the actual command line.
     */
    while (--argc) {
        bool advance;
        argv++;
        if (argv[0][0] == '@') {
            /*
             * We have a response file, so process this as a set of
             * arguments like the environment variable. This allows us
             * to have multiple arguments on a single line, which is
             * different to the -@resp file processing below for regular
             * NASM.
             */
            process_response_file(argv[0]+1, pass);
            argc--;
            argv++;
        }
        if (!stopoptions && argv[0][0] == '-' && argv[0][1] == '@') {
            p = get_param(argv[0], argc > 1 ? argv[1] : NULL, &advance);
            if (p) {
                rfile = nasm_open_read(p, NF_TEXT);
                if (rfile) {
                    process_respfile(rfile, pass);
                    fclose(rfile);
                } else {
                    nasm_nonfatalf(ERR_USAGE, "unable to open response file `%s'", p);
                }
            }
        } else
            advance = process_arg(argv[0], argc > 1 ? argv[1] : NULL, pass);
        argv += advance, argc -= advance;
    }

    /*
     * Look for basic command line typos. This definitely doesn't
     * catch all errors, but it might help cases of fumbled fingers.
     */
    if (pass != 2)
        return;

    if (!inname)
        nasm_fatalf(ERR_USAGE, "no input file specified");
    else if ((errname && !strcmp(inname, errname)) ||
             (outname && !strcmp(inname, outname)) ||
             (listname &&  !strcmp(inname, listname))  ||
             (depend_file && !strcmp(inname, depend_file)))
        nasm_fatalf(ERR_USAGE, "will not overwrite input file");

    if (errname) {
        error_file = nasm_open_write(errname, NF_TEXT);
        if (!error_file) {
            error_file = stderr;        /* Revert to default! */
            nasm_fatalf(ERR_USAGE, "cannot open file `%s' for error messages", errname);
        }
    }
}

static void process_insn(insn *instruction)
{
    int32_t n;
    int64_t l;

    if (!instruction->times)
        return;                 /* Nothing to do... */

    nasm_assert(instruction->times > 0);

    /*
     * NOTE: insn_size() can change instruction->times
     * (usually to 1) when called.
     */
    if (!pass_final()) {
        int64_t start = location.offset;
        for (n = 1; n <= instruction->times; n++) {
            l = insn_size(location.segment, location.offset,
                          globalbits, instruction);
            /* l == -1 -> invalid instruction */
            if (l != -1)
                increment_offset(l);
        }
        if (list_option('p')) {
            struct out_data dummy;
            memset(&dummy, 0, sizeof dummy);
            dummy.type   = OUT_RAWDATA; /* Handled specially with .data NULL */
            dummy.offset = start;
            dummy.size   = location.offset - start;
            lfmt->output(&dummy);
        }
    } else {
        l = assemble(location.segment, location.offset,
                     globalbits, instruction);
                /* We can't get an invalid instruction here */
        increment_offset(l);

        if (instruction->times > 1) {
            lfmt->uplevel(LIST_TIMES, instruction->times);
            for (n = 2; n <= instruction->times; n++) {
                l = assemble(location.segment, location.offset,
                             globalbits, instruction);
                increment_offset(l);
            }
            lfmt->downlevel(LIST_TIMES);
        }
    }
}

static void assemble_file()
{
    uint64_t prev_offset_changed;
    int64_t stall_count = 0; /* Make sure we make forward progress... */

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

    prev_offset_changed = INT64_MAX;

    if (listname && !keep_all) {
        /* Remove the list file in case we die before the output pass */
        remove(listname);
    }

        global_offset_changed = 0;

	/*
	 * Create a warning buffer list unless we are in
         * pass 2 (everything will be emitted immediately in pass 2.)
	 */
	if (warn_list) {
            if (warn_list->nstr || pass_final())
                strlist_free(&warn_list);
        }

	if (!pass_final() && !warn_list)
            warn_list = strlist_alloc(false);

        globalbits = cmd_sb;  /* set 'bits' to command line default */
        cpu = cmd_cpu;
        if (listname) {
            if (pass_final() || list_on_every_pass()) {
                lfmt->init(listname);
            } else if (list_active()) {
                /*
                 * Looks like we used the list engine on a previous pass,
                 * but now it is turned off, presumably via %pragma -p
                 */
                lfmt->cleanup();
                if (!keep_all)
                    remove(listname);
            }
        }

        in_absolute = false;
        location.segment = NO_SEG;
        location.offset  = 0;
        if (pass_first())
            location.known = true;

  /* mov ax,4 */
  insn output_ins =
  {
    .label = 0x0,
    .prefixes = {0, 0, 0, 0, 254, 0, 0},
    .opcode = I_MOV,
    .condition = C_none,
    .operands = 2,
    .addr_size = 32,
    .oprs = {
      {.type = 17180393733, .disp_size = 0, .basereg = R_AX, .indexreg = R_none, .scale = 0, 
      .hintbase = 0, .hinttype = EAH_NOHINT, .segment = -1, .offset = 0, .wrt = -1, .eaflags = 0, .opflags = 0, 
      .decoflags = 0},
      {.type = 7864322, .disp_size = 0, .basereg = R_none, .indexreg = R_none, .scale = 0, .hintbase = 0, 
      .hinttype = EAH_NOHINT, .segment = -1, .offset = 4, .wrt = -1, .eaflags = 0, .opflags = 0, .decoflags = 0},
      {.type = 0, .disp_size = 0, .basereg = R_none, .indexreg = R_none, .scale = 0, .hintbase = 0,
      .hinttype = EAH_NOHINT, .segment = -1, .offset = 0, .wrt = -1, .eaflags = 0, .opflags = 0, .decoflags = 0},
      {.type = 0, .disp_size = 0, 
      .basereg = R_zero, .indexreg = R_AH, .scale = 0, .hintbase = 1433524224, .hinttype = 21845, .segment = 2007385088, 
      .offset = 140737488346032, .wrt = 1432088262, .eaflags = 21845, .opflags = 1432502559, .decoflags = 21845},
      {
      .type = 0, .disp_size = 1432504795, .basereg = R_CR0, .indexreg = 1432502606, .scale = 16, .hintbase = -9200, 
      .hinttype = 32767, .segment = 1432225456, .offset = 201863454064, .wrt = 1432702944, .eaflags = 21845, 
      .opflags = 1433502432, .decoflags = 21845}
    },
    .eops = 0x0,
    .eops_float = 1844422398,
    .times = 1,
    .forw_ref = false,
    .rex_done = false,
    .rex = 0,
    .vexreg = 14,
    .vex_cm = 0,
    .vex_wlp = 0,
    .evex_p = "\000\000",
    .evex_tuple = 1433518784,
    .evex_rm = 0,
    .evex_brerop = -1
  };
            /* Not a directive, or even something that starts with [ */
            process_insn(&output_ins);

        /* We better not be having an error hold still... */
        nasm_assert(!errhold_stack);

        if (global_offset_changed) {
            switch (pass_type()) {
            case PASS_OPT:
                /*
                 * This is the only pass type that can be executed more
                 * than once, and therefore has the ability to stall.
                 */
                if (global_offset_changed < prev_offset_changed) {
                    prev_offset_changed = global_offset_changed;
                    stall_count = 0;
                } else {
                    stall_count++;
                }

                break;

            case PASS_STAB:
                /*!
                 *!phase [off] phase error during stabilization
                 *!  warns about symbols having changed values during
                 *!  the second-to-last assembly pass. This is not
                 *!  inherently fatal, but may be a source of bugs.
                 */
                nasm_warn(WARN_PHASE|ERR_UNDEAD,
                          "phase error during stabilization "
                          "pass, hoping for the best");
                break;

            case PASS_FINAL:
                nasm_nonfatalf(ERR_UNDEAD,
                               "phase error during code generation pass");
                break;

            default:
                /* This is normal, we'll keep going... */
                break;
            }
        }

        reset_warnings();

    if (opt_verbose_info && pass_final()) {
        /*  -On and -Ov switches */
        nasm_info("assembly required 1+%"PRId64"+2 passes\n", pass_count()-3);
    }

    lfmt->cleanup();
    strlist_free(&warn_list);
}

/**
 * get warning index; 0 if this is non-suppressible.
 */
static size_t warn_index(errflags severity)
{
    size_t index;

    if ((severity & ERR_MASK) >= ERR_FATAL)
        return 0;               /* Fatal errors are never suppressible */

    /* Warnings MUST HAVE a warning category specifier! */
    nasm_assert((severity & (ERR_MASK|WARN_MASK)) != ERR_WARNING);

    index = WARN_IDX(severity);
    nasm_assert(index < WARN_IDX_ALL);

    return index;
}

static bool skip_this_pass(errflags severity)
{
    errflags type = severity & ERR_MASK;

    /*
     * See if it's a pass-specific error or warning which should be skipped.
     * We can never skip fatal errors as by definition they cannot be
     * resumed from.
     */
    if (type >= ERR_FATAL)
        return false;

    /*
     * ERR_LISTMSG messages are always skipped; the list file
     * receives them anyway as this function is not consulted
     * for sending to the list file.
     */
    if (type == ERR_LISTMSG)
        return true;

    /*
     * This message not applicable unless it is the last pass we are going
     * to execute; this can be either the final code-generation pass or
     * the single pass executed in preproc-only mode.
     */
    return (severity & ERR_PASS2) && !pass_final_or_preproc();
}

/**
 * check for suppressed message (usually warnings or notes)
 *
 * @param severity the severity of the warning or error
 * @return true if we should abort error/warning printing
 */
static bool is_suppressed(errflags severity)
{
    /* Fatal errors must never be suppressed */
    if ((severity & ERR_MASK) >= ERR_FATAL)
        return false;

    /* This error/warning is pointless if we are dead anyway */
    if ((severity & ERR_UNDEAD) && terminate_after_phase)
        return true;

    if (!(warning_state[warn_index(severity)] & WARN_ST_ENABLED))
        return true;

    return false;
}

/**
 * Return the true error type (the ERR_MASK part) of the given
 * severity, accounting for warnings that may need to be promoted to
 * error.
 *
 * @param severity the severity of the warning or error
 * @return true if we should error out
 */
static errflags true_error_type(errflags severity)
{
    const uint8_t warn_is_err = WARN_ST_ENABLED|WARN_ST_ERROR;
    int type;

    type = severity & ERR_MASK;

    /* Promote warning to error? */
    if (type == ERR_WARNING) {
        uint8_t state = warning_state[warn_index(severity)];
        if ((state & warn_is_err) == warn_is_err)
            type = ERR_NONFATAL;
    }

    return type;
}

/*
 * The various error type prefixes
 */
static const char * const error_pfx_table[ERR_MASK+1] = {
    ";;; ", "debug: ", "info: ", "warning: ",
        "error: ", "fatal: ", "critical: ", "panic: "
};
static const char no_file_name[] = "nasm"; /* What to print if no file name */

/*
 * For fatal/critical/panic errors, kill this process.
 */
static fatal_func die_hard(errflags true_type, errflags severity)
{
    fflush(NULL);

    if (true_type == ERR_PANIC && abort_on_panic)
        abort();

    if (ofile) {
        fclose(ofile);
        if (!keep_all)
            remove(outname);
        ofile = NULL;
    }

    if (severity & ERR_USAGE)
        usage();

    /* Terminate immediately */
    exit(true_type - ERR_FATAL + 1);
}

/*
 * Returns the struct src_location appropriate for use, after some
 * potential filename mangling.
 */
static struct src_location error_where(errflags severity)
{
    struct src_location where;

    if (severity & ERR_NOFILE) {
        where.filename = NULL;
        where.lineno = 0;
    } else {
        where = src_where_error();

        if (!where.filename) {
            where.filename =
            inname && inname[0] ? inname :
                outname && outname[0] ? outname :
                NULL;
            where.lineno = 0;
        }
    }

    return where;
}

/*
 * error reporting for critical and panic errors: minimize
 * the amount of system dependencies for getting a message out,
 * and in particular try to avoid memory allocations.
 */
fatal_func nasm_verror_critical(errflags severity, const char *fmt, va_list args)
{
    struct src_location where;
    errflags true_type = severity & ERR_MASK;
    static bool been_here = false;

    if (unlikely(been_here))
        abort();                /* Recursive error... just die */

    been_here = true;

    where = error_where(severity);
    if (!where.filename)
        where.filename = no_file_name;

    fputs(error_pfx_table[severity], error_file);
    fputs(where.filename, error_file);
    if (where.lineno) {
        fprintf(error_file, "%s%"PRId32"%s",
                errfmt->beforeline, where.lineno, errfmt->afterline);
    }
    fputs(errfmt->beforemsg, error_file);
    vfprintf(error_file, fmt, args);
    fputc('\n', error_file);

    die_hard(true_type, severity);
}

/**
 * Stack of tentative error hold lists.
 */
struct nasm_errtext {
    struct nasm_errtext *next;
    char *msg;                  /* Owned by this structure */
    struct src_location where;  /* Owned by the srcfile system */
    errflags severity;
    errflags true_type;
};
struct nasm_errhold {
    struct nasm_errhold *up;
    struct nasm_errtext *head, **tail;
};

static void nasm_free_error(struct nasm_errtext *et)
{
    nasm_free(et->msg);
    nasm_free(et);
}

static void nasm_issue_error(struct nasm_errtext *et);

struct nasm_errhold *nasm_error_hold_push(void)
{
    struct nasm_errhold *eh;

    nasm_new(eh);
    eh->up = errhold_stack;
    eh->tail = &eh->head;
    errhold_stack = eh;

    return eh;
}

void nasm_error_hold_pop(struct nasm_errhold *eh, bool issue)
{
    struct nasm_errtext *et, *etmp;

    /* Allow calling with a null argument saying no hold in the first place */
    if (!eh)
        return;

    /* This *must* be the current top of the errhold stack */
    nasm_assert(eh == errhold_stack);

    if (eh->head) {
        if (issue) {
            if (eh->up) {
                /* Commit the current hold list to the previous level */
                *eh->up->tail = eh->head;
                eh->up->tail = eh->tail;
            } else {
                /* Issue errors */
                list_for_each_safe(et, etmp, eh->head)
                    nasm_issue_error(et);
            }
        } else {
            /* Free the list, drop errors */
            list_for_each_safe(et, etmp, eh->head)
                nasm_free_error(et);
        }
    }

    errhold_stack = eh->up;
    nasm_free(eh);
}

/**
 * common error reporting
 * This is the common back end of the error reporting schemes currently
 * implemented.  It prints the nature of the warning and then the
 * specific error message to error_file and may or may not return.  It
 * doesn't return if the error severity is a "panic" or "debug" type.
 *
 * @param severity the severity of the warning or error
 * @param fmt the printf style format string
 */
void nasm_verror(errflags severity, const char *fmt, va_list args)
{
    struct nasm_errtext *et;
    errflags true_type = true_error_type(severity);

    if (true_type >= ERR_CRITICAL)
        nasm_verror_critical(severity, fmt, args);

    if (is_suppressed(severity))
        return;

    nasm_new(et);
    et->severity = severity;
    et->true_type = true_type;
    et->msg = nasm_vasprintf(fmt, args);
    et->where = error_where(severity);

    if (errhold_stack && true_type <= ERR_NONFATAL) {
        /* It is a tentative error */
        *errhold_stack->tail = et;
        errhold_stack->tail = &et->next;
    } else {
        nasm_issue_error(et);
    }

    /*
     * Don't do this before then, if we do, we lose messages in the list
     * file, as the list file is only generated in the last pass.
     */
    if (skip_this_pass(severity))
        return;
}

/*
 * Actually print, list and take action on an error
 */
static void nasm_issue_error(struct nasm_errtext *et)
{
    const char *pfx;
    char warnsuf[64];           /* Warning suffix */
    char linestr[64];           /* Formatted line number if applicable */
    const errflags severity  = et->severity;
    const errflags true_type = et->true_type;
    const struct src_location where = et->where;

    if (severity & ERR_NO_SEVERITY)
        pfx = "";
    else
        pfx = error_pfx_table[true_type];

    *warnsuf = 0;
    if ((severity & (ERR_MASK|ERR_HERE|ERR_PP_LISTMACRO)) == ERR_WARNING) {
        /*
         * It's a warning without ERR_HERE defined, and we are not already
         * unwinding the macros that led us here.
         */
        snprintf(warnsuf, sizeof warnsuf, " [-w+%s%s]",
                 (true_type >= ERR_NONFATAL) ? "error=" : "",
                 warning_name[warn_index(severity)]);
    }

    *linestr = 0;
    if (where.lineno) {
        snprintf(linestr, sizeof linestr, "%s%"PRId32"%s",
                 errfmt->beforeline, where.lineno, errfmt->afterline);
    }

    if (!skip_this_pass(severity)) {
        const char *file = where.filename ? where.filename : no_file_name;
        const char *here = "";

        if (severity & ERR_HERE) {
            here = where.filename ? " here" : " in an unknown location";
        }

        if (warn_list && true_type < ERR_NONFATAL) {
            /*
             * Buffer up warnings until we either get an error
             * or we are on the code-generation pass.
             */
            strlist_printf(warn_list, "%s%s%s%s%s%s%s",
                           file, linestr, errfmt->beforemsg,
                           pfx, et->msg, here, warnsuf);
        } else {
            /*
             * Actually output an error.  If we have buffered
             * warnings, and this is a non-warning, output them now.
             */
            if (true_type >= ERR_NONFATAL && warn_list) {
                strlist_write(warn_list, "\n", error_file);
                strlist_free(&warn_list);
            }

            fprintf(error_file, "%s%s%s%s%s%s%s\n",
                    file, linestr, errfmt->beforemsg,
                    pfx, et->msg, here, warnsuf);
        }
    }

    /* Are we recursing from error_list_macros? */
    if (severity & ERR_PP_LISTMACRO)
        goto done;

    /*
     * Don't suppress this with skip_this_pass(), or we don't get
     * pass1 or preprocessor warnings in the list file
     */
    if (severity & ERR_HERE) {
        if (where.lineno)
            lfmt->error(severity, "%s%s at %s:%"PRId32"%s",
                        pfx, et->msg, where.filename, where.lineno, warnsuf);
        else if (where.filename)
            lfmt->error(severity, "%s%s in file %s%s",
                        pfx, et->msg, where.filename, warnsuf);
        else
            lfmt->error(severity, "%s%s in an unknown location%s",
                        pfx, et->msg, warnsuf);
    } else {
        lfmt->error(severity, "%s%s%s", pfx, et->msg, warnsuf);
    }

    if (skip_this_pass(severity))
        goto done;

    if (true_type >= ERR_FATAL)
        die_hard(true_type, severity);
    else if (true_type >= ERR_NONFATAL)
        terminate_after_phase = true;

done:
    nasm_free_error(et);
}

static void usage(void)
{
    fprintf(error_file, "Type %s -h for help.\n", _progname);
}

static void help(FILE *out)
{
    int i;

    fprintf(out,
            "Usage: %s [-@ response_file] [options...] [--] filename\n"
            "       %s -v (or --v)\n",
            _progname, _progname);
    fputs(
        "\n"
        "Options (values in brackets indicate defaults):\n"
        "\n"
        "    -h            show this text and exit (also --help)\n"
        "    -v (or --v)   print the NASM version number and exit\n"
        "    -@ file       response file; one command line option per line\n"
        "\n"
        "    -o outfile    write output to outfile\n"
        "    --keep-all    output files will not be removed even if an error happens\n"
        "\n"
        "    -Xformat      specifiy error reporting format (gnu or vc)\n"
        "    -s            redirect error messages to stdout\n"
        "    -Zfile        redirect error messages to file\n"
        "\n"
        "    -M            generate Makefile dependencies on stdout\n"
        "    -MG           d:o, missing files assumed generated\n"
        "    -MF file      set Makefile dependency file\n"
        "    -MD file      assemble and generate dependencies\n"
        "    -MT file      dependency target name\n"
        "    -MQ file      dependency target name (quoted)\n"
        "    -MP           emit phony targets\n"
        "\n"
        "    -f format     select output file format\n"
        , out);
    fputs(
        "\n"
        "    -g            generate debugging information\n"
        "    -F format     select a debugging format (output format dependent)\n"
        "    -gformat      same as -g -F format\n"
        , out);
    dfmt_list(out);
    fputs(
        "\n"
        "    -l listfile   write listing to a list file\n"
        "    -Lflags...    add optional information to the list file\n"
        "       -Lb        show builtin macro packages (standard and %use)\n"
        "       -Ld        show byte and repeat counts in decimal, not hex\n"
        "       -Le        show the preprocessed output\n"
        "       -Lf        ignore .nolist (force output)\n"
        "       -Lm        show multi-line macro calls with expanded parmeters\n"
        "       -Lp        output a list file every pass, in case of errors\n"
        "       -Ls        show all single-line macro definitions\n"
        "       -Lw        flush the output after every line (very slow!)\n"
        "       -L+        enable all listing options except -Lw (very verbose!)\n"
        "\n"
        "    -Oflags...    optimize opcodes, immediates and branch offsets\n"
        "       -O0        no optimization\n"
        "       -O1        minimal optimization\n"
        "       -Ox        multipass optimization (default)\n"
        "       -Ov        display the number of passes executed at the end\n"
        "    -t            assemble in limited SciTech TASM compatible mode\n"
        "\n"
        "    -E (or -e)    preprocess only (writes output to stdout by default)\n"
        "    -a            don't preprocess (assemble only)\n"
        "    -Ipath        add a pathname to the include file path\n"
        "    -Pfile        pre-include a file (also --include)\n"
        "    -Dmacro[=str] pre-define a macro\n"
        "    -Umacro       undefine a macro\n"
        "   --pragma str   pre-executes a specific %%pragma\n"
        "   --before str   add line (usually a preprocessor statement) before the input\n"
        "   --no-line      ignore %line directives in input\n"
        "\n"
        "   --prefix str   prepend the given string to the names of all extern,\n"
        "                  common and global symbols (also --gprefix)\n"
        "   --suffix str   append the given string to the names of all extern,\n"
        "                  common and global symbols (also --gprefix)\n"
        "   --lprefix str  prepend the given string to local symbols\n"
        "   --lpostfix str append the given string to local symbols\n"
        "\n"
        "   --reproducible attempt to produce run-to-run identical output\n"
        "\n"
        "    -w+x          enable warning x (also -Wx)\n"
        "    -w-x          disable warning x (also -Wno-x)\n"
        "    -w[+-]error   promote all warnings to errors (also -Werror)\n"
        "    -w[+-]error=x promote warning x to errors (also -Werror=x)\n"
        , out);

    fprintf(out, "       %-20s %s\n",
            warning_name[WARN_IDX_ALL], warning_help[WARN_IDX_ALL]);

    for (i = 1; i < WARN_IDX_ALL; i++) {
        const char *me   = warning_name[i];
        const char *prev = warning_name[i-1];
        const char *next = warning_name[i+1];

        if (prev) {
            int prev_len = strlen(prev);
            const char *dash = me;

            while ((dash = strchr(dash+1, '-'))) {
                int prefix_len = dash - me; /* Not including final dash */
                if (strncmp(next, me, prefix_len+1)) {
                    /* Only one or last option with this prefix */
                    break;
                }
                if (prefix_len >= prev_len ||
                    strncmp(prev, me, prefix_len) ||
                    (prev[prefix_len] != '-' && prev[prefix_len] != '\0')) {
                    /* This prefix is different from the previous option */
                    fprintf(out, "       %-20.*s all warnings prefixed with \"%.*s\"\n",
                            prefix_len, me, prefix_len+1, me);
                }
            }
        }

        fprintf(out, "       %-20s %s%s\n",
                warning_name[i], warning_help[i],
                (warning_default[i] & WARN_ST_ERROR) ? " [error]" :
                (warning_default[i] & WARN_ST_ENABLED) ? " [on]" : " [off]");
    }

    fputs(
        "\n"
        "   --limit-X val  set execution limit X\n"
        , out);
}
