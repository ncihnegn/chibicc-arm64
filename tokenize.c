#include "chibicc.h"

// Keyword
static char *kw[] = {"return", "if", "else", "while"};

// Multi-character punctuator
static char *ops[] = {"==", "!=", "<=", ">="};

// Program input
char *user_input;

// Current token
Token *token;

// Report an error and exit
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

// Consume the current token if it matches `op`.
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      strncmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// Consume the current token if it is an identifier.
Token *consume_id() {
  if (token->kind != TK_ID)
    return NULL;
  Token *t = token;
  token = token->next;
  return t;
}

// Ensure the current token if it matches `op`.
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      strncmp(token->str, op, token->len))
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

bool startswith(char *p, char *q) { return strncmp(p, q, strlen(q)) == 0; }

char *starts_with_reserved(char *p) {
  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (startswith(p, kw[i]) && !isalnum(p[strlen(kw[i])]))
      return kw[i];

  for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++)
    if (startswith(p, ops[i]))
      return ops[i];

  return NULL;
}

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

    // Reserved
    char *kw = starts_with_reserved(p);
    if (kw) {
      int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    }

    // Single-character punctuator
    if (strchr("+-*/()<>;=", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Identifier
    if (isalpha(*p)) {
      char *q = p++;
      while (isalnum(*p))
        p++;
      cur = new_token(TK_ID, cur, q, p - q);
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
