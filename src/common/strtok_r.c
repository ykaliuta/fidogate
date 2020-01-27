/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Specialized strtok() variants for FIDOGATE
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

char *strtok(char *s, const char *delim)
{
    static char *lasts;

    return strtok_r_ext(s, delim, &lasts, FALSE);
}

char *xstrtok(char *s, const char *delim)
{
    static char *lasts;

    return strtok_r_ext(s, delim, &lasts, DQUOTE);
}

char *strtok_r(char *s, const char *delim, char **lasts)
{
    return strtok_r_ext(s, delim, lasts, FALSE);
}

char *strtok_r_ext(char *s, const char *delim, char **lasts, int quote)
{
    char *tok, *p;

    if (quote == TRUE)
        quote = DQUOTE;
    if (s == NULL && (s = *lasts) == NULL)
        return NULL;

    /* Skip leading delimiters */
    while (*s && strchr(delim, *s))
        s++;
    if (!*s) {
        *lasts = NULL;
        return NULL;
    }
    tok = s++;

    /* In quote mode, check for string enclosed in "..." */
    if (quote && *tok == quote) {
        tok++;
        while (*s) {
            if (s[0] == '\\' && s[1] == quote)
                s++;
            else if (s[0] == quote)
                break;
            s++;
        }
        if (*s == quote)
            *s++ = 0;
        *lasts = s;
        s = p = tok;
        while (*s) {
            if (s[0] == '\\' && s[1] == quote)
                s++;
            *p++ = *s++;
        }
        *p = 0;
        return tok;
    }

    /* Scan token */
    while (*s && !strchr(delim, *s))
        s++;
    if (*s)
        *s++ = 0;
    else
        s = NULL;
    /* Skip trailing delimiters */
    while (s && *s && strchr(delim, *s))
        s++;
    *lasts = s;
    return tok;
}

/***** TEST ******************************************************************/

#ifdef TEST
/*
 * Function test
 */
int main(int argc, char *argv[])
{
    char *d, *p;
    char *last;
    int i;

    BUF_COPY(buffer, "Dies ist ein Test fuer die strtok-Funktionen\n");
    d = DELIM_WS;
    printf("String = %s", buffer);
    for (i = 0, p = strtok_r_ext(buffer, d, &last, FALSE);
         p; i++, p = strtok_r_ext(NULL, d, &last, FALSE))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "Dies ist \"ein Test\" \"fuer die\" strtok-Funktionen\n");
    d = DELIM_WS;
    printf("String = %s", buffer);
    for (i = 0, p = strtok_r_ext(buffer, d, &last, FALSE);
         p; i++, p = strtok_r_ext(NULL, d, &last, FALSE))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "Dies ist \"ein Test\" \"fuer die\" strtok-Funktionen\n");
    d = DELIM_WS;
    printf("String = %s", buffer);
    for (i = 0, p = strtok_r_ext(buffer, d, &last, TRUE);
         p; i++, p = strtok_r_ext(NULL, d, &last, TRUE))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer,
             "Dies ist \"ein Test\\\" \\\"fuer die\" strtok-Funktionen\n");
    d = DELIM_WS;
    printf("String = %s", buffer);
    for (i = 0, p = strtok_r_ext(buffer, d, &last, DQUOTE);
         p; i++, p = strtok_r_ext(NULL, d, &last, DQUOTE))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer,
             "Dies\\s ist \"ein\\1 Test\\\" \\\"fuer \\2die\" strtok-Funktionen\\n\n");
    d = DELIM_WS;
    printf("String = %s", buffer);
    for (i = 0, p = strtok_r_ext(buffer, d, &last, DQUOTE);
         p; i++, p = strtok_r_ext(NULL, d, &last, DQUOTE))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "Dies 'ist ein Test' 'fuer die' strtok-Funktionen\n");
    d = DELIM_WS;
    printf("String = %s", buffer);
    for (i = 0, p = strtok_r_ext(buffer, d, &last, SQUOTE);
         p; i++, p = strtok_r_ext(NULL, d, &last, SQUOTE))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "fido.de,de.answers,comp.misc.misc,alt.x.xx.xxx.xxxx");
    d = ",";
    printf("String = %s\n", buffer);
    for (i = 0, p = strtok_r_ext(buffer, d, &last, FALSE);
         p; i++, p = strtok_r_ext(NULL, d, &last, FALSE))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "\"12,5\",\"0,9\",\"13,75\",\"0,99\",\"1,23\"");
    d = ",";
    printf("String = %s\n", buffer);
    for (i = 0, p = strtok_r_ext(buffer, d, &last, QUOTE);
         p; i++, p = strtok_r_ext(NULL, d, &last, QUOTE))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "Dies ist \"ein Test\" \"fuer die\" strtok()-Funktion\n");
    d = DELIM_WS;
    printf("String = %s", buffer);
    for (i = 0, p = strtok(buffer, d); p; i++, p = strtok(NULL, d))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "Dies ist \"ein Test\" \"fuer die\" xstrtok()-Funktion\n");
    d = DELIM_WS;
    printf("String = %s", buffer);
    for (i = 0, p = xstrtok(buffer, d); p; i++, p = xstrtok(NULL, d))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "Origin\t\tDies ist ein Test\n");
    printf("String = %s", buffer);
    for (i = 0, p = xstrtok(buffer, DELIM_WS);
         p; i++, p = xstrtok(NULL, DELIM_EOL))
        printf("    %02d = [%s]\n", i, p);

    BUF_COPY(buffer, "Origin\t\t\"Dies ist ein Test\"\n");
    printf("String = %s", buffer);
    for (i = 0, p = xstrtok(buffer, DELIM_WS);
         p; i++, p = xstrtok(NULL, DELIM_EOL))
        printf("    %02d = [%s]\n", i, p);

    exit(0);
}
#endif /**TEST**/
