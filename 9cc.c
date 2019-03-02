#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Token types
enum {
  TK_NUM = 256, // integer
  TK_EOF,       // end of input
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

Token tokens[100];
int pos = 0;

void tokenize(char *p) {
  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+'
        || *p == '-'
        || *p == '*' 
        || *p == '/'
        || *p == '('
        || *p == ')') {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "Cannot tokenize: %s\n", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

void error(int i) {
  fprintf(stderr, "Unexpected token: %s\n", tokens[i].input);
  exit(1);
}

// Parser
enum {
  ND_NUM = 256,
};

typedef struct Node {
  int ty;
  struct Node *lhs; // Left hand side
  struct Node *rhs; // Right hand side
  int val;
} Node;

Node *add();
Node *mul();
Node *term();

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

// check next token type with expected type
// proceed to next if true,
// return 0 if false
int consume(int ty) {
  if (tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *term() {
  if (consume('(')) {
    Node *node = add();
    if (!consume(')'))
      fprintf(stderr, "No closing parentheses: %s", tokens[pos].input);
    return node;
  }

  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  fprintf(stderr, "Not a number nor open parentheses: %s", tokens[pos].input);
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

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
    case '+':
      printf("  add rax, rdi\n");
      break;
    case '-':
      printf("  sub rax, rdi\n");
      break;
    case '*':
      printf("  mul rdi\n");
      break;
    case '/':
      printf("  mov rdx, 0\n");
      printf("  div rdi\n");
  }

  printf("  push rax\n");
}



int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // Tokenize and parse
  tokenize(argv[1]);
  Node *node = add();

  // First Template of assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

