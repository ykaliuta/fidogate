/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Miscellaneous functions
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
 *  _____ _____
 * |	 |___  |   Martin Junius	     FIDO:	2:2452/110
 * | | | |   | |   Radiumstr. 18  	     Internet:	mj@fidogate.org
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FIDOGATE; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/

#include "fidogate.h"

/*
 * str_change_ext(): change extension of string buffer containing filename
 */
char *str_change_ext(char *new, size_t len, char *old, char *ext)
{
    int off;

    str_copy(new, len, old);
    off = strlen(new) - strlen(ext);
    if (off < 0)
        off = 0;
    str_copy(new + off, len - off, ext);

    return new;
}

/*
 * str_printf(): wrapper for sprintf()/snprintf()
 */
void str_printf(char *buf, size_t len, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

#ifdef HAVE_SNPRINTF
    vsnprintf(buf, len, fmt, args);
    /**FIXME: check for n==-1 and errno**/
#else

    if (vsprintf(buf, fmt, args) >= len) {
        fatal("Internal error - str_printf() buf overflow", EX_SOFTWARE);
        /**NOT REACHED**/
        return;
    }
#endif
    /* Make sure that buf[] is terminated with a \0. vsnprintf()
     * should do this automatically as required by the ANSI C99
     * proposal, but one never knows ... see also discussion on
     * BugTraq */
    buf[len - 1] = 0;

    va_end(args);

    return;
}

/*
 * Last char in string
 */
int str_last(char *s, size_t len)
{
    int l = strlen(s) - 1;
    if (l >= len)
        l = len - 1;
    if (l < 0)
        l = 0;

    return s[l];
}

/*
 * Convert to lower case
 */
char *str_lower(char *s)
{
    char *p;

    if (!s)
        return NULL;

    for (p = s; *p; p++)
        if (isupper(*p))
            *p = tolower(*p);

    return s;
}

/*
 * Convert to upper case
 */
char *str_upper(char *s)
{
    char *p;

    if (!s)
        return NULL;

    for (p = s; *p; p++)
        if (islower(*p))
            *p = toupper(*p);

    return s;
}

/*
 * Secure string functions:
 *
 * str_copy  (d,n,s)              - copy string
 * str_copy2 (d,n,s1,s2)          - copy concatenation of 2 strings
 * str_copy3 (d,n,s1,s2,s3)       - copy concatenation of 3 strings
 * str_copy4 (d,n,s1,s2,s3,s4)    - copy concatenation of 4 strings
 * str_copy5 (d,n,s1,s2,s3,s4,s5) - copy concatenation of 5 strings
 * str_append(d,n,s)              - append string
 *
 * d = destination buffer
 * n = destination size
 */
char *str_copy(char *d, size_t n, char *s)
{

    memccpy((void *)d, (void *)s, 0, n);
    d[n - 1] = '\0';

    return d;
}

char *str_append(char *d, size_t n, char *s)
{
    int max = n - strlen(d) - 1;
    strncat(d, s, max);
    d[n - 1] = 0;

    return d;
}

char *str_append2(char *d, size_t n, char *s1, char *s2)
{
    int sz = strlen(d);
    char *p = s1;

    for (; *p && sz < n; p++, sz++)
        d[sz] = *p;

    for (p = s2; *p && sz < n; p++, sz++)
        d[sz] = *p;

    d[sz++] = 0;

    return d;
}

char *str_copy2(char *d, size_t n, char *s1, char *s2)
{
    int i = 0;
    char *p = s1;

    for (; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s2; *p && i < n; p++, i++)
        d[i] = *p;

    d[i++] = 0;

    return d;
}

char *str_copy3(char *d, size_t n, char *s1, char *s2, char *s3)
{

    int i = 0;
    char *p = s1;

    for (; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s2; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s3; *p && i < n; p++, i++)
        d[i] = *p;

    d[i++] = 0;

    return d;
}

char *str_copy4(char *d, size_t n, char *s1, char *s2, char *s3, char *s4)
{
    int i = 0;
    char *p = s1;

    for (; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s2; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s3; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s4; *p && i < n; p++, i++)
        d[i] = *p;

    d[i++] = 0;

    return d;
}

char *str_copy5(char *d, size_t n, char *s1, char *s2, char *s3, char *s4,
                char *s5)
{
    int i = 0;
    char *p = s1;

    for (; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s2; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s3; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s4; *p && i < n; p++, i++)
        d[i] = *p;

    for (p = s5; *p && i < n; p++, i++)
        d[i] = *p;

    d[i++] = 0;

    return d;
}

/*
 * Copy string range
 */
char *str_copy_range(char *d, size_t n, char *s, char *lim)
{
    int i;

    for (i = 0; i < n - 1 && s < lim; i++, s++)
        d[i] = *s;
    d[i] = 0;

    return d;
}

int strlen_zero(char *s)
{
    int i = 0;

    while (s[i] != 0)
        i++;

    return i;
}

#if !defined(HAVE_STRCASECMP) && !defined(HAVE_STRICMP)
/***** strnicmp() --- compare n chars of strings ignoring case ***************/

int strnicmp(char *sa, char *sb, int len)
{
    while (len--)
        if (tolower(*sa) == tolower(*sb)) {
            sa++;
            sb++;
        } else if (tolower(*sa) < tolower(*sb))
            return (-1);
        else
            return (1);
    return (0);
}

/***** stricmp() --- compare strings ignoring case ***************************/

int stricmp(char *sa, char *sb)
{
    while (tolower(*sa) == tolower(*sb)) {
        if (!*sa)
            return (0);
        sa++;
        sb++;
    }
    return (tolower(*sa) - tolower(*sb));
}
#endif /**!HAVE_STRCASECMP && !HAVE_STRICMP**/

