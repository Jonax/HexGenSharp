#include "base.h"
#include <stdio.h> // fprintf
#include <string.h>
#include <errno.h>

void hg14_print_exception
(
    const char *type,
    const char *file,
    unsigned int line,
    const char *func,
    const char *label,
    const char *msg,
    int errnum
)
{
    char errnobuf[256];
    
    if (!errnum)
    {
        sprintf(errnobuf, "(no errno)");
    }
    else
    {
        strncpy(errnobuf, strerror(errnum), 256);
        errnobuf[255] = '\0';
    }
    
    if (!msg)
    {
        fprintf
        (
            stdout,
            PROGID ": %s:%u %s: %s 'err_%s' at line %u\n"
            "       (errno: %d, '%s')\n",
              file, line, func, type, label, line,
              errnum, errnobuf
        );
    }
    else
    {
        fprintf
        (
            stdout,
            PROGID ": %s:%u %s: %s 'err_%s' at line %u\n"
            "      (errno: %d, '%s')\n"
            "      (detail: '%s')\n",
              file, line, func, type, label, line,
              errnum, errnobuf,
              msg
        );
    }
    
    fflush(stdout);
}


void hg14_print_warning
(
    const char *type,
    const char *file,
    unsigned int line,
    const char *func,
    const char *msg,
    int errnum
)
{
    char errnobuf[256];
    
    if (!errnum)
    {
        sprintf(errnobuf, "(no errno)");
    }
    else
    {
        strncpy(errnobuf, strerror(errnum), 256);
        errnobuf[255] = '\0';
    }
    
    if (!msg)
    {
        fprintf
        (
            stdout,
            PROGID ": %s:%u %s: %s at line %u\n"
            "      (errno: %d, '%s')\n",
              file, line, func, type, line,
              errnum, errnobuf
        );
    }
    else
    {
        fprintf
        (
            stdout,
            PROGID ": %s:%u %s: %s at line %u\n"
            "      (errno: %d, '%s')\n"
            "      (detail: '%s')\n",
              file, line, func, type, line,
              errnum, errnobuf,
              msg
        );
    }
    
    fflush(stdout);
}


