#include "chibicc.h"

int labelseq = 0;

// Push the given node's address to the stack.
void gen_addr(Node *node) {
  if (node->kind != ND_VAR)
    error("not an lvalue");

  printf("\tsub x0, x29, #%d\n", node->var->offset);
  printf("\tstr x0, [sp, #-16]!\n");
  return;
}

void load() {
  printf("\tldr x0, [sp], #16\n");
  printf("\tldr x0, [x0]\n");
  printf("\tstr x0, [sp, #-16]!\n");
}

void store() {
  printf("\tldr x1, [sp], #16\n");
  printf("\tldr x0, [sp], #0\n");
  printf("\tstr x1, [x0]\n");
  printf("\tstr x1, [sp, #-16]!\n");
}

// Generate code for a given node.
void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("\tmov w0, #%d\n", node->val);
    printf("\tstr w0, [sp, #-16]!\n");
    return;
  case ND_STMT:
    gen(node->lhs);
    return;
  case ND_VAR:
    gen_addr(node);
    load();
    return;
  case ND_ASSIGN:
    gen_addr(node->lhs);
    gen(node->rhs);
    store();
    return;
  case ND_IF:
    labelseq++;
    if (node->els) {
      gen(node->cond);
      printf("\tldr w0, [sp], #16\n");
      printf("\tcbz w0, Lelse%d\n", labelseq);
      gen(node->then);
      printf("\tb\tLend%d:\n", labelseq);
      printf("Lelse%d:\n", labelseq);
      gen(node->els);
      printf("Lend%d:\n", labelseq);
    } else {
      gen(node->cond);
      printf("\tldr w0, [sp], #16\n");
      printf("\tcbz w0, Lend%d\n", labelseq);
      gen(node->then);
      printf("Lend%d:\n", labelseq);
    }
    return;
  case ND_WHILE:
    labelseq++;
    printf("\tLbegin%d:\n", labelseq);
    gen(node->cond);
    printf("\tldr w0, [sp], #16\n");
    printf("\tcbz w0, Lend%d\n", labelseq);
    gen(node->then);
    printf("\tb\tLbegin%d\n", labelseq);
    printf("Lend%d:\n", labelseq);
    return;
  case ND_RT:
    gen(node->lhs);
    printf("\tldr w0, [sp], #16\n");
    printf("\tb\tLreturn\n");
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

void codegen(Program *prog) {
  // Assembly header
  printf("\t.global _main\n");
  printf("\t.p2align 2\n");
  printf("_main:\n");

  // Prologue
  printf("\tstr x29, [sp, #-16]!\n"); // Save frame pointer register
  printf("\tmov x29, sp\n");
  printf("\tsub sp, sp, #%d\n", prog->stack_size); // Reserve

  for (Node *n = prog->node; n; n = n->next) {
    gen(n);
  }

  // Epilogue
  printf("Lreturn:\n");
  printf("\tmov sp, x29\n");
  printf("\tstr x29, [sp], #16\n");

  printf("\tret\n");
}
