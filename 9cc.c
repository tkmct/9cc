#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenize.c"
#include "parser.c"
#include "gen.c"

int expect(int line, int expected, int actual) {
  if (expected == actual)
    return 1;
  fprintf(stderr, "%d: %d expected, but got %d\n",
          line, expected, actual);
  exit(1);
}

void runtest() {
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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  if (argv[1] == "-test") {
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

