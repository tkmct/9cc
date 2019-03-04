#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TOKENIZER
enum
{
  TK_NUM = 256,
  TK_EOF,
};

typedef struct
{
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector()
{
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem)
{
  if (vec->capacity == vec->len)
  {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

typedef struct
{
  int ty;
  int val;
} Token;

Token *new_token(int ty, int val)
{
  Token *t = calloc(1, sizeof(Token));
  t->ty = ty;
  t->val = val;

  return t;
}

// token position
int pos = 0;
Vector *tokens;

void tokenize(char *p)
{
  tokens = new_vector();

  int i = 0;
  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
    {
      vec_push(tokens, new_token(*p, 0));
      p++;
      continue;
    }

    if (isdigit(*p))
    {
      vec_push(tokens, new_token(TK_NUM, strtol(p, &p, 10)));
      continue;
    }

    fprintf(stderr, "Cannot tokenize: %s\n", p);
    exit(1);
  }

  vec_push(tokens, new_token(TK_EOF, 0));
}

// PARSER
enum
{
  ND_NUM = 256,
};

typedef struct Node
{
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
} Node;

Node *add();
Node *mul();
Node *term();

Node *new_node(int ty, Node *lhs, Node *rhs)
{
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val)
{
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int consume(int ty)
{
  if (((Token *)tokens->data[pos])->ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *term()
{
  if (consume('('))
  {
    Node *node = add();
    if (!consume(')'))
      fprintf(stderr, "No closing parentheses: %c", ((Token *)tokens->data[pos])->ty);
    return node;
  }

  if (((Token *)tokens->data[pos])->ty == TK_NUM)
    return new_node_num(((Token *)tokens->data[pos++])->val);

  fprintf(stderr, "Not a number nor open parentheses: %c", ((Token *)tokens->data[pos])->ty);
}

Node *mul()
{
  Node *node = term();

  for (;;)
  {
    if (consume('*'))
      node = new_node('*', node, term());
    else if (consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *add()
{
  Node *node = mul();

  for (;;)
  {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

// GENERATOR
void gen(Node *node)
{
  if (node->ty == ND_NUM)
  {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty)
  {
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

int expect(int line, int expected, int actual)
{
  if (expected == actual)
    return 1;
  fprintf(stderr, "%d: %d expected, but got %d\n",
          line, expected, actual);
  exit(1);
}

void runtest()
{
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++)
    vec_push(vec, (void *)i);

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);

  printf("OK\n");
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  if (argc == 2 && !strcmp(argv[1], "-test")) {
    runtest();
    return 0;
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
