#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
// #include "tlpi_hdr.h"

int main()
{
    pid_t pid;

    pid = fork(); // Create a new process

    if (pid < 0)
    {
        perror("for failed");
        return (1);
    }
    else if (pid == 0)
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
    else
    {
        // Parent process waits for the child to finish
        wait(NULL);
        printf("Child process finished.\n");
    }
}
void echo1111()
{
    printf("Hello from echo1111!\n");
}

// int main(int argc, char *argv[])
// {
//     int istack = 222; /* Allocated in stack segment */
//     pid_t childPid;

//     int idata = 111;
//     switch (childPid = fork())
//     {
//     case -1:
//         errExit("fork");

//     case 0:
//         idata *= 3;
//         istack *= 3;
//         break;

//     default:
//         sleep(3); /* Give child a chance to execute */
//         break;
//     }

//     /* Both parent and child come here */

//     printf("PID=%ld %s idata=%d istack=%d\n", (long)getpid(),
//            (childPid == 0) ? "(child) " : "(parent)", idata, istack);

//     exit(EXIT_SUCCESS);
// }