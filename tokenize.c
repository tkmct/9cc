#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256,
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

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

Token *new_token(int ty, int val, char* input) {
  Token *t = malloc(sizeof(Token));
  t->ty = ty;
  t->val = val;
  t->input = input;

  return t;
}

// token position
int pos = 0;
Vector *tokens;

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
      vec_push(tokens, new_token(*p, 0, p));
      p++;
      continue;
    }

    if (isdigit(*p)) {
      vec_push(tokens, new_token(TK_NUM, p, strtol(p, &p, 10)));
      continue;
    }

    fprintf(stderr, "Cannot tokenize: %s\n", p);
    exit(1);
  }

  vec_push(tokens, new_token(TK_EOF, 0, p));
}
