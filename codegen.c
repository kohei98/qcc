#include "qcc.h"
int begin_no = 0;
int end_no = 0;
int else_no = 0;
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
    if (node->kind == ND_BLOCK)
    {
        int i = 0;
        while (node->next_blockstmt[i] != NULL)
        {
            gen(node->next_blockstmt[i]);
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
        if (node->els == NULL)
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lend%d\n", end_no);
            gen(node->then);
            printf(".Lend%d:\n", end_no);
            end_no++;
            return;
        }
        else
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lelse%d\n", else_no);
            gen(node->then);
            printf("    jmp .Lend%d\n", end_no);
            printf(".Lelse%d:\n", else_no);
            gen(node->els);
            printf(".Lend%d:\n", end_no);
            end_no++;
            else_no++;
            return;
        }
    }
    if (node->kind == ND_WHILE)
    {
        printf(".Lbegin%d:\n", begin_no);
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", end_no);
        gen(node->then);
        printf("    jmp .Lbegin%d\n", begin_no);
        begin_no++;
        printf(".Lend%d:\n", end_no);
        end_no++;

        return;
    }
    if (node->kind == ND_FOR)
    {

        if (node->init != NULL)
        {
            gen(node->init);
        }
        printf(".Lbegin%d:\n", begin_no);
        if (node->cond != NULL)
        {
            gen(node->cond);
        }
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je .Lend%d\n", end_no);
        gen(node->then);
        if (node->step != NULL)
        {
            gen(node->step);
        }
        printf("    jmp .Lbegin%d\n", begin_no);
        printf(".Lend%d:\n", end_no);
        begin_no++;
        end_no++;

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
