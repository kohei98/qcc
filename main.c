#include "9cc.h"
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    // トークナイズしてパースする．
    user_input = argv[1];
    token = tokenize(argv[1]);
    program();
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // プロローグ　26文字分の領域を確保する
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    // 先頭の式から順にコードを生成
    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        printf("    pop rax\n");
    }

    // エピローグ
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}