#include "9cc.h"

Node *primary();
Node *mul();
Node *expr();

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}
Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// primary = "(" expr ")" | num
Node *primary() {
    // '('が来たら"(" expr() ")"のハズ
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
        //ほかは数値のハズ
    } else
        return new_node_num(expect_number());
}

// mul = primary("*" primary | "/" primary) *
Node *mul() {
    Node *node = primary();

    for (;;) {
        if (consume('*'))
            node = new_node(ND_MUL, node, primary());
        else if (consume('/'))
            node = new_node(ND_DIV, node, primary());
        else
            return node;
    }
}

// expr = mul("+" mul | "-" mul) *
Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}