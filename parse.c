#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

Map *vars;
Vector *code;

// TOKENIZER
Token *new_token(int ty, int val, char *input) {
  Token *t = calloc(1, sizeof(Token));
  t->ty = ty;
  t->val = val;
  t->input = input;

  return t;
}

// token position
int pos = 0;
Vector *tokens;

void tokenize(char *p) {
  tokens = new_vector();

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if ( *p == '+'
        || *p == '-'
        || *p == '*'
        || *p == '/'
        || *p == '('
        || *p == ')'
        || *p == '='
        || *p == ';') {
      vec_push(tokens, new_token(*p, 0, NULL));
      p++;
      continue;
    }

    if (isdigit(*p)) {
      vec_push(tokens, new_token(TK_NUM, strtol(p, &p, 10), NULL));
      continue;
    }

    if (isalpha(*p) || *p == '_') {
      int len = 1;
      while (isalpha(p[len]) || isdigit(p[len]) || p[len] == '_')
        len++;

      char *name = strndup(p, len);
      vec_push(tokens, new_token(TK_IDENT, 0, name));
      p += len;
      continue;
    }

    fprintf(stderr, "Cannot tokenize: %s\n", p);
    exit(1);
  }

  vec_push(tokens, new_token(TK_EOF, 0, NULL));
}

// PARSER
Node *stmt();
Node *assign();
Node *add();
Node *mul();
Node *term();

void program() {
  code = new_vector();
  vars = new_map();

  while (((Token *)tokens->data[pos])->ty != TK_EOF)
    vec_push(code, (void *)stmt());
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_ident(char *ident) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = ident;
  return node;
}

int consume(int ty) {
  if (((Token *)tokens->data[pos])->ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *stmt() {
  Node *node = assign();
  if (!consume(';')) {
    fprintf(stderr, "Invalid token. expected ';' but got %s:\n", ((Token *)tokens->data[pos])->input);
    exit(1);
  }
  return node;
}

Node *assign() {
  Node *node = add();

  if (consume('=')) {
    Node* rhs = assign();
    map_put(vars, node->name, (void *)vars->keys->len);
    node = new_node('=', node, rhs);
  }
  return node;
}

Node *term() {
  if (consume('(')) {
    Node *node = add();
    if (!consume(')')) {
      fprintf(stderr, "No closing parentheses: %c", ((Token *)tokens->data[pos])->ty);
      exit(1);
    }
    return node;
  }

  if (((Token *)tokens->data[pos])->ty == TK_NUM)
    return new_node_num(((Token *)tokens->data[pos++])->val);

  if (((Token *)tokens->data[pos])->ty == TK_IDENT)
    return new_node_ident(((Token *)tokens->data[pos++])->input);

  fprintf(stderr, "Not a number nor open parentheses: %c", ((Token *)tokens->data[pos])->ty);
  exit(1);
}

Node *mul() {
  Node *node = term();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, term());
    else if (consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}


