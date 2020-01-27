/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Read PASSWD file for ftnaf and other programs
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
 * Local prototypes
 */
static Passwd *passwd_parse_line(char *);
static int passwd_do_file(char *);

static Passwd *passwd_list = NULL;
static Passwd *passwd_last = NULL;

/*
 * Read list of passwords from CONFIGDIR/PASSWD file.
 *
 * Format:
 *     CONTEXT  Z:N/F.P  PASSWORD  [ optional args ]
 */
static Passwd *passwd_parse_line(char *buf)
{
    Passwd *p;
    char *c, *n, *w, *r;

    c = strtok(buf, " \t");
    n = strtok(NULL, " \t");
    w = strtok(NULL, " \t");
    r = strtok(NULL, "");
    while (r && *r && is_space(*r))
        r++;
    if (!c || !n)
        return NULL;
    if (strieq(c, "include")) {
        passwd_do_file(n);
        return NULL;
    }
    if (!w)
        return NULL;

    p = (Passwd *) xmalloc(sizeof(Passwd));

    p->context = strsave(c);
    asc_to_node(n, &p->node, FALSE);
    p->passwd = strsave(w);
    p->args = r ? strsave(r) : NULL;
    p->next = NULL;

    debug(15, "passwd: %s %s %s", p->context, znfp1(&p->node), p->passwd);

    return p;
}

static int passwd_do_file(char *name)
{
    FILE *fp;
    Passwd *p;

    debug(14, "Reading passwd file %s", name);

    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if (!fp)
        return ERROR;

    while (cf_getline(buffer, BUFFERSIZE, fp)) {
        p = passwd_parse_line(buffer);
        if (!p)
            continue;

        /*
         * Put into linked list
         */
        if (passwd_list)
            passwd_last->next = p;
        else
            passwd_list = p;
        passwd_last = p;
    }

    fclose(fp);

    return OK;
}

void passwd_init(void)
{
    passwd_do_file(cf_p_passwd());
}

/*
 * Lookup password in list
 */
Passwd *passwd_lookup(char *context, Node * node)
{
    Passwd *p;

    for (p = passwd_list; p; p = p->next)
        if (!strcmp(context, p->context) && node_eq(node, &p->node))
            return p;

    return NULL;
}

void passwd_free(void)
{
    Passwd *p, *n;

    for (p = passwd_list; p; p = n) {
        n = p->next;

        xfree(p->context);
        xfree(p->passwd);
        xfree(p->args);
        xfree(p);
    }

}
