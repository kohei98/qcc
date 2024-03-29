#include "qcc.h"

char *user_input; //入力の先頭
Token *token;

static bool is_alpha(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c) { return is_alpha(c) || ('0' <= c && c <= '9'); }

// エラーを報告する関数の改良版
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input); //入力を表示
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//エラーを報告する関数
// printfと同じ引数を取る
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}
//次のトークンが期待している記号のときはトークンを一つ読み進めて
//真を返す
bool consume(char *op)
{
    if (token->kind != TK_RESERVED ||
        strlen(op) !=
            token->len || //次のトークンが期待している記号の長さでない時
        memcmp(token->str, op, token->len))
    { // 長さが同じでも違う記号の時
        return false;
    }
    token = token->next;
    return true;
}
// 次のトークンが期待している記号のときはトークンを一つ読み進めて
//それ以外はエラーを返す
void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%s'ではありません", op);
    token = token->next;
}

// 次のトークンが期待している記号のときはトークンを一つ読み進めて
//それ以外はfalseを返して何もしない
bool preview(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}
Token *consume_ident()
{
    if (token->kind != TK_IDENT)
        return NULL;
    Token *t = token;
    token = token->next; //一つすすめる
    return t;
}

Token *consume_tktype(TokenKind TK_KIND)
{
    if (token->kind != TK_KIND)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

Token *prev_tktype(TokenKind TK_KIND)
{
    if (token->kind != TK_KIND)
    {
        return NULL;
    }
    Token *t = token;
    token = token->next;
    return t;
}

//次のトークンが数値の場合，トークンを一つ読み進めてその数値を返す
//それ以外はエラーを返す
int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
} // pの接頭辞とqが一致しているかを判定

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
            startswith(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr(";+-*/()<>={},", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // returnかどうかを調べる
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p += 6, 6);
            continue;
        }

        //ifかどうかを調べる
        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2]))
        {
            cur = new_token(TK_IF, cur, p += 2, 2);
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4]))
        {
            cur = new_token(TK_ELSE, cur, p += 4, 4);
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5]))
        {
            cur = new_token(TK_WHILE, cur, p += 5, 5);
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3]))
        {
            cur = new_token(TK_FOR, cur, p += 3, 3);
            continue;
        }
        // 識別子
        if (is_alpha(*p))
        {
            char *q = p++;
            while (is_alnum(*p))
                p++;
            cur = new_token(TK_IDENT, cur, q, p - q);
            continue;
        }

        if (isdigit(*p))
        { //整数のとき
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10); //ポインタのポインタを渡して返り値にするアレ
            cur->len = p - q;
            // printf("%s\n", cur->str);
            continue;
        }
        error("トークナイズできません");
    }
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
