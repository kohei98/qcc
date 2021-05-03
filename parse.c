#include "qcc.h"
Node *code[100];
LVar *locals[100];
int func_no = 0;
Node *func();
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

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}
Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// program = func*
void program()
{
    // int i = 0;
    while (!at_eof())
    {
        locals[func_no] = localsinit();
        code[func_no] = func(); //一つのfuncをcodeに順に格納していく
        func_no++;
    }
    code[func_no] = NULL; //末尾はNULL
    locals[func_no] = NULL;
}
//関数定義 引数の処理が終わったらstmt()を呼ぶ
// func = "("ident*")" stmt*
Node *func()
{
    Node *node = calloc(1, sizeof(Node));
    Token *tok = consume_tktype(TK_IDENT); // identかどうかの判定
    if (tok)
    {

        Node *node = calloc(1, sizeof(Node));
        node->arg_number = 0;
        if (preview("("))
        {
            node->func_name = calloc(tok->len, sizeof(char));
            memcpy(node->func_name, tok->str, tok->len);

            node->kind = ND_DEF_FUNC;

            while (!preview(")"))
            {
                Token *tok = consume_tktype(TK_IDENT); // identかどうかの判定
                if (tok)
                {
                    node->arg_number++;
                    // node->kind = ND_LVAR;
                    LVar *lvar = find_lvar(tok); // tokに対応する変数名を検索
                    if (lvar)
                    { //すでに存在するとき
                        node->offset = lvar->offset;
                    }
                    else
                    {
                        lvar = calloc(1, sizeof(LVar));
                        lvar->next = locals[func_no];
                        lvar->name = tok->str;
                        lvar->len = tok->len;
                        lvar->offset = locals[func_no]->offset + 8; //新しい変数のオフセット
                        node->offset = lvar->offset;
                        locals[func_no] = lvar;
                    }
                    preview(",");
                }
            }
        }
        // expect("{");
        // while (!preview("}"))
        node->then = stmt();
        // expect("}");
        return node;
    }
    return NULL;
}

Node *assign()
{
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

// expr = assign
Node *expr() { return assign(); }

// stmt = expr? ";"
//      | "{" stmt* "}"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr?";")stmt
Node *stmt()
{
    Node *node;
    if (preview("{"))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        int i = 0;
        while (!preview("}"))
        {
            node->next_blockstmt[i] = stmt();
            i++;
        }
        node->next_blockstmt[i] = NULL;
        return node;
    }
    if (consume_tktype(TK_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
    }
    //if
    else if (consume_tktype(TK_IF))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        consume("(");
        node->cond = expr();
        consume(")");
        node->then = stmt();
        //else
        if (prev_tktype(TK_ELSE))
        {
            // consume("else");
            node->els = stmt();
        }
        return node;
    }
    // while
    else if (consume_tktype(TK_WHILE))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        consume("(");
        node->cond = expr();
        consume(")");
        node->then = stmt();
        return node;
    }

    else if (consume_tktype(TK_FOR))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        consume("(");
        if (!preview(";"))
        {
            node->init = expr();
            consume(";");
        }
        if (!preview(";"))
        {
            node->cond = expr();
            consume(";");
        }
        if (!preview(")"))
        {
            node->step = expr();
            consume(")");
        }
        node->then = stmt();
        return node;
    }
    else
    {
        if (!preview(";"))
        {
            node = expr();
            consume(";");
            return node;
        }
        else
        {
            node = calloc(1, sizeof(Node));
            node->kind = ND_BLOCK;
        }
    }
    return node;
}

// equality = relational("==" relational | "!=" relational) *
Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
    Node *node = add();
    for (;;)
    {
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
Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary("*" unary | "/" unary) *
Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ("+" | "-")? primary
Node *unary()
{
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary()); //-x を0-xで実現
    return primary();
}

// primary =num
//          | ident ("("")")?
//          | "(" expr ")"
Node *primary()
{
    // '('が来たら"(" expr() ")"のハズ
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
        //ほかは数値のハズ
    }
    Token *tok = consume_tktype(TK_IDENT); // identかどうかの判定
    if (tok)
    {
        Node *node = calloc(1, sizeof(Node));
        //変数or関数
        if (preview("("))
        {
            node->func_name = calloc(tok->len, sizeof(char));
            memcpy(node->func_name, tok->str, tok->len);
            node->kind = ND_CALL_FUNC;

            int i = 0;
            while (!preview(")"))
            {
                node->argument[i] = add();
                preview(",");
                i++;
            }
            node->argument[i] = NULL;
            // return node;
        }
        else
        {
            node->kind = ND_LVAR;
        }
        LVar *lvar = find_lvar(tok); // tokに対応する変数名を検索
        if (lvar)
        { //すでに存在するとき
            node->offset = lvar->offset;
        }
        else
        {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals[func_no];
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals[func_no]->offset + 8; //新しい変数のオフセット
            node->offset = lvar->offset;
            locals[func_no] = lvar;
        }

        return node;
    }
    else
        return new_node_num(expect_number());
}

LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals[func_no]; var; var = var->next)
    {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    }
    return NULL;
}
