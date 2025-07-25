#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>

// typedef struct s_token
// {
//     char *str;            // Pointer to the input line
//     struct s_token *next; // Pointer to the next token in the linked list
// } t_token;

int main()
{
    // Initialize readline
    char *input;

    // Read a line from the user
    input = readline("minishell$ ");

    // If input is not NULL, print it
    if (input)
    {
        printf("You entered: %s\n", input);
        free(input); // Free the/ allocated memory for input
    }
    else
    {
        printf("No input received.\n");
    }

    return 0;
}