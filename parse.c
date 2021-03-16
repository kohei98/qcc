#include "9cc.h"
Node *code[100];
LVar *locals;

Node *stmt();
Node *assign();
Node *primary();
Node *mul();
Node *expr();
Node *unary();
Node *equality();
Node *relational();
Node *add();
LVar *find_lvar(Token *tok);

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

// program = stmt*
void program() {
    int i = 0;
    while (!at_eof()) code[i++] = stmt();  //一つのstmtをcodeに順に格納していく
    code[i] = NULL;                        //末尾はNULL
}

Node *assign() {
    Node *node = equality();
    if (consume("=")) node = new_node(ND_ASSIGN, node, assign());
    return node;
}

// expr = assign
Node *expr() { return assign(); }

// stmt = expr ";"
//      | "return" expr ";"
Node *stmt() {
    Node *node;
    if (consume_tktype(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else
        node = expr();
    expect(";");
    return node;
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

// unary = ("+" | "-")? primary
Node *unary() {
    if (consume("+")) return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());  //-x を0-xで実現
    return primary();
}

// primary =num | ident | "(" expr ")"
Node *primary() {
    // '('が来たら"(" expr() ")"のハズ
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
        //ほかは数値のハズ
    }
    Token *tok = consume_tktype(TK_IDENT);  // identかどうかの判定
    if (tok) {
        //真のとき
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        LVar *lvar = find_lvar(tok);  // tokに対応する変数名を検索
        if (lvar) {                   //すでに存在するとき
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;  //新しい変数のオフセット
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    } else
        return new_node_num(expect_number());
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    }
    return NULL;
}