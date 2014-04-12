/*
 
 src/base.h - common header
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2012 - 2014 Ben Golightly <golightly.ben@googlemail.com>

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
 
*/

#ifndef HG14_BASE_H
#   define HG14_BASE_H

#   define PROGID "hexgen2014"

    /* Guess at minimum C spec version feature detection */
#   if defined(__STDC__)
#       define IS_C89

#       if defined(__STDC_VERSION__)
#           define IS_C90

#           if (__STDC_VERSION__ >= 199409L)
#               define IS_C94
#           endif

#           if (__STDC_VERSION__ >= 199901L)
#               define IS_C99
#           endif
#       endif
#   endif


    /* == Basic Macros === */
#   define UNUSED(x)            (void)x
#   define NOP                  ((void)0)

#   define ALIGNMENT_OF(t) \
           offsetof( struct { char anon_x; anon_alignment; }, \
                     anon_alignment)


    /* This isn't always defined. It turns __STRING(PHRASE) into "PHRASE" */
#   ifndef __STRING
#       define __STRING(x) #x
#   endif


    /* convert e.g. __LINE__ to "123" */
#   define STR_HELPER(x) #x
#   define MACRO_EVAL_STR(x) STR_HELPER(x)


    /* __func__, if available, is a actually a special char array -- note
       that this means it cannot be concatinated by the preproccessor like
       string literals! This also means that the pre-C99 version here is of a
       different type, so watch out and only use STRING_FUNC as a standalone
       parameter to printf.
   */
#   ifdef IS_C99
#       define STRING_FUNC  __func__
#   else
#       define STRING_FUNC   "(unknown function)"
#   endif


    /* == Inlining == */
#   ifdef __GNUC__
#       define INLINE       __inline__
#       define FORCE_INLINE __attribute__((always_inline)) __inline__
#   else
#       warning Using default inline keyword (expected GNUC)
#       define INLINE inline
#       define FORCE_INLINE inline
#   endif

#   define HEADER_FUNC static FORCE_INLINE


    /* === library symbol visibility (compile with -fvisibility=hidden) === */
#   if (defined(__GNUC__)) && (__GNUC__ >= 4)
#       define public   __attribute__ ((visibility ("default")))
#       define private  __attribute__ ((visibility ("hidden")))
#   else
#       warning Cannot mark attributes as public or private (expected GNUC >= 4)
#       define public
#       define private
#   endif


    /* == Exceptions == */

#   ifdef X
#       error X already defined
#   endif

#   ifdef X2
#       error X2 already defined
#   endif

#   ifdef W
#       error W already defined
#   endif

#   ifdef W
#       error W2 already defined
#   endif

#   ifndef QUIET_EXCEPTIONS
#       define X(label) \
            X2(label, NULL)

#       define X2(label, msg) \
            hg14_print_exception("Exception", __FILE__, __LINE__, STRING_FUNC, __STRING(label), msg, 0); \
            goto err_##label;

#       define X3(label, msg, errnum) \
            hg14_print_exception("Exception", __FILE__, __LINE__, STRING_FUNC, __STRING(label), msg, errnum); \
            goto err_##label;

#       define W(msg) \
            hg14_print_warning("Warning", __FILE__, __LINE__, STRING_FUNC, msg, 0);

#       define W2(msg, errnum) \
            hg14_print_warning("Warning", __FILE__, __LINE__, STRING_FUNC, msg, errnum);

#   else
#       define X(label)               goto err_##label;
#       define X2(label, msg)         goto err_##label;
#       define X3(label, msg, errnum) goto err_##label;
#       define W(msg)          NOP;
#       define W2(msg, errnum) NOP;
#endif

void hg14_print_exception
(
    const char *type,
    const char *file,
    unsigned int line,
    const char *func,
    const char *label,
    const char *msg,
    int errnum
);

void hg14_print_warning
(
    const char *type,
    const char *file,
    unsigned int line,
    const char *func,
    const char *msg,
    int errnum
);

#endif // BASE_H
