#ifndef MINISHELL_P_H
#define MINISHELL_P_H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h> // ファイル操作用のフラグ定義

#define SINGLE_QUOTE_CHAR '\''
#define ERROR_TOKENIZE 258
#define PATH_MAX 4096

// Token kinds
typedef enum e_token_kind
{
    TK_WORD,
    TK_RESERVED,
    TK_OP,
    TK_REDIRECT_IN,     // < 入力リダイレクション
    TK_REDIRECT_OUT,    // > 出力リダイレクション
    TK_REDIRECT_APPEND, // >> 追記リダイレクション
    TK_EOF,
} t_token_kind;

// Node kinds
typedef enum e_node_kind
{
    ND_SIMPLE_CMD,
    ND_PIPE,            // パイプ |
    ND_AND,             // AND演算子 &&
    ND_OR,              // OR演算子 ||
    ND_SEQUENCE,        // セミコロン ;
    ND_REDIRECT_IN,     // < 入力リダイレクション
    ND_REDIRECT_OUT,    // > 出力リダイレクション
    ND_REDIRECT_APPEND, // >> 追記リダイレクション
} t_node_kind;

// Token structure
typedef struct s_token
{
    char *word;
    t_token_kind kind;
    struct s_token *next;
} t_token;

// Redirect structure
typedef struct s_redirect
{
    int fd;           // ファイルディスクリプタ（標準入力:0, 標準出力:1, 標準エラー:2）
    char *filename;   // リダイレクト先ファイル名
    t_node_kind type; // リダイレクション種類
    struct s_redirect *next;
} t_redirect;

// Node structure
typedef struct s_node
{
    t_token *args;
    t_node_kind kind;
    struct s_node *left;
    struct s_node *right;
    struct s_node *next; // 次のノード（シーケンスの場合）
    t_redirect *redirects;
} t_node;

// void fatal_error(const char *msg);
// bool at_eof(t_token *tok);
// t_node *new_node(t_node_kind kind);
// t_token *tokdup(t_token *tok);
// void append_tok(t_token **tokens, t_token *tok);
// void assert_error(const char *msg);
// void tokenize_error(const char *location, char **rest, char *line);
// t_token *new_token(char *word, t_token_kind kind);
// t_token *tokenize(char *line);
// t_node *parse(t_token *tok);
// void interpret(char *line, int *stat_loc);

#endif