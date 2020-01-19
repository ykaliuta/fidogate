/*
 * $Id: xstrnlen.c,v 5.1 2006/10/31 21:06:02 anray Exp $
 */

#include "fidogate.h"

size_t xstrnlen(const char *str, size_t len)
{
    size_t i;

    for (i = 0; i < len && str[i]; i++) ;
    return i;
}
