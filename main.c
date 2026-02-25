#include "defs.h"

int main(int argc, char **args) {

  if (argc <= 1) {
    printf("usage: %s <file.c>\n", args[0]);
    return 0;
  }

  if (!lex_init(args[1])) {
    return 1;
  }

  parse();

  return 0;
}
