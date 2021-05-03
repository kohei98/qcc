#include "qcc.h"

LVar *localsinit()
{
    LVar *var = calloc(1, sizeof(LVar));
    var->next = NULL;
    var->name = "";
    var->offset = 0;
    var->len = 0;
    return var;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    // トークナイズしてパースする．
    // LVar *locals;  //ローカル変数を格納する連結リスト
    // locals[0] = localsinit(); //変数のリストを初期化
    user_input = argv[1];
    token = tokenize(argv[1]);

    program();
    // printf("---------\n");
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");

    // // プロローグ　ローカル変数分の領域を確保する

    // 先頭の式から順にコードを生成
    for (int i = 0; code[i]; i++)
    {
        func_no = i;
        gen(code[func_no]);
    }
    return 0;
}
