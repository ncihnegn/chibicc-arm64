#include "chibicc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments", argv[0]);
    return 1;
  }

  user_input = argv[1];
  token = tokenize();
  Program *prog = program();

  // Assign offsets to local variables.
  int offset = 0;
  for (Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }
  int bytes = offset / 8;
  // ARM64 standard ABI requires 16-byte alignment
  prog->stack_size = (bytes / 2 + bytes & 0x1) * 16;
  // Traverse the AST to emit assembly.
  codegen(prog);
  return 0;
}
