#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid
#include <stdio.h>      // printf
#include <stdlib.h>     // _Exit
#include <unistd.h>     // fork
#include <errno.h>
#include <string.h>     // strlen
#include "base.h"       // X (exception)
#include "test/_test.h"

extern int hg14_Xquiet;

int run(int (*fp)(void))
{
    pid_t pid = fork();
    
    if (pid == -1)
    {
        switch (errno)
        {
            case EAGAIN: X2(fork, "resource limit encountered");
            case ENOMEM: X2(fork, "not enough memory to attempt");
            case ENOSYS: X2(fork, "unsupported on this platform");
            default:     X2(fork, "unkown error");
        }
    }
    else if (pid == 0) // in child
    {
        hg14_Xquiet = 1;
        int result = fp();
        hg14_Xquiet = 0;
        
        _Exit(result);
    }
    else // in parent where pid == child pid
    {
        int status;
        
        if (-1 == waitpid(pid, &status, 0))
        {
            switch (errno)
            {
                case ECHILD: X2(wait, "no waitable child process");
                case EINTR:  X2(wait, "unblocked signal or SIGCHLD caught");
                case EINVAL: X2(wait, "invalid options argument to waitpid");
                default:     X2(wait, "unkown error");
            }
        }
        
        if (!WIFEXITED(status)) { X2(wait, "child process terminated abnormally"); }
        return (int) (WEXITSTATUS(status));
    }
    
    err_wait:
    err_fork:
        return 0;
}

int test(int (*fp)(void), const char *name, const char *desc)
{
    char align[16];
    size_t i = strlen(name);
    
    if (i < 22) { i = 22 - i; } else { i = 0; }
    
    align[i] = '\0';
    while (i) { i--; align[i] = ' '; }
    
    fflush(stdout);
    printf("[%s]%s Testing %s ... ", name, align, desc);
    fflush(stdout);
    
    return (run(fp));
}

void report(const char *file, int line, const char *expr)
{
    fflush(stdout);
    printf("\nFAIL: %s:%d: (%s)", file, line, expr);
    fflush(stdout);
}

void result(Result r)
{
    fflush(stdout);
    
    if (r.fail == -1)
    {
        printf("\nABORT (%d/\?\?\? pass)\n", r.pass);
    }
    else if (r.fail > 0)
    {
        printf("\nFAIL: %d passes, %d fails.\n", r.pass, r.fail);
    }
    else
    {
        printf("PASS (%d)\n", r.pass);
    }
    
    fflush(stdout);
}

int main(void)
{
    int result = 1;
    
#   define T(a, b) \
        int a(void); \
        result = test(a, __STRING(a), b) && result;
        
#   include "_plan.h"
    
    fflush(stdout);
    if (result)
    {
        printf("\nOVERALL SUCCESS.\n");
        return EXIT_SUCCESS;
    }
    else
    {
        printf("\nOVERALL FAIL.\n");
        return EXIT_FAILURE;
    }
}
