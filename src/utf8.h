#include <stddef.h> // len

int ParseMultibyte(const char *string, size_t *len);
size_t MultibyteBackwardsLength(const char *string, size_t start, size_t end);
