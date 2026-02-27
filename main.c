#include "defs.h"

int main(int argc, char **args) {

  if (argc <= 1) {
    printf("usage: %s <file.c>\n", args[0]);
    return 0;
  }

  if (!lex_init(args[1])) {
    return 1;
  }

  ast_node_p n = parse();
  if (!n) {
    return 1;
  }

  sema_check(n);

  ast_dump(n);

  return 0;
}
