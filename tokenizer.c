#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define SINGLE_QUOTE_CHAR '\''
#define ERROR_TOKENIZE 258
#define PATH_MAX 4096

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

struct s_node
{
    t_token *tok;        // トークンのリスト
    char **argv;         // コマンドの引数
    struct s_node *next; // 次のノードへのポインタ
} typedef t_node;

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
    return NULL; // ここには到達しないはず
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
    char quote_char = *line; // is_quate関数により、最初の一文字は"か'のどっちか
    const char *start;
    line++;       // 開始クォートをスキップ
    start = line; // クォート内の文字列の開始位置

    while (*line && *line != quote_char)
        line++;
    if (*line != quote_char)
    {
        tokenize_error("Unclosed quote", rest, line);
        return (new_token(NULL, TK_EOF)); // ダミートークンを返す
    }

    // クォート内の内容のみを文字列として保存
    char *word = strndup(start, line - start);
    if (word == NULL)
        fatal_error("strndup");

    line++; // 終了クォートをスキップ
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

size_t ft_strlen(const char *str)
{
    size_t length;

    length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return (length);
}

size_t ft_strnlen(const char *str, size_t n)
{
    size_t length;

    length = 0;
    while (length < n && str[length] != '\0')
    {
        length++;
    }
    return (length);
}

size_t ft_strlcat(char *dst, const char *src, size_t size)
{
    size_t dst_len;
    size_t src_len;
    size_t total_len;
    size_t i;

    dst_len = ft_strnlen(dst, size);
    src_len = ft_strlen(src);
    total_len = dst_len + src_len;
    i = 0;
    if (size <= dst_len)
        return (src_len + size);
    while (dst_len + 1 < size && src[i] != '\0')
    {
        dst[dst_len] = src[i];
        dst_len++;
        i++;
    }
    dst[dst_len] = '\0';
    return (total_len);
}

void ft_bzero(void *b, size_t len)
{
    unsigned char *p;
    size_t i;

    p = b;
    i = 0;
    while (i < len)
    {
        p[i] = 0;
        i++;
    }
}

char *ft_strchr(const char *s, int c)
{
    char cchar;

    cchar = (char)c;
    while (*s != '\0')
    {
        if (*s == cchar)
            return ((char *)s);
        s++;
    }
    if (cchar == '\0')
        return ((char *)s);
    else
        return (NULL);
}

char *ft_strdup(const char *string)
{
    char *dst;
    size_t len;
    size_t i;

    i = 0;
    len = ft_strlen(string);
    dst = (char *)malloc((len + 1) * sizeof(char));
    if (!dst)
        return (NULL);
    while (string[i])
    {
        dst[i] = string[i];
        i++;
    }
    dst[i] = '\0';
    return (dst);
}

char *search_path(const char *filename)
{
    char path[PATH_MAX];
    char *value;
    char *end;

    // PATHが設定されていない場合
    value = getenv("PATH");
    while (*value)
    {
        ft_bzero(path, PATH_MAX);
        end = ft_strchr(value, ':');
        if (end)
            strncpy(path, value, end - value);
        else
            strncpy(path, value, PATH_MAX);
        ft_strlcat(path, "/", PATH_MAX);
        ft_strlcat(path, filename, PATH_MAX);
        if (access(path, X_OK) == 0)
        {
            char *dup;

            dup = ft_strdup(path);
            if (dup == NULL)
                fatal_error("strdup");
            return (dup);
        }
        if (end == NULL)
            return (NULL);
        value = end + 1;
    }
    return (NULL);
}

char **token_list_to_argv(t_token *tok) // ここの*currentをtokをそのまま使用せずnode->argvに変更する
{
    int count = 0;
    t_token *current = tok;

    // トークン数をカウント（TK_WORDのみ、EOF除く）
    while (current && current->kind != TK_EOF)
    {
        if (current->kind == TK_WORD)
            count++;
        current = current->next;
    }
    // 配列確保（＋１はNULL用）
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

t_node *parse(t_token *tok)
{
    t_node *head = NULL;
    t_node *current = NULL;

    while (tok)
    {
        // 新しいノードを作成
        t_node *new_node = malloc(sizeof(t_node));
        if (!new_node)
            fatal_error("malloc");
        new_node->tok = tok;
        new_node->argv = token_list_to_argv(tok);
        new_node->next = NULL;

        // リストに追加
        if (!head)
            head = new_node;
        else
            current->next = new_node;
        current = new_node;

        // 次のトークンへ
        tok = tok->next;
    }
    return head;
}

void interpret(char *line, int *stat_loc) // ここ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
{
    t_token *tok = tokenize(line);
    t_node *node = parse(tok); // トークンをパースして構文木を生成
    t_token *current = tok;

    if (tok->kind == TK_EOF)
        *stat_loc = 0; // 正常
    else if (syntax_error)
        *stat_loc = ERROR_TOKENIZE;
    else
    {
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
        // トークンをargv配列に変換(ここでnode->argvを使用する,トークンのグループ分け済ませておく)
        char **argv = token_list_to_argv(tok);
        // コマンドのパスを探す
        char *path = search_path(argv[0]);
        if (path)
        {
            printf("Found executable at: %s\n", path);
            // forkして子プロセスで実行
            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork failed");
                free(path);
                *stat_loc = 1;
            }
            else if (pid == 0)
            {
                // 子プロセス：execveで実行
                execve(path, argv, NULL);
                // execveが失敗した場合のみここに到達
                perror("execve failed");
                exit(1);
            }
            else
            {
                // 親プロセス：子プロセスの終了を待つ
                int child_status;
                waitpid(pid, &child_status, 0);
                *stat_loc = WEXITSTATUS(child_status);
            }
            free(path);
        }
        else
        {
            printf("Executable '%s' not found in PATH.\n", argv[0]);
            *stat_loc = 127; // command not found
        }
        // argv配列の解放
        free_argv(argv);
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
