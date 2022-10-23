#include "chibicc.h"

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

Node *new_lvar(char name) {
  Node *node = new_node(ND_LVAR);
  node->name = name;
  return node;
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
Node *program() {
  Node head;
  head.next = NULL;
  Node *cur = &head;

  while (!at_eof()) {
    cur->next = stmt();
    cur = cur->next;
  }
  return head.next;
}

// stmt = "return" expr ";"
//      | expr ";"
Node *stmt() {
  Node *node;
  if (consume("return"))
    node = new_unary(ND_RT, expr());
  else
    node = new_unary(ND_STMT, expr());
  expect(";");
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
  if (tok)
    return new_lvar(*tok->str);

  return new_num(expect_number());
}
