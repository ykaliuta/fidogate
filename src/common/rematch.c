/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Regular expression (POSIX functions) handling for FIDOGATE
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

#ifdef HAS_POSIX_REGEX /*******************************************************/
#ifdef HAVE_RX
#include <rxposix.h>
#else
#include <regex.h>
#endif

/*
 * List of regular expressions
 */
typedef struct st_regex {
    struct st_regex *next;
    char *re_s;
    regex_t re_c;
} Regex;

static Regex *regex_list = NULL;
static Regex *regex_last = NULL;

/*
 * Alloc and init new Regex struct
 */
static Regex *regex_new(void)
{
    Regex *p;

    p = (Regex *) xmalloc(sizeof(Regex));

    /* Init */
    p->next = NULL;
    p->re_s = NULL;

    return p;
}

/*
 * Create new Regex struct for string
 */
static Regex *regex_parse_line(char *s)
{
    Regex *p;
    int err;

    /* New regex entry */
    p = regex_new();

    p->re_s = strsave(s);
    err = regcomp(&p->re_c, p->re_s, REG_EXTENDED | REG_ICASE);
    if (err) {
        fglog("WARNING: error compiling regex %s", p->re_s);
        xfree(p);
        return NULL;
    }

    debug(15, "regex: pattern=%s", p->re_s);

    return p;
}

/*
 * Put regex into linked list
 */
static int regex_do_entry(char *s)
{
    Regex *p;

    p = regex_parse_line(s);
    if (!p)
        return ERROR;

    /* Put into linked list */
    if (regex_list)
        regex_last->next = p;
    else
        regex_list = p;
    regex_last = p;

    return OK;
}

/*
 * Match string against regex list
 */
#define MAXREGMATCH	10

static regmatch_t regex_pmatch[MAXREGMATCH];

int regex_match(const char *s)
{
    Regex *p;

    for (p = regex_list; p; p = p->next) {
        if (regexec(&p->re_c, s, MAXREGMATCH, regex_pmatch, 0) == OK)
            return TRUE;
    }
    return FALSE;
}

/*
 * Get i'th sub-expression from regex match
 */
static regmatch_t *regex_match_sub(int i)
{
    return i < 0 || i >= MAXREGMATCH ? NULL : &regex_pmatch[i];
}

/*
 * Copy i'th sub-expression to string buffer
 */
char *str_regex_match_sub(char *buf, size_t len, int idx, const char *s)
{
    regmatch_t *p;
    int i, j;

    p = regex_match_sub(idx);
    if (p == NULL) {
        buf[0] = 0;
        return NULL;
    }

    for (i = 0, j = p->rm_so; i < len - 1 && j < p->rm_eo; i++, j++)
        buf[i] = s[j];
    buf[i] = 0;

    return buf;
}

/*
 * Initialize regex list
 */
void regex_init(void)
{
    char *s;

    /* regex patterns from fidogate.conf */
    for (s = cf_get_string("Regex1stLine", TRUE);
         s; s = cf_get_string("Regex1stLine", FALSE))
        regex_do_entry(s);
}

#endif /** HAS_POSIX_REGEX ****************************************************/

/***** TEST ******************************************************************/

#ifdef TEST

#ifdef HAS_POSIX_REGEX
void debug_subs(void)
{
    int i;

    printf("pmatch[]:");
    for (i = 0; i < MAXREGMATCH; i++)
        if (regex_pmatch[i].rm_so != -1)
            printf(" %d-%d", regex_pmatch[i].rm_so, regex_pmatch[i].rm_eo);
    printf("\n");
}
#endif

/*
 * Function test
 */
int main(int argc, char *argv[])
{
#ifdef HAS_POSIX_REGEX
    char buf[MAXINETADDR];

    regex_init();

#if 0
    do {
        printf("Enter regex pattern [ENTER=end of list]: ");
        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        strip_crlf(buffer);
        if (buffer[0])
            regex_do_entry(buffer);
    }
    while (buffer[0]);

    printf("\n");
#endif

    /* Read strings to match */
    do {
        printf("Enter string [ENTER=end]: ");
        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        strip_crlf(buffer);
        if (buffer[0]) {
            if (regex_match(buffer)) {
                printf("MATCH, ");
                debug_subs();
                str_regex_match_sub(buf, sizeof(buf), 1, buffer);
                printf("       (1) = \"%s\"\n", buf);
            } else {
                printf("NO MATCH\n");
            }
        }
    }
    while (buffer[0]);
#endif

    exit(0);
}

#endif /**TEST**/
