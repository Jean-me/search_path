#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#define PATH_MAX 4096

void fatal_error(const char *msg)
{
    dprintf(STDERR_FILENO, "Fatal Error: %s\n", msg);
    exit(1);
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

int exe(char *path)
{
    char *args[] = {path, NULL, NULL};
    execve(path, args, NULL);
    return (0);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        dprintf(STDERR_FILENO, "Usage: %s <filename>\n", argv[0]);
        return (1);
    }

    char *path = search_path(argv[1]);
    if (path)
    {
        printf("Found executable at: %s\n", path);
        exe(path);
        free(path);
    }
    else
    {
        printf("Executable not found in PATH.\n");
    }

    return (0);
}