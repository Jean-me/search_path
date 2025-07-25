#include "minishell_p.h"

bool syntax_error = false;

void fatal_error(const char *msg)
{
    perror(msg);
    exit(1);
}

bool at_eof(t_token *tok) // トークンがkind:TK_EOFかどうかを確認
{
    return (tok->kind == TK_EOF);
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

t_node *new_node(t_node_kind kind) // nodeの種類を指定して新しいノードを作成
{
    t_node *node;

    node = calloc(1, sizeof(*node));
    if (node == NULL)
        fatal_error("calloc");
    node->kind = kind;
    return (node);
}

t_token *tokdup(t_token *tok)
{
    char *word;

    word = strdup(tok->word);
    if (word == NULL)
        fatal_error("strdup");
    return (new_token(word, tok->kind));
}

void append_tok(t_token **tokens, t_token *tok)
{
    if (*tokens == NULL)
    {
        *tokens = tok;
        return;
    }
    append_tok(&(*tokens)->next, tok);
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
    static char *const operators[] = {"||", "&&", ">>", "&", ";;", ";", "(", ")", "|", ">", "<"};
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
    static char *const operators[] = {"||", "&&", ">>", "&", ";;", ";", "(", ")", "|", ">", "<"};
    size_t i;
    char *op;
    t_token_kind kind;
    i = 0;

    while (i < sizeof(operators) / sizeof(*operators))
    {
        if (startswith(line, operators[i]))
        {
            op = strdup(operators[i]);
            if (op == NULL)
                fatal_error("strdup");
            *rest = line + strlen(op);

            // リダイレクション演算子の場合は適切なトークンタイプを設定
            if (strcmp(op, ">>") == 0)
                kind = TK_REDIRECT_APPEND;
            else if (strcmp(op, ">") == 0)
                kind = TK_REDIRECT_OUT;
            else if (strcmp(op, "<") == 0)
                kind = TK_REDIRECT_IN;
            else
                kind = TK_OP; // その他の演算子はTK_OPとして扱う

            return (new_token(op, kind));
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

// リダイレクションノードを作成する関数
t_redirect *new_redirect(t_node_kind type, char *filename, int fd)
{
    t_redirect *redirect = malloc(sizeof(t_redirect));
    if (!redirect)
        fatal_error("malloc");

    redirect->type = type;
    redirect->filename = strdup(filename);
    if (!redirect->filename)
        fatal_error("strdup");
    redirect->next = NULL;
    redirect->fd = fd;

    return redirect;
}

// リダイレクションをノードに追加する関数
void append_redirect(t_node *node, t_redirect *redirect)
{
    if (!node->redirects)
    {
        node->redirects = redirect;
        return;
    }

    t_redirect *current = node->redirects;
    while (current->next)
        current = current->next;
    current->next = redirect;
}

// 単純コマンドのみをパースする関数
t_node *parse_simple_command(t_token **tok_ptr)
{
    t_node *node = new_node(ND_SIMPLE_CMD);
    t_token *tok = *tok_ptr;

    while (tok && !at_eof(tok))
    {
        if (tok->kind == TK_WORD)
        {
            append_tok(&node->args, tokdup(tok));
            tok = tok->next;
        }
        else if (tok->kind == TK_REDIRECT_IN || tok->kind == TK_REDIRECT_OUT || tok->kind == TK_REDIRECT_APPEND)
        {
            // リダイレクション演算子の処理
            t_node_kind redirect_type;
            int default_fd;

            if (tok->kind == TK_REDIRECT_IN)
            {
                redirect_type = ND_REDIRECT_IN;
                default_fd = 0; // 標準入力
            }
            else if (tok->kind == TK_REDIRECT_OUT)
            {
                redirect_type = ND_REDIRECT_OUT;
                default_fd = 1; // 標準出力
            }
            else
            {
                redirect_type = ND_REDIRECT_APPEND;
                default_fd = 1; // 標準出力
            }

            tok = tok->next; // リダイレクション演算子をスキップ

            // 次のトークンがファイル名でなければエラー
            if (!tok || tok->kind != TK_WORD)
            {
                fprintf(stderr, "Error: Expected filename after redirection\n");
                *tok_ptr = tok;
                return node;
            }

            // リダイレクションを追加
            t_redirect *redirect = new_redirect(redirect_type, tok->word, default_fd);
            append_redirect(node, redirect);

            tok = tok->next; // ファイル名をスキップ
        }
        else
        {
            break; // その他のトークンは終了
        }
    }
    *tok_ptr = tok;
    return node;
}

// 演算子を含む複合コマンドをパースする関数（現在パイプのみ）
t_node *parse(t_token *tok)
{
    t_node *left, *right, *op_node; // op_nodeはオペレーションのポインタ

    // 左の、最初の単純コマンド：例　echo "hello"
    left = parse_simple_command(&tok);

    // オペレーションを探してる : tokトークンが、pipeなどのTK_OPにあたった場合
    while (tok && !at_eof(tok) && tok->kind == TK_OP)
    {
        printf("Info: Processing operator '%s'\n", tok->word);

        // オペレーションの種類に応じて、オペレーションノードを作成
        if (strcmp(tok->word, "|") == 0)
            op_node = new_node(ND_PIPE);
        else
        {
            printf("Warning: Unsupported operator '%s'\n", tok->word);
            tok = tok->next;
            continue;
        }
        // オペレーションの次のトークンに進む
        tok = tok->next;
        // 右側のコマンド　例：wc -l
        right = parse_simple_command(&tok);

        // ↑で作ったパイプの左右にあるコマンドノードを、それぞれパイプの左と右に設定（echo "he" | wc -l） (op_node.left, op_node, op_node.right)
        op_node->left = left;
        op_node->right = right;
        op_node->args = NULL;

        left = op_node;
    }
    return left;
}

// デバッグ用関数：nodeの内容を表示
void print_node_debug(t_node *node)
{
    if (!node)
    {
        printf("Node: NULL\n");
        return;
    }

    printf("=== NODE DEBUG ===\n");
    printf("Node kind: "); // 真ん中のノード
    switch (node->kind)
    {
    case ND_SIMPLE_CMD:
        printf("SIMPLE_CMD\n");
        break;
    case ND_PIPE: // ← この部分が抜けていました！
        printf("PIPE\n");
        break;
    default:
        printf("UNKNOWN (%d)\n", node->kind);
        break;
    }

    if (node->kind == ND_SIMPLE_CMD)
    {

        printf("Arguments in this node:\n");
        t_token *arg = node->args;
        int arg_count = 0;
        while (arg)
        {
            printf("  [%d] %s (kind: %s)\n",
                   arg_count,
                   arg->word ? arg->word : "NULL",
                   arg->kind == TK_WORD ? "WORD" : arg->kind == TK_OP ? "OP"
                                               : arg->kind == TK_EOF  ? "EOF"
                                                                      : "UNKNOWN");
            arg = arg->next;
            arg_count++;
        }
        printf("Total arguments: %d\n", arg_count);

        // リダイレクション情報を表示
        if (node->redirects)
        {
            printf("Redirections:\n");
            t_redirect *redirect = node->redirects;
            int redirect_count = 0;
            while (redirect)
            {
                const char *type_str;
                switch (redirect->type)
                {
                case ND_REDIRECT_IN:
                    type_str = "INPUT <";
                    break;
                case ND_REDIRECT_OUT:
                    type_str = "OUTPUT >";
                    break;
                case ND_REDIRECT_APPEND:
                    type_str = "APPEND >>";
                    break;
                default:
                    type_str = "UNKNOWN";
                    break;
                }
                printf("  [%d] %s %s (fd: %d)\n", redirect_count, type_str, redirect->filename, redirect->fd);
                redirect = redirect->next;
                redirect_count++;
            }
        }
    }
    else
    {
        printf("This is an operator node (no direct arguments)\n");

        if (node->left)
        {
            printf("=== LEFT CHILD ===\n");
            print_node_debug(node->left);
        }

        if (node->right)
        {
            printf("=== RIGHT CHILD ===\n");
            print_node_debug(node->right);
        }
    }
    if (node->next)
    {
        printf("--- Next Node ---\n");
        print_node_debug(node->next); // 再帰的に次のノードも表示
    }

    printf("==================\n\n");
}

// パイプを実行する関数
void execute_pipe(t_node *pipe_node, int *stat_loc)
{
    int pipefd[2];
    pid_t pid1, pid2;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        *stat_loc = 1;
        return;
    }

    // パイプの左側のコマンド
    pid1 = fork();
    if (pid1 == -1)
    {
        perror("fork");
        *stat_loc = 1;
        return;
    }
    else if (pid1 == 0)
    {
        // 子プロセス１：左側のコマンドを実行
        close(pipefd[0]);               // 読み取り側を閉じる
        dup2(pipefd[1], STDOUT_FILENO); // 標準出力をパイプの書き込み側に
        close(pipefd[1]);

        // 左側のコマンドを実行...1.pipe左側のトークンをargvに変換
        char **left_argv = token_list_to_argv(pipe_node->left->args);
        char *left_path = search_path(left_argv[0]);
        if (left_path)
        {
            execve(left_path, left_argv, NULL);
            perror("execve failed");
        }
        else
            printf("Command not found: %s\n", left_argv[0]);
        exit(1);
    }

    // 右側のコマンド（パイプの出力側）
    pid2 = fork();
    if (pid2 == -1)
    {
        perror("fork");
        *stat_loc = 1;
        return;
    }
    else if (pid2 == 0)
    {
        // 子プロセス２：右側のコマンドを実行
        close(pipefd[1]);              // 書き込み側を閉じる
        dup2(pipefd[0], STDIN_FILENO); // 標準入力をパイプの読み取り側に
        close(pipefd[0]);

        // 右側のコマンドを実行
        char **right_argv = token_list_to_argv(pipe_node->right->args);
        char *right_path = search_path(right_argv[0]);
        if (right_path)
        {
            execve(right_path, right_argv, NULL);
            perror("execve failed");
        }
        else
            printf("Command not found: %s\n", right_argv[0]);
        exit(1);
    }

    // 親プロセス：パイプ閉じて子プロセスを待つ
    close(pipefd[0]);
    close(pipefd[1]);

    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    *stat_loc = WEXITSTATUS(status2); // 右側のコマンドの終了ステータス
}

// リダイレクションを設定する関数
int setup_redirections(t_redirect *redirects)
{
    t_redirect *redirect = redirects;

    while (redirect)
    {
        int fd;

        switch (redirect->type)
        {
        case ND_REDIRECT_IN:
            fd = open(redirect->filename, O_RDONLY);
            if (fd == -1)
            {
                perror(redirect->filename);
                return -1;
            }
            if (dup2(fd, redirect->fd) == -1) // redirect->fdを使用
            {
                perror("dup2");
                close(fd);
                return -1;
            }
            close(fd);
            break;

        case ND_REDIRECT_OUT:
            fd = open(redirect->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                perror(redirect->filename);
                return -1;
            }
            if (dup2(fd, redirect->fd) == -1) // redirect->fdを使用
            {
                perror("dup2");
                close(fd);
                return -1;
            }
            close(fd);
            break;

        case ND_REDIRECT_APPEND:
            fd = open(redirect->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1)
            {
                perror(redirect->filename);
                return -1;
            }
            if (dup2(fd, redirect->fd) == -1) // redirect->fdを使用
            {
                perror("dup2");
                close(fd);
                return -1;
            }
            close(fd);
            break;

        default:
            fprintf(stderr, "Unknown redirection type\n");
            return -1;
        }

        redirect = redirect->next;
    }

    return 0;
}

// ノードを実行する関数
void execute_node(t_node *node, int *stat_loc)
{
    if (!node)
    {
        *stat_loc = 0;
        return;
    }
    switch (node->kind)
    {
    case ND_SIMPLE_CMD:
    {
        char **argv = token_list_to_argv(node->args);
        char *path = search_path(argv[0]);
        if (path)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                // 子プロセスでリダイレクションを設定
                if (node->redirects && setup_redirections(node->redirects) == -1)
                {
                    exit(1);
                }

                execve(path, argv, NULL);
                perror("execve failed");
                exit(1);
            }
            else if (pid > 0)
            {
                int child_status;
                waitpid(pid, &child_status, 0);
                *stat_loc = WEXITSTATUS(child_status);
            }
            else
            {
                perror("fork failed");
                *stat_loc = 1;
            }
            free(path);
        }
        else
        {
            printf("Command not found: %s\n", argv[0]);
            *stat_loc = 127;
        }
        free_argv(argv);
    }
    break;

    case ND_PIPE:
        execute_pipe(node, stat_loc);
        break;

    default:
        printf("Unsupported node type: %d\n", node->kind);
        *stat_loc = 1;
        break;
    }
}

void interpret(char *line, int *stat_loc)
{
    t_token *tok = tokenize(line);
    t_node *node = parse(tok);
    // 例：echo "hello" | wc -l　なら、leftとrightにecho...とwc..をつけたPIPE属性のノードが返ってくる

    if (tok->kind == TK_EOF)
        *stat_loc = 0;
    else if (syntax_error)
        *stat_loc = ERROR_TOKENIZE;
    else
    {
        printf("=== PARSING RESULT ===\n");
        print_node_debug(node);

        printf("=== EXECUTING COMMAND ===\n");
        execute_node(node, stat_loc);
    }

    // メモリ解放
    while (tok)
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
