#include "9cc.h"
void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");  // lhsの値
    printf("    pop rax\n");  // rhsの値
    switch (node->kind) {
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
    }
    printf("    push rax\n");
}