/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Function for handling temporary strings
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

/*
 * Initial size of temp string for printf() functions. If vsnprintf() is
 * not supported, this size must be large enough to hold the complete output
 * string.
 */
#ifdef HAVE_SNPRINTF
#define TMPS_PRINTF_BUFSIZE	128
#else
#define TMPS_PRINTF_BUFSIZE	4096    /* To be reasonably safe */
#endif

/*
 * Linked list of temporary strings
 */
static TmpS *tmps_list = NULL;
static TmpS *tmps_last = NULL;

/*
 * Fatal error message and exit
 */
void fatal(char *msg, int ex)
{
    fputs(msg, stderr);
    fputs("\n", stderr);
    /**FIXME:#ifdef for abort()**/
    exit(ex);
    /**NOT REACHED**/
}

/*
 * Allocate new temporary string
 */
TmpS *tmps_alloc(size_t len)
{
    TmpS *p;

    /* Alloc */
    p = (TmpS *) xmalloc(sizeof(TmpS));

    p->s = (char *)xmalloc(len);
    p->len = len;
    p->next = NULL;

    memset(p->s, 0, len);

    /* Put into linked list */
    if (tmps_list)
        tmps_last->next = p;
    else
        tmps_list = p;
    tmps_last = p;

    return p;
}

/*
 * Realloc temporary string
 */
TmpS *tmps_realloc(TmpS * s, size_t len)
{
    s->s = (char *)xrealloc(s->s, len);
    s->len = len;

    return s;
}

/*
 * Find temporary string by char *
 */
TmpS *tmps_find(char *s)
{
    TmpS *p;

    for (p = tmps_list; p; p = p->next)
        if (p->s == s)
            /* Found */
            return p;

    return NULL;
}

/*
 * Free temporary string
 */
void tmps_free(TmpS * s)
{
    TmpS *p, *pp;

    p = tmps_list;
    pp = NULL;
    while (p) {
        if (p == s) {
            /* Remove from list */
            if (pp)
                pp->next = p->next;
            else
                tmps_list = p->next;
            p->next = NULL;
            p->len = 0;
            xfree(p->s);
            xfree(p);

            return;
        }

        pp = p;
        p = p->next;
    }

    exit_free();

    /* Not found, internal error */
    fatal("tmps_free() internal error - freeing invalid temp string",
          EX_SOFTWARE);
}

/*
 * Free all temporary strings
 */
void tmps_freeall(void)
{
    TmpS *p, *pp;

    p = tmps_list;
    pp = NULL;
    while (p) {
        pp = p;
        p = p->next;
#ifdef TEST
        printf("tmps_freeall(): tmps=%08x s=%08x len=%d\n",
               (unsigned)pp, (unsigned)pp->s, pp->len);
#endif
        pp->next = NULL;
        pp->len = 0;
        xfree(pp->s);
        xfree(pp);
    }

    tmps_list = NULL;
    tmps_last = NULL;
}

/*
 * New temp string, init with printf()
 */
TmpS *tmps_printf(const char *fmt, ...)
{
    va_list args;
    TmpS *p;
    int n;

    va_start(args, fmt);

    p = tmps_alloc(TMPS_PRINTF_BUFSIZE);
#ifdef HAVE_SNPRINTF
    do {
        n = vsnprintf(p->s, p->len, fmt, args);
        /* Resize if too small */
        if (n == ERROR)
            tmps_realloc(p, p->len * 2);
    }
    while (n == ERROR);
#else
    n = vsprintf(p->s, fmt, args);
    if (n >= p->len) {
        fatal("Internal error - temp string printf overflow", EX_SOFTWARE);
    /**NOT REACHED**/
        return NULL;
    }
#endif
    /* Realloc to actual size */
    tmps_realloc(p, strlen(p->s) + 1);

    va_end(args);

    return p;
}

/*
 * New temp string, init with char *
 */
TmpS *tmps_copy(char *s)
{
    TmpS *p;

    p = tmps_alloc(strlen(s) + 1);
    str_copy(p->s, p->len, s);

    return p;
}

/*
 * Resize temp string to actual length
 */
TmpS *tmps_stripsize(TmpS * s)
{
    tmps_realloc(s, strlen(s->s) + 1);
    return s;
}

/*
 * Alloc new temporary string, return char *
 */
char *s_alloc(size_t len)
{
    return tmps_alloc(len)->s;
}

/*
 * Realloc temporary string, return char *
 */
char *s_realloc(char *s, size_t len)
{
    TmpS *p;

    if ((p = tmps_find(s))) {
        tmps_realloc(p, len);
        return p->s;
    }

    /* Not found, internal error */
    fatal("Internal error - realloc for invalid temp string", EX_SOFTWARE);
    /**NOT REACHED**/
    return NULL;
}

