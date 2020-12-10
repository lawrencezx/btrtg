#include "compiler.h"

#include "seed.h"
#include "generator.h"
#include "insn/insn-test.h"
#include "ut.h"

enum {
  OK_ARG,
  EXIT_ARG,
  ERR_ARG
};

static void usage(void)
{
  fprintf(stdout, "usage: ut\n");
  fprintf(stdout, "  %-15s%-40s\n", "-h", "print this help");
  fprintf(stdout, "  %-15s\n", "-help");
  fprintf(stdout, "  %-15s%-40s\n", "-boundary", "generator boundary test (default is random)");
}

static int parse_args(int argc, char *argv[])
{
  for (int i = 1; i < argc; ++i) {
    char *param = argv[i];
    if (strcmp(param, "-h") == 0 || strcmp(param, "-help") == 0) {
      usage();
      return EXIT_ARG;
    } else {
      fprintf(stderr, "Parse arguments error!\n");
      usage();
      return ERR_ARG;
    }
  }
  return OK_ARG;
}

/* ==========================================================================*/
/* Entry point                                                               */
/* ==========================================================================*/
 
int main(int argc, char *argv[])
{
  int op = parse_args(argc, argv);
  if (op == ERR_ARG || op == EXIT_ARG) {
    return 0;
  }
  generator_init(false);
  if (!gen_test_file_ADD()) {
      fprintf(stderr, "genrate ADD test file failed!\n");
  }
  if (!gen_test_file_MOV()) {
      fprintf(stderr, "genrate MOV test file failed!\n");
  }
  if (!gen_test_file_DIV()) {
      fprintf(stderr, "genrate DIV test file failed!\n");
  }
  generator_exit();
  return 0;
}
