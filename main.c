#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "%s: invalid number of arguments\n", argv[0]);
    return 1;
  }

  printf("\t.global _main\n");
  printf("\t.p2align 2\n");
  printf("_main:\n");
  printf("\tmov\tw0, #%d\n", atoi(argv[1]));
  printf("\tret\n");
  return 0;
}
