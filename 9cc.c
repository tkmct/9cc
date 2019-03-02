#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenize.c"
#include "parser.c"
#include "gen.c"

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

