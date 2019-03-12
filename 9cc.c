#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// data structures
// Vector
enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EOF,
};

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

void test_vec() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++)
    vec_push(vec, (void *)i);

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);

  printf("OK: test_vec() passed.\n");
}

// Map
typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

Map *new_map() {
  Map *map = malloc(sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  return map;
}

void map_put(Map *map, char *key, void *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
  for (int i = map->keys->len -1; i >= 0; i--)
    if (strcmp(map->keys->data[i], key) == 0)
      return map->vals->data[i];
  return NULL;
}

void test_map() {
  Map *map = new_map();
  expect(__LINE__, 0, (int)map_get(map, "foo"));

  map_put(map, "foo", (void *)2);
  expect(__LINE__, 2, (int)map_get(map, "foo"));

  map_put(map, "bar", (void *)4);
  expect(__LINE__, 4, (int)map_get(map, "bar"));

  map_put(map, "foo", (void *)6);
  expect(__LINE__, 6, (int)map_get(map, "foo"));

  printf("OK: test_map() passed.\n");
}

// TOKENIZER
typedef struct {
  int ty;
  int val;
  char *input;
} Token;

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

    if ('a' <= *p && *p <= 'z') {
      vec_push(tokens, new_token(TK_IDENT, 0, p));
      p++;
      continue;
    }

    fprintf(stderr, "Cannot tokenize: %s\n", p);
    exit(1);
  }

  vec_push(tokens, new_token(TK_EOF, 0, NULL));
}

// PARSER
enum {
  ND_NUM = 256,
  ND_IDENT,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char name;
} Node;

Node *stmt();
Node *assign();
Node *add();
Node *mul();
Node *term();

Vector *code;

void program() {
  code = new_vector();
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
  node->name = *ident;
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
    fprintf(stderr, "Invalid token. expected ';' but got %s:", ((Token *)tokens->data[pos])->input);
    exit(1);
  }
  return node;
}

Node *assign() {
  Node *node = add();

  if (consume('='))
    node = new_node('=', node, assign());
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

// GENERATOR
void gen_lval(Node *node) {
  if (node->ty != ND_IDENT) {
    perror("Invalid left value");
    exit(1);
  }

  int offset = ('z' - node->name + 1) * 8;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}


void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
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

int expect(int line, int expected, int actual) {
  if (expected == actual)
    return 1;
  fprintf(stderr, "%d: %d expected, but got %d\n",
          line, expected, actual);
  exit(1);
}

void runtest() {
  test_vec();
  test_map();
}

int main(int argc, char **argv) {
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
  program();

  // First Template of assembly
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n"); // Need to calculate base pointer offset by number of variables

  // generate code from the first line
  for (int i = 0; i < code->len; i++) {
    gen(code->data[i]);

    // pop remaining statement's value
    printf("  pop rax\n");
  }

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
