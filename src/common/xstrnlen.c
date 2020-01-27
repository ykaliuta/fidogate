/*
 */

#include "fidogate.h"

size_t xstrnlen(const char *str, size_t len)
{
    size_t i;

    for (i = 0; i < len && str[i]; i++) ;
    return i;
}
