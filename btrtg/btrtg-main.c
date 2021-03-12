#include "compiler.h"

#include "nasm.h"
#include "seed.h"
#include "generator.h"
#include "tmplt.h"
#include "ofmt.h"
#include "dfmt.h"
#include "insn-test.h"
#include "parseXML.h"

static bool terminate_directly = false;
static char *template_file_name = NULL;
static char *output_file_name = "btrtg.s";

char *tk_tmplt_path = "../xmlmodel/tks";
char *group_tmplt_path = "../xmlmodel/templates/group";

static void help(void)
{
    printf("Usage: btrtg -k <path> -g <path> -t <file> [options]\n");
    printf("Options:\n");
    printf("    --help        show this text and exit (also -h)\n");
    printf("    -h            show this text and exit (also --help)\n");
    printf("    -k <path>     path to the testing knowledge template files\n");
    printf("    -g <path>     path to the instruction group template files\n");
    printf("    -t <file>     select a template <file>\n");
    printf("    -o <file>     place the output into <file>\n");
}

static void parse_cmdline(int argc, char *argv[])
{
    if (argc < 2) {
        help();
        terminate_directly = true;
        return;
    }
    int argci = 1;
    while (argci < argc) {
        if (argv[argci][0] == '-') {
            if (argv[argci][1] == '-') {
                if (strcmp(&argv[argci][2], "help") == 0) {
                    help();
                    terminate_directly = true;
                } else {
                    terminate_directly = true;
                }
            } else {
                switch (argv[argci][1]) {
                    case 'h':
                        help();
                        terminate_directly = true;
                        break;
                    case 'k':
                        tk_tmplt_path = argv[++argci];
                        break;
                    case 'g':
                        group_tmplt_path = argv[++argci];
                        break;
                    case 'o':
                        output_file_name = argv[++argci];
                        break;
                    case 't':
                        template_file_name = argv[++argci];
                        break;
                    default:
                        terminate_directly = true;
                        break;
                }
            }
        }
        argci++;
    }
}

int main(int argc, char *argv[])
{
    parse_cmdline(argc, argv);

    if (terminate_directly)
        return 0;

    if (template_file_name == NULL) {
        help();
        return 0;
    }

    generator_init(false);

    parse_tmplts_file(template_file_name);

    ofmt->init(output_file_name);
    dfmt->init("debug_btrtg.txt");

    gsp_init();
    
    walk_tmplt();

    gsp_finish();

    ofmt->cleanup();
    dfmt->cleanup();

    generator_exit();
    return 0;
}
