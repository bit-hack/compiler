#include "defs.h"

int main(int argc, char **args) {

  if (argc <= 1) {
    printf("usage: %s <file.c>\n", args[0]);
    return 0;
  }

  if (!lInit(args[1])) {
    return 1;
  }

  ast_node_p n = pParse();
  if (!n) {
    return 1;
  }

  sCheck(n);

  aDump(n);

  return 0;
}
