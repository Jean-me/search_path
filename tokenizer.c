#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#define SINGLE_QUOTE_CHAR '\''
#define ERROR_TOKENIZE 258

// typedef struct s_token t_token;
bool syntax_error = false;

enum e_token_kind
{
    TK_WORD,
    TK_RESERVED,
    TK_OP,
    TK_EOF,
};
typedef enum e_token_kind t_token_kind;

struct s_token
{
    char *word;
    t_token_kind kind;
    struct s_token *next;
} typedef t_token;

void fatal_error(const char *msg)
{
    perror(msg);
    exit(1);
}

void assert_error(const char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

void tokenize_error(const char *location, char **rest, char *line)
{
    syntax_error = true;
    dprintf(STDERR_FILENO, "minishell: syntax error near %s\n", location);
    while (*line && *line != '\n')
        line++;
    *rest = line;
}

t_token *new_token(char *word, t_token_kind kind)
{
    t_token *tok;

    tok = calloc(1, sizeof(*tok));
    if (tok == NULL)
        fatal_error("calloc");
    tok->word = word;
    tok->kind = kind;
    return (tok);
}

bool is_blank(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

bool consume_blank(char **rest, char *line)
{
    if (is_blank(*line))
    {
        while (*line && is_blank(*line))
            line++;
        *rest = line;
        return (true);
    }
    *rest = line;
    return (false);
}

bool startswith(const char *s, const char *keyword)
{
    return (memcmp(s, keyword, strlen(keyword)) == 0); // 一致したら０
}

bool is_operator(const char *s)
{
    static char *const operators[] = {"||", "&&", "&", ";;", ";", "(", ")", "|"};
    size_t i;
    i = 0;

    while (i < sizeof(operators) / sizeof(*operators))
    {
        if (startswith(s, operators[i]))
            return (true);
        i++;
    }
    return (false);
}

bool is_metacharacter(char c)
{
    return (c && strchr("|&;()<> \t\n", c));
}

bool is_word(const char *s)
{
    return (*s && !is_metacharacter(*s));
}

bool is_quote(char c)
{
    return (c == '\'' || c == '"');
}

t_token *operator(char **rest, char *line)
{
    static char *const operators[] = {"||", "&&", "&", ";;", ";", "(", ")", "|"};
    size_t i;
    char *op;
    i = 0;

    while (i < sizeof(operators) / sizeof(*operators))
    {
        if (startswith(line, operators[i]))
        {
            op = strdup(operators[i]);
            if (op == NULL)
                fatal_error("strdup");
            *rest = line + strlen(op);
            return (new_token(op, TK_OP));
        }
        i++;
    }
    assert_error("Unexpected operator");
}

t_token *word(char **rest, char *line)
{
    const char *start = line;
    char *word;

    while (*line && !is_metacharacter(*line))
        line++; // ただの文字
    word = strndup(start, line - start);
    if (word == NULL)
        fatal_error("strndup");
    *rest = line;
    return (new_token(word, TK_WORD));
}

t_token *quated_word(char **rest, char *line) //
{
    char quote_char = *line; // is_quate関数により、最初の一文字は"か`のどっちか
    const char *start = line;
    line++;

    while (*line && *line != quote_char)
        line++;
    if (*line != quote_char)
    {
        tokenize_error("Unclosed quote", rest, line);
        return (new_token(NULL, TK_EOF)); // ダミートークンを返す
    }
    line++; // 終了クォートをスキップ

    char *word = strndup(start, line - start);
    if (word == NULL)
        fatal_error("strndup");
    *rest = line;
    return (new_token(word, TK_WORD));
}

t_token *tokenize(char *line)
{
    t_token head;
    t_token *tok;

    syntax_error = false;
    head.next = NULL;
    tok = &head;
    while (*line)
    {
        if (consume_blank(&line, line))
            continue;
        else if (is_quote(*line))
            tok = tok->next = quated_word(&line, line);
        else if (is_operator(line))
            tok = tok->next = operator(&line, line);
        else if (is_word(line))
            tok = tok->next = word(&line, line);
        else
            tokenize_error("Unexpected Token", &line, line);
    }
    tok->next = new_token(NULL, TK_EOF);
    return (head.next);
}

int exec()
{
    char *args[] = {"/bin/pwd", NULL};

    extern char **environ;

    if (execve("/bin/pwd", args, environ) == -1)
    {
        perror("execve failed");
        return (1);
    }
    return 0; // Child process exits after execve
}

char **token_list_to_argv(t_token *tok)
{
    int count = 0;
    t_token *current = tok;
    char *argv;
    // トークン数をカウント（TK_WORDのみ、EOF除く）
    while (current && current->kind != TK_EOF)
    {
        if (current->kind == TK_WORD)
            count++;
        current = current->next;
    }
    // 配列確保（＋１はNULL用
    char **argv = malloc(sizeof(char *) * (count + 1));
    if (!argv)
        fatal_error("malloc");
    // トークンから文字列をコピー
    int i = 0;
    current = tok;
    while (current && current->kind != TK_EOF)
    {
        if (current->kind == TK_WORD)
        {
            argv[i] = strdup(current->word);
            if (!argv[i])
                fatal_error("strdup");
            i++;
        }
        current = current->next;
    }
    argv[i] = NULL;
    return argv;
}

// argv配列のメモリ解放
void free_argv(char **argv)
{
    if (!argv)
        return;

    for (int i = 0; argv[i]; i++)
        free(argv[i]);
    free(argv);
}

void interpret(char *line, int *stat_loc)
{
    t_token *tok = tokenize(line);
    t_token *current = tok;

    if (tok->kind == TK_EOF)
        *stat_loc = 0; // 正常
    else if (syntax_error)
        *stat_loc = ERROR_TOKENIZE;
    else
    {
        // 実際のコマンド実行ロジックが必要
        exec();
        // ここでいったんprintfさせたい
        while (current)
        {
            printf("Token: [%s], Kind: ", current->word ? current->word : "NULL");

            switch (current->kind)
            {
            case TK_WORD:
                printf("WORD\n");
                break;
            case TK_RESERVED:
                printf("RESERVED\n");
                break;
            case TK_OP:
                printf("OPERATOR\n");
                break;
            case TK_EOF:
                printf("EOF\n");
                break;
            default:
                printf("UNKNOWN\n");
                break;
            }

            current = current->next;
        }
        // トークンをargv配列に変換
        char **argv = token_list_to_argv(tok);
        // コマンドを実行
        //.....
        // argv配列の解放
        free_argv(argv);
        *stat_loc = 0; // 正常
    }
    while (tok) // フリー
    {
        t_token *temp = tok;
        tok = tok->next;
        free(temp->word);
        free(temp);
    }
}

int main(int argc, char *argv[])
{
    int status = 0;

    char input[1024] = "";
    int i = 1;
    if (argc < 2)
    {
        // 標準入力から読み取り
        printf("Enter command: ");
        if (!fgets(input, sizeof(input), stdin))
            return (1);
        input[strcspn(input, "\n")] = '\0'; // 改行削除
        // fprintf(stderr, "Usage: %s <input_string>\n", argv[0]);
        // return (1);
    }
    else
    {
        while (i < argc)
        {
            if (i > 1)
                strcat(input, " ");
            strcat(input, argv[i]);
            i++;
        }
    }
    interpret(input, &status);
    printf("Exit status: %d\n", status);

    return 0;
}

// int main(int argc, char *argv[])
// {

//     char input[1024] = "";
//     int i = 1;
//     if (argc < 2)
//     {
//         fprintf(stderr, "Usage: %s <input_string>\n", argv[0]);
//         return (1);
//     }

//     while (i < argc)
//     {
//         if (i > 1)
//             strcat(input, " ");
//         strcat(input, argv[i]);
//         i++;
//     }
//     // ここで、interpret関数をよぶ。下記の作業をやる
//     t_token *tokens = tokenize(input);
//     t_token *current = tokens;

//     // トークンを順番に表示
//     while (current)
//     {
//         printf("Token: [%s], Kind: ", current->word ? current->word : "NULL");

//         switch (current->kind)
//         {
//         case TK_WORD:
//             printf("WORD\n");
//             break;
//         case TK_RESERVED:
//             printf("RESERVED\n");
//             break;
//         case TK_OP:
//             printf("OPERATOR\n");
//             break;
//         case TK_EOF:
//             printf("EOF\n");
//             break;
//         default:
//             printf("UNKNOWN\n");
//             break;
//         }

//         current = current->next;
//     }
//     // メモリ解放
//     while (tokens)
//     {
//         t_token *temp = tokens;
//         tokens = tokens->next;
//         free(temp->word);
//         free(temp);
//     }
//     return (0);
// }