/*
 * Remove \r\n from end of line
 */
void strip_crlf(char *line)
{
    int len;

    if (!line)
        return;

    len = strlen(line);
    if (line[len - 1] == '\n')
        line[len - 1] = 0;

    len = strlen(line);
    if (line[len - 1] == '\r')
        line[len - 1] = 0;
}

/*
 * Remove white space at end of line
 */
char *strip_space(char *line)
{
    int i;
    char *s;

    if (!line)
        return NULL;

    for (i = strlen(line) - 1; i >= 0; i--)
        if (is_space(line[i]))
            line[i] = 0;
        else
            break;

    for (s = line; is_space(*s); s++) ;
    return s;
}

/*
 * isspace() replacement, checking for SPACE, TAB, CR, LF
 */
int is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/*
 * checking for SPACE or TAB
 */
int is_blank(int c)
{
    return c == ' ' || c == '\t';
}

/*
 * signed char-safe version of isdigit()
 */
int is_digit(int c)
{
    /* Some <ctype.h> implementation only accept a parameter value range
     * of [-1,255]. This may lead to problems, because we're quite often
     * passing *p's to is_digit() with a char *p variable. The default
     * char type is signed in most C implementation. */
    return isdigit((c & 0xff));
}

int atooct(register char *s)
{
    register int i = 0;

    while (*s >= '0' && *s <= '7')
        i = (i << 3) | (*s++ - '0');

    return i;
}

/*
 * Check for hex digits, signed char-safe version of isxdigit()
 */
int is_xdigit(int c)
{
    /* Some <ctype.h> implementation only accept a parameter value range
     * of [-1,255]. This may lead to problems, because we're quite often
     * passing *p's to is_digit() with a char *p variable. The default
     * char type is signed in most C implementation. */
    return isxdigit((c & 0xff));
}

/*
 * Check for octal digits
 */
int is_odigit(int c)
{
    return c >= '0' && c < '8';
}

/*
 * Expand %X at start of file names
 *
 * See config.make for definition of abbreviations.
 */
static struct st_atable {
    int c;
    char *(*func)(void);
} atable[] = {
    /* Include code generated by subst.pl -A */
#include "cf_abbrev.c"
    {0, NULL}
};

char *str_expand_name(char *d, size_t n, char *s)
{
    int i;

    d[0] = 0;

    if (s[0] == '%') {
        for (i = 0; atable[i].c; i++)
            if (atable[i].c == s[1]) {
                str_append(d, n, atable[i].func());
                s += 2;
                break;
            }
    }

    str_append(d, n, s);

    return d;
}

/*
 * Convert `/' to `\' for MSDOS / OS2 filenames
 */
char *str_dosify(char *s)
{
    for (; *s; s++)
        switch (*s) {
        case '/':
            *s = '\\';
            break;
        }
    return s;
}

/*
 * Run system command, return exit code
 */
int run_system(char *s)
{
    char cmd[BUFFERSIZE];

    BUF_EXPAND(cmd, s);
    DOSIFY_IF_NEEDED(cmd);
    return (system(cmd) >> 8) & 0xff;
}

int xfeof(FILE * fp)
{
    int c;

    c = fgetc(fp);
    if (EOF == c)
        return TRUE;
    else {
        ungetc(c, fp);
        return FALSE;
    }
}

/*=================================================================*/
/* This is part of INN 1.7.2                                       */
/* Modify by Andy Igoshin                                          */
static int argify(char *line, char ***argvp)
{

    register char **argv;
    register char *p;
    register int i;

    if (*argvp != NULL) {
        xfree((void *)*argvp[0]);
        xfree((void *)*argvp);
    }

    /*  Copy the line, which we will split up. */
    while (*line == ' ' || *line == '\t')
        line++;

    i = strlen(line);
    p = strsave(line);

    /* Allocate worst-case amount of space. */
    *argvp = argv = (char **)xmalloc(sizeof(char *) * (i + 2));
    while (*p) {
        /* Mark start of this word, find its end. */
        for (*argv++ = p; *p && *p != ' ' && *p != '\t';)
            p++;
        if (*p == '\0')
            break;
        /* Nip off word, skip whitespace. */
        for (*p++ = '\0'; *p == ' ' || *p == '\t';)
            p++;
    }
    *argv = NULL;

    return argv - *argvp;
}

/*
**  Parse a newsgroups line, return TRUE if there were any.
*/
void list_init(char ***argvp, char *list)
{

    register char *p;

    if (list) {
        for (p = list; *p; p++)
            if (*p == ',')
                *p = ' ';
        argify(list, argvp);
    }
    return;
}

/*
 * Free the list, it's a bit more complicated than just free() call
 */
void list_free(char **argv)
{

    if (argv == NULL)
        return;

    /*
     * In argify() the result of strlen() is saved in the first
     * element, then elements point inside. And the pointers array
     * itself.
     */
    xfree((void *)argv[0]);
    xfree((void *)argv);
}

/*
**  Match a list of newsgroup specifiers against a list of newsgroups.
**  func is called to see if there is a match.
*/
int list_match(char **Pats, char **list)
{

    register int match = FALSE;
    register int i;
    register char *p;

    if (Pats && list && Pats[0] != NULL) {
        for (; *list; list++) {
            for (i = 0; (p = Pats[i]) != NULL; i++) {
                if (p[0] == '!') {
                    if (wildmat(*list, ++p))
                        match = FALSE;
                } else if (wildmat(*list, p))
                    match = TRUE;
            }
        }
    }
    return match;
}
