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
        _progname = "test";

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
