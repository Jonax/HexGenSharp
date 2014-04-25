#include "utf8.h"

/*
 * ParseMultibyte
 * ==============
 * 
 * Attempts to parse <string> as a utf8-encoded string. The length
 * of the first utf8 character is stored in the memory location pointed to by
 * <len>.
 * 
 * Returns true on success, false on error or if the sequence is invalid.
 * If false is returned, The value stored in <len> may be undefined.
 * 
 * License: CC BY-SA 3.0
 * Source: http://stackoverflow.com/a/1031773/275677
 * Author: http://stackoverflow.com/users/48015/christoph
*/
int ParseMultibyte(const char *string, size_t *len)
{
    if (!len) { return 0; }
    *len = 0;
    if (!string) { return 0; }
    
    const unsigned char * bytes = (const unsigned char *)string;
    
    if (bytes[0] == '\0') { return 0; }
    
    if
    (   (// ASCII
         // use bytes[0] <= 0x7F to allow ASCII control characters
            bytes[0] == 0x09 ||
            bytes[0] == 0x0A ||
            bytes[0] == 0x0D ||
            (0x20 <= bytes[0] && bytes[0] <= 0x7E)
        )
    )
    {
        *len = 1;
        return 0;
    }
    
    if (bytes[1] == '\0') { return 0; }
    
    if
    (   (// non-overlong 2-byte
            (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF)
        )
    )
    {
        *len = 2;
        return 1;
    }
    
    if (bytes[2] == '\0') { return 0; }
    
    if
    (   (// excluding overlongs
            bytes[0] == 0xE0 &&
            (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)
        ) ||
        (// straight 3-byte
            ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
                bytes[0] == 0xEE ||
                bytes[0] == 0xEF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)
        ) ||
        (// excluding surrogates
            bytes[0] == 0xED &&
            (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)
        )
    )
    {
        *len = 3;
        return 1;
    }
    
    if (bytes[3] == '\0') { return 0; }
    
    if
    (   (// planes 1-3
            bytes[0] == 0xF0 &&
            (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)
        ) ||
        (// planes 4-15
            (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)
        ) ||
        (// plane 16
            bytes[0] == 0xF4 &&
            (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)
        )
    )
    {
        *len = 4;
        return 1;
    }

    return 0; // something else
}


/*
 * MultibyteBackwardsLength
 * ========================
 * 
 * Attempts to return a relative offset from the <end> of <string> bounded
 * by <start> such that the returned length represents the length in bytes
 * of the last utf8 character in the string.
 * 
 * This does not validate that the character is valid; use ParseMultibyte for
 * that.
 *
 * On failure, such as reaching the start of the string or reading an obviously
 * invalid sequence, this function returns 0.
 * 
*/
size_t MultibyteBackwardsLength(const char *string, size_t start, size_t end)
{
    if (!string)      { return 0; }
    if (!end)         { return 0; }
    if (end <= start) { return 0; }
    size_t length = 0;
    end--;
    
    unsigned int c = (unsigned int) (string[end]);
    
    if ((c & 0x80) != 0x80) { return 1; } // 0xxxxxxx / ASCII
    
    while ((c & 0xC0) == 0x80) // 10xxxxxx
    {
        if (c == 0) { return 0; }
        if (end == start) { return 0; }
        
        end--;
        length++;
        c = (unsigned int) (string[end]);
    }
    
    if (length == 0) { return 0; }
    if (length >= 4) { return 0; }
    
    return length + 1;
}
