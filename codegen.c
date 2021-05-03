#include "qcc.h"
static int labelseq = 1;
const char arg_regi[7][4] = {"rdi\0", "rsi\0", "rdx\0", "rcx\0", "r8\0", "r9\0", "\0"};
void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error("代入の左辺値が変数ではありません");
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}
void gen(Node *node)
{

    if (node->kind == ND_DEF_FUNC)
    {
        printf("%s:\n", node->func_name);
        // // プロローグ　ローカル変数分の領域を確保する
        int var_size = 0;
        for (LVar *var = locals[func_no]; var; var = var->next)
        {
            var_size++;
        }
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, %d\n", (var_size - 1) * 8);
        //レジスタ内の引数の値をローカル変数領域に書き込む
        for (int i = 0; i < node->arg_number; i++)
        {
            // printf("i: %d", i);
            int ofs = (i + 1) * 8;
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n", ofs);
            printf("    mov [rax], %s\n", arg_regi[i]);
        }

        // 先頭の式から順にコードを生成
        gen(node->then);
        //エピローグ
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    }
    if (node->kind == ND_CALL_FUNC)
    {
        int i = 0;
        while (node->argument[i] != NULL)
        {
            gen(node->argument[i]);
            printf("    pop %s\n", arg_regi[i]);
            i++;
        }
        int seq = labelseq++;
        //16バイトアライメントの処理
        printf("    mov rax, rsp\n");
        printf("    and rax, 15\n");         //raxが16の倍数なら15とandを取れば0になる
        printf("    jnz .L.call.%d\n", seq); //0出ない場合分岐
        printf("    mov rax, 0\n");
        printf("    call %s\n", node->func_name);
        printf("    jmp .L.end.%d\n", seq);

        printf(".L.call.%d:\n", seq);
        printf("    sub rsp, 8\n");
        printf("    mov rax,0\n");
        printf("    call %s\n", node->func_name);
        printf("    add rsp, 8\n");
        printf("    .L.end.%d:\n", seq);

        printf("    push rax\n");
        return;
    }
    if (node->kind == ND_BLOCK)
    {
        int i = 0;
        while (node->next_blockstmt[i] != NULL)
        {
            gen(node->next_blockstmt[i]);
            // printf("    pop rax\n");
            i++;
        }
        return;
    }
    if (node->kind == ND_RETURN)
    {
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    }
    if (node->kind == ND_IF)
    {
        int seq = labelseq++;
        if (node->els == NULL)
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lend%d\n", seq);
            gen(node->then);
            printf(".Lend%d:\n", seq);
            return;
        }
        else
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lelse%d\n", seq);
            gen(node->then);
            printf("    jmp .Lend%d\n", seq);
            printf(".Lelse%d:\n", seq);
            gen(node->els);
            printf(".Lend%d:\n", seq);
            return;
        }
    }
    if (node->kind == ND_WHILE)
    {
        int seq = labelseq++;
        printf(".Lbegin%d:\n", seq);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", seq);
        gen(node->then);
        printf("    jmp .Lbegin%d\n", seq);
        printf(".Lend%d:\n", seq);
        return;
    }
    if (node->kind == ND_FOR)
    {
        int seq = labelseq++;
        if (node->init != NULL)
        {
            gen(node->init);
        }
        printf(".Lbegin%d:\n", seq);
        if (node->cond != NULL)
        {
            gen(node->cond);
        }
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", seq);
        gen(node->then);
        if (node->step != NULL)
        {
            gen(node->step);
        }
        printf("    jmp .Lbegin%d\n", seq);
        printf(".Lend%d:\n", seq);
        return;
    }
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs); //"="の左側は左辺値であるはず
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n"); // lhsの値
    printf("    pop rax\n"); // rhsの値
    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n"); // raxとrdiを比較
        printf("    sete al\n");      // 値が同じならalに1が入る
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n"); // raxとrdiを比較
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }
    printf("    push rax\n");
}