/*
 * Free temporary string, using char *
 */
void s_free(char *s)
{
    TmpS *p, *pp;

    p = tmps_list;
    pp = NULL;
    while (p) {
        if (p->s == s) {
            /* Remove from list */
            if (pp)
                pp->next = p->next;
            else
                tmps_list = p->next;
            p->next = NULL;
            p->len = 0;
            xfree(p->s);
            xfree(p);

            return;
        }

        pp = p;
        p = p->next;
    }

    /* Not found, internal error */
    fatal("s_free() internal error - freeing invalid temp string", EX_SOFTWARE);
}

/*
 * Free all temporary strings
 */
void s_freeall(void)
{
    tmps_freeall();
}

/*
 * New temp string, init with printf()
 */
char *s_printf(const char *fmt, ...)
{
    va_list args;
    TmpS *p;
    int n;

    va_start(args, fmt);

    p = tmps_alloc(TMPS_PRINTF_BUFSIZE);
#ifdef HAVE_SNPRINTF
    do {
        n = vsnprintf(p->s, p->len, fmt, args);
        /* Resize if too small */
        if (n == ERROR)
            tmps_realloc(p, p->len * 2);
    }
    while (n == ERROR);
#else
    n = vsprintf(p->s, fmt, args);
    if (n >= p->len) {
        fatal("Internal error - temp string printf overflow", EX_SOFTWARE);
    /**NOT REACHED**/
        return NULL;
    }
#endif
    /* Realloc to actual size */
    tmps_realloc(p, strlen(p->s) + 1);

    va_end(args);

    return p->s;
}

/*
 * New temp string, init with char *
 */
char *s_copy(char *s)
{
    return tmps_copy(s)->s;
}

/*
 * Resize temp string to actual length
 */
char *s_stripsize(char *s)
{
    TmpS *p;

    p = tmps_find(s);
    if (!p)
        /* Not found, internal error */
        fatal("s_stripsize() internal error - freeing invalid temp string",
              EX_SOFTWARE);

    tmps_realloc(p, strlen(p->s) + 1);

    return p->s;
}

#ifdef TEST /****************************************************************/

int testf()
{
    s_copy("testf string 1");
    s_copy("testf string 2");

    if (1)
        TMPS_RETURN(255);

    TMPS_RETURN(0);

    return 1;
}

int main(int argc, char *argv[])
{
    int i;
    TmpS *p;
    char *s, *s2, *s4;

    printf("Calling testf()\n");
    i = testf();
    printf("Returned from testf(), ret=%d\n", i);

    for (i = 1; i <= 10; i++) {
        s = s_alloc(i * 20);
        str_printf(s, i * 20, "temp: >%d<", i);
        if (i == 2)
            s2 = s;
        if (i == 4)
            s4 = s;
    }

    printf("List of temporary strings:\n");
    for (p = tmps_list; p; p = p->next)
        printf("    %08lx: s=%s, len=%ld, next=%08lx\n",
               (unsigned long)p, p->s, (long)p->len, (unsigned long)p->next);

    s_free(s2);
    s_free(s4);

    printf("List of temporary strings:\n");
    for (p = tmps_list; p; p = p->next)
        printf("    %08lx: s=%s, len=%ld, next=%08lx\n",
               (unsigned long)p, p->s, (long)p->len, (unsigned long)p->next);

    s_freeall();

    printf("Expect empty list\n");
    printf("List of temporary strings:\n");
    for (p = tmps_list; p; p = p->next)
        printf("    %08lx: s=%s, len=%ld, next=%08lx\n",
               (unsigned long)p, p->s, (long)p->len, (unsigned long)p->next);

    s_printf("Test printf %d %08X %-8.8s %4.2f",
             15, 0x1234abcd, "1234567890", 25.6);
    s_printf("Test printf %d %08X %-8.8s %4.2f",
             1234, 0xfe00128d, "( )", 300.1256);
    s_printf("Test printf %d %08X %-8.8s %4.2f",
             16, 0xffff0000, "----------------", 3.14159265);
    s_printf("Test printf LARGE %d\n%-80s%80s%-80s---",
             123456, "abc", "def", "ghi");

    printf("List of temporary strings:\n");
    for (p = tmps_list; p; p = p->next)
        printf("    %08lx: s=%s, len=%ld, next=%08lx\n",
               (unsigned long)p, p->s, (long)p->len, (unsigned long)p->next);

    printf("Expect error message\n");
    s_free(s2);

    exit(0);
    return 0;
}

#endif /**TEST***************************************************************/
