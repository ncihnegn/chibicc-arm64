#include "chibicc.h"

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("\tmov w0, #%d\n", node->val);
    // ARM64 standard ABI requires 16-byte alignment
    printf("\tstr w0, [sp, #-16]!\n");
    return;
  case ND_RT:
    gen(node->lhs);
    return;
  default:;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("\tldr w1, [sp], #16\n");
  printf("\tldr w0, [sp], #16\n");

  switch (node->kind) {
  case ND_ADD:
    printf("\tadd w0, w0, w1\n");
    break;
  case ND_SUB:
    printf("\tsub w0, w0, w1\n");
    break;
  case ND_MUL:
    printf("\tmul w0, w0, w1\n");
    break;
  case ND_DIV:
    printf("\tudiv w0, w0, w1\n");
    break;
  case ND_EQ:
    printf("\tsubs\tw0, w0, w1\n");
    printf("\tcset\tw0, eq\n");
    printf("\tand w0, w0, #0x1\n");
    break;
  case ND_NE:
    printf("\tsubs\tw0, w0, w1\n");
    printf("\tcset\tw0, ne\n");
    printf("\tand w0, w0, #0x1\n");
    break;
  case ND_LT:
    printf("\tsubs\tw0, w0, w1\n");
    printf("\tcset\tw0, lt\n");
    printf("\tand w0, w0, #0x1\n");
    break;
  case ND_LE:
    printf("\tsubs\tw0, w0, w1\n");
    printf("\tcset\tw0, le\n");
    printf("\tand w0, w0, #0x1\n");
    break;
  default:
    error("unexpected node kind: %d", node->kind);
  }

  printf("\tstr w0, [sp, #-16]!\n");
}

void codegen(Node *node) {
  // Assembly header
  printf("\t.global _main\n");
  printf("\t.p2align 2\n");
  printf("_main:\n");

  for (Node *n = node; n; n = n->next) {
    gen(n);
    // A result must be at the top of the stack, so pop it to return.
    printf("\tldr w0, [sp], #16\n");
  }

  printf("\tret\n");
}
