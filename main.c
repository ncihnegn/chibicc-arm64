#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer
//

// Token kind
typedef enum {
  TK_RESERVED, // Keywords or punctuators
  TK_NUM,      // Integer literals
  TK_EOF,      // End-of-file markers
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
  TokenKind kind; // Token kind
  Token *next;    // Next token
  int val;        // If kind is TK_NUM, its value
  char *str;      // Token string
  int len;        // Token length
};

// Program input
char *user_input;

// Current token
Token *token;

// Report an error and exit.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Report an error location and exit.
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print `pos` leading spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Consumes the current token if it matches `op`.
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// Ensure the current token if it matches `op`.
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "expected \"%s\"", op);
  token = token->next;
}

// Ensure the current token is TK_NUM.
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "expected a number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// Create a new token and add it as the next token of `cur`.
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

// Tokenize `user_input` and returns new tokens.
Token *tokenize() {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  char *p = user_input;
  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Multi-character punctuator
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // Single-character punctuator
    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Integer literal
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

//
// Parser
//

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind; // Node kind
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side
  int val;       // Used if kind == ND_NUM
};

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr();
Node *eq();
Node *cmp();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = eq
Node *expr() { return eq(); }

// eq = cmp ("==" cmp | "!=" cmp)*
Node *eq() {
  Node *node = cmp();

  while (true) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, cmp());
    else if (consume("!="))
      node = new_binary(ND_NE, node, cmp());
    else
      return node;
  }
}

// cmp = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *cmp() {
  Node *node = add();

  while (true) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  while (true) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  while (true) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? unary
//       | primary
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());
  return primary();
}

// primary = "(" expr ")" | num
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  return new_num(expect_number());
}

//
// Code generator
//

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("\tmov w0, #%d\n", node->val);
    // ARM64 standard ABI requires 16-byte alignment
    printf("\tstr w0, [sp, #-16]!\n");
    return;
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

int main(int argc, char **argv) {
  if (argc != 2) {
    error("%s: invalid number of arguments", argv[0]);
    return 1;
  }

  // Assembly header
  printf("\t.global _main\n");
  printf("\t.p2align 2\n");
  printf("_main:\n");

  user_input = argv[1];
  token = tokenize();
  Node *node = expr();
  // Traverse the AST to emit assembly.
  gen(node);
  printf("\tldr w0, [sp], #16\n");
  printf("\tret\n");
  return 0;
}
