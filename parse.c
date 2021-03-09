#include "9cc.h"

Node *primary();
Node *mul();
Node *expr();
Node *unary();
Node *equality();
Node *relational();
Node *add();

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
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
        //ほかは数値のハズ
    } else
        return new_node_num(expect_number());
}

// mul = unary("*" unary | "/" unary) *
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// expr = equality
Node *expr() { return equality(); }
// unary = ("+" | "-")? primary
Node *unary() {
    if (consume("+")) return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());  //-x を0-xで実現
    return primary();
}

// equality = relational("==" relational | "!=" relational) *
Node *equality() {
    Node *node = relational();
    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
    Node *node = add();
    for (;;) {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}
// add = mul ("+" mul | "-" mul)*
Node *add() {
    Node *node = mul();
    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}