/*

 src/test/_host.h - Simple sandboxing test harness.
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2014 Ben Golightly <golightly.ben@googlemail.com>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 ------------------------------------------------------------------------------
 
 This is a simple test framework that uses fork to execute tests in a child
 process so that abnormal termination (e.g. a segfault) can be caught and
 treated as a failure.
 
 See also: src/test/_plan.h which details the functions that are executed by
 separate forked processes, in order, and /src/test/ *.c which are the actual
 implementations of the tests.
 
*/

#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid
#include <stdio.h>      // printf
#include <stdlib.h>     // _Exit
#include <unistd.h>     // fork
#include <errno.h>
#include <string.h>     // strlen
#include "base.h"       // X (exception)
#include "test/_test.h"

extern int hg14_Xquiet; // src/exception.c


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
        hg14_Xquiet = 1; // suppress all errors from src/exception.c
        int result = fp();
        hg14_Xquiet = 0; // turn error reporting back on
        
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
