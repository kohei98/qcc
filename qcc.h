#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Token Token; // 別名をつけることで,以降structが不要になる．
typedef struct Node Node;   // structの省略
typedef struct LVar LVar;

//ローカル変数の型
struct LVar
{
    LVar *next; //次の変数かNULL
    char *name; //変数の名前
    int len;    //名前の長さ
    int offset; // RBPからのオフセット
};
//ローカル変数のリスト
extern LVar *locals;

typedef enum
{
    TK_RESERVED, //記号
    TK_IDENT,    //識別子
    TK_NUM,      //整数トークン
    TK_RETURN,   // returnトークン
    TK_IF,       // ifトークン
    TK_ELSE,     // elseトークン
    TK_WHILE,    // whileトークン
    TK_FOR,      //forトークン
    TK_EOF,      //入力の終わりを示すトークン
} TokenKind;

//トークン型
struct Token
{
    TokenKind kind;
    Token *next; //次のトークン
    int val;     // kindがTK_NUMの場合，その数値
    char *str;   //トークン文字列
    int len;     //トークンの長さ
};

typedef enum
{
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_NUM,    //　整数
    ND_LVAR,   //ローカル変数
    ND_ASSIGN, //　=
    ND_RETURN, // return
    ND_IF,     // if
    ND_ELSE,   // else
    ND_WHILE,  // while
    ND_FOR,    // for
    ND_BLOCK,
} NodeKind;

extern Node *code[100];

struct Node
{
    NodeKind kind; //ノードの型
    Node *lhs;     //左辺
    Node *rhs;     //右辺
    int val;       // kindが数値のとき
    int offset;    // kindがND_LVARの場合のみ使う

    //if文のときとかに使う
    Node *cond; //
    Node *then; // condが真のときに実行する
    Node *els;
    Node *init;                // for文の初期値
    Node *step;                // forのstep
    Node *next_blockstmt[100]; //ND_BLOCKのときに使うstmtのベクタ 実装をサボったので，そのうちポインタを使って動的に確保できたらよい
};

extern char *user_input; //入力の先頭
extern Token *token;     //今見ているトークン

// parse.c
void program();

// tokenize.c
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char *op);
void expect(char *op);
bool prev(char *op);
int expect_number();
bool at_eof();
Token *tokenize(char *p);
Token *consume_tktype(TokenKind TK_KIND);
Token *prev_tktype(TokenKind TK_KIND);

// codegen.c
extern int begin_no;
extern int end_no;
extern int else_no;

void gen(Node *node);
