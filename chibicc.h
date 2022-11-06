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
  TK_ID,       // Identifiers
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

extern char *user_input;
extern Token *token;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_id();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

//
// Parser
//

// Local variable
typedef struct Var Var;
struct Var {
  Var *next;
  char *name;
  int offset; // Offset from frame pointer
};

// AST node
typedef enum {
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_ASSIGN, // =
  ND_RT,     // "return"
  ND_IF,     // "if"
  ND_WHILE,  // "while"
  ND_FOR,    // "for"
  ND_BLOCK,  // { ... }
  ND_CALL,   // Function call
  ND_STMT,   // Expression statement
  ND_VAR,    // Variable
  ND_NUM,    // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
  NodeKind kind; // Node kind
  int val;       // Used if kind == ND_NUM
  Var *var;      // Used if kind == ND_VAR
  char *funcname;
  Node *args;

  Node *lhs;  // Left-hand side
  Node *rhs;  // Right-hand side
  Node *next; // Next node

  // conditional statement
  Node *cond;
  Node *then;
  Node *els;
  // "for" statement
  Node *init;
  Node *inc;

  Node *body; // Block
};

typedef struct Program {
  Node *node;
  Var *locals;
  int stack_size;
} Program;

Program *program();

//
// Code generator
//

void codegen(Program *prog);
