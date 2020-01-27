/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Read lines from stdio FILE, optionally limited by size parameter
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
 *  _____ _____
 * |     |___  |   Martin Junius             <mj@fidogate.org>
 * | | | |   | |   Radiumstr. 18
 * |_|_|_|@home|   D-51069 Koeln, Germany
 *
 * This file is part of FIDOGATE.
 *
 * FIDOGATE is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * FIDOGATE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FIDOGATE; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/

#include "fidogate.h"

static long read_size = -1;

/***** Read line from FILE, like fgets() but with read_size checking *********/

char *read_line(char *buf, int n, FILE * stream)
{
    int c;
    char *s;

    if (n <= 1)
        return NULL;
    if (read_size != -1 && read_size <= 0)
        return NULL;
    if ((c = getc(stream)) == EOF)
        return NULL;

    s = buf;
    n--;

    while (TRUE) {
        *s++ = c;
        n--;
        if (read_size != -1)
            read_size--;
        if (n <= 0 || (read_size != -1 && read_size <= 0))
            break;
        if (c == '\n') {
            *s = 0;
            return buf;
        }
        if ((c = getc(stream)) == EOF) {
            *s = 0;
            return buf;
        }
    }

    *s = 0;
    return buf;
}

/***** Read "#! rnews nnnn" and set read_size ********************************/

long read_rnews_size(FILE * stream)
{
    char buffer[32];
    long n;

    if (!fgets(buffer, sizeof(buffer), stream))
        return 0;
    if (buffer[0] != '#' && strlen(buffer) < 10)
        return -1;
    if (strncmp(buffer, "#! rnews ", 9))
        return -1;

    n = atol(buffer + 9);
    if (n > 0) {
        read_size = n;
        return n;
    } else {
        read_size = -1;
        return 0;
    }
}

#ifdef TEST

int main(int argc, char *argv[])
{
#if 0
    char test[16];
    char buffer[16];
    int i;

    memset(test, 0, sizeof(test));

    while (read_line(buffer, sizeof(buffer), stdin)) {
        printf("n=%d buffer=<%s>\n", strlen(buffer), buffer);
    }

    for (i = 0; i < sizeof(test); i++)
        if (test[i])
            printf("Error - overwrite!\n");
#endif
#if 1
    char buffer[512];
    long n, lines, articles;

    articles = 0;
    while ((n = read_rnews_size(stdin)) > 0) {
        printf("size=%ld ", n);
        lines = 0;
        articles++;

        while (read_line(buffer, sizeof(buffer), stdin))
            if (buffer[strlen(buffer) - 1] == '\n')
                lines++;
        printf("lines=%ld\n", lines);
    }
    printf("articles=%ld\n", articles);
    if (n < 0)
        printf("Error!\n");
#endif
}

#endif /**TEST**/
