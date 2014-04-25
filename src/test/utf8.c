// BSAG 2014. public domain.

#include "utf8.h"
#include "test/_test.h"
#include <stdio.h>
#include <string.h>


// int ParseMultibyte(const char *string, size_t *len);
// size_t MultibyteBackwardsLength(const char *string, size_t start, size_t end);


static int utf8_strlen(const char *msg)
{
    size_t str_len = 0;
    size_t char_len = 0;
    
    while (*msg)
    {
        ParseMultibyte(msg, &char_len);
        if (char_len == 0) { return -1; }
    
        str_len++;
        msg += char_len;
    }
    
    return str_len;
}


static int utf8_strlen2(const char *msg)
{
    size_t str_len = 0;
    size_t end = strlen(msg);
    
    while (end)
    {
        size_t char_len = MultibyteBackwardsLength(msg, 0, end);
        if (char_len == 0) { return -1; }
    
        if (end < char_len) { return -1; }
        end -= char_len;
        str_len++;
    }
    
    return str_len;
}


int test_utf8_strlen(void)
{
    START;
    
#   define TEST_STRLEN(len, str) \
        TEST(len == utf8_strlen(str)); \
        TEST(len == utf8_strlen2(str));
    
    TEST_STRLEN(0, "");
    TEST_STRLEN(1, "a");
    TEST_STRLEN(3, "abc");
    TEST_STRLEN(1, "£");
    TEST_STRLEN(3, "£££");
    TEST_STRLEN(6, "a£b£c£");
    TEST_STRLEN(1, "€");
    TEST_STRLEN(5, "€£$1\n");
    
    END;
}

