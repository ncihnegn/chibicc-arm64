#include "chibicc.h"

Var *locals;

// Find a local variable by name.
Var *find_var(Token *tok) {
  for (Var *var = locals; var; var = var->next)
    if (strlen(var->name) == tok->len &&
        !strncmp(tok->str, var->name, tok->len))
      return var;
  return NULL;
}

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_unary(NodeKind kind, Node *expr) {
  Node *node = new_node(kind);
  node->lhs = expr;
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

Node *new_var(Var *var) {
  Node *node = new_node(ND_VAR);
  node->var = var;
  return node;
}

Var *push_var(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->next = locals;
  var->name = name;
  locals = var;
  return var;
}

Node *stmt();
Node *expr();
Node *assign();
Node *eq();
Node *cmp();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// program = stmt*
Program *program() {
  locals = NULL;

  Node head;
  head.next = NULL;
  Node *cur = &head;

  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }

  Program *prog = calloc(1, sizeof(Program));
  prog->node = head.next;
  prog->locals = locals;
  return prog;
}

Node *read_expr_stmt() { return new_unary(ND_STMT, expr()); }

// stmt = "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";"  expr? ")" stmt
//      | "return" expr ";"
//      | expr ";"
Node *stmt() {
  Node *node = NULL;
  if (consume("if")) {
    node = new_node(ND_IF);
  } else if (consume("while")) {
    node = new_node(ND_WHILE);
  } else if (consume("for")) {
    node = new_node(ND_FOR);
  }
  if (node) {
    expect("(");
    if (node->kind == ND_FOR) {
      if (!consume(";")) {
        node->init = read_expr_stmt();
        expect(";");
      }
      if (!consume(";")) {
        node->cond = expr();
        expect(";");
      }
      if (!consume(")")) {
        node->inc = read_expr_stmt();
        expect(")");
      }
    } else {
      node->cond = expr();
      expect(")");
    }
    node->then = stmt();
    if (node->kind == ND_IF && consume("else"))
      node->els = stmt();
  } else {
    if (consume("return"))
      node = new_unary(ND_RT, expr());
    else
      node = read_expr_stmt();
    expect(";");
  }
  return node;
}

// expr = assign
Node *expr() { return assign(); }

// assign = eq ("=" assign)?
Node *assign() {
  Node *node = eq();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}

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

// primary = "(" expr ")" | id | num
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_id();
  if (tok) {
    Var *var = find_var(tok);
    if (!var)
      var = push_var(strndup(tok->str, tok->len));
    return new_var(var);
  }

  return new_num(expect_number());
}
