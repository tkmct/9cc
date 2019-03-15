#include "9cc.h"

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
  printf("  sub rsp, %d\n", vars->keys->len * 8);

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

