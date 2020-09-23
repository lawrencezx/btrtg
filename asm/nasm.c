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
#include "listing.h"
#include "ver.h"
#include "test.h"

const char *_progname;

static void parse_cmdline(int, char **, int);
static void usage(void);
static void help(FILE *);

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

int main(int argc, char *argv[])
{
  _progname = argv[0];
  if (!_progname || !_progname[0])
      _progname = "nasm";

  parse_cmdline(argc, argv, 1);
  if (terminate_after_phase) {
      if (want_usage)
          usage();
      return 1;
  }

  parse_cmdline(argc, argv, 2);
  if (terminate_after_phase) {
      if (want_usage)
          usage();
      return 1;
  }

  test_MOV();

  if (want_usage)
      usage();

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
        case 'L':        /* listing options */
            if (pass == 2) {
                while (*param)
                    list_options |= list_option_mask(*param++);
            }
            break;

        case 'h':
            help(stdout);
            exit(0);    /* never need usage message here */
            break;

        case 'v':
            show_version();
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
                case OPT_MANGLE:
                    if (pass == 2)
                        set_label_mangle(tx->pvt, param);
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
}

static void usage(void)
{
    fprintf(stderr, "Type %s -h for help.\n", _progname);
}

static void help(FILE *out)
{
    int i;

    fprintf(out,
            "Usage: %s [-@ response_file] [options...] [--]\n"
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
        , out);
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
