/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Processing of FTN ^A kludges in message body
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
 * Process the addressing kludge lines in the message body:
 * ^ATOPT, ^AFMPT, ^AINTL. Remove these kludges from MsgBody and put
 * the information in Message.
 */
void kludge_pt_intl(MsgBody * body, Message * msg, short int del)
{
    Textline *line;
    Textlist *list;
    char *p, *s;
    Node node;

    list = &body->kludge;

    /* ^AINTL */
    if ((p = kludge_get(list, "INTL", &line))) {
        p = strsave(p);

        /* Retrieve addresses from ^AINTL */
        if ((s = strtok(p, " \t\r\n"))) /* Destination */
            if (asc_to_node(s, &node, FALSE) == OK)
                msg->node_to = node;
        if ((s = strtok(NULL, " \t\r\n")))  /* Source */
            if (asc_to_node(s, &node, FALSE) == OK)
                msg->node_from = node;

        xfree(p);

        if (del)
            tl_delete(list, line);
    }

    /* ^AFMPT */
    if ((p = kludge_get(list, "FMPT", &line))) {
        msg->node_from.point = atoi(p);

        if (del)
            tl_delete(list, line);
    }

    /* ^ATOPT */
    if ((p = kludge_get(list, "TOPT", &line))) {
        msg->node_to.point = atoi(p);

        if (del)
            tl_delete(list, line);
    }
}

/*
 * Gives kludge's value if Textline contains the kludge
 *
 * @tl -- full textline
 * @kname -- kludge name
 *
 * returns pointer to the (const) kludge value inside @tl,
 * or NULL if wrong kludge.
 */
static char *kludge_from_tline(Textline *tl, char *kname)
{
    char *s = tl->line;
    int len = strlen(kname);

    /* borrow the code from the original, just reuse s.
     * So, not reverted condition */
    if (s[0] == '\001' &&
        !strnicmp(s + 1, kname, len) &&
        (s[len + 1] == ' ' || s[len + 1] == ':')) {    /* Found it */

        s = s + 1 + len;
        /* Skip ':' and white space */
        if (*s == ':')
            s++;
        while (is_space(*s))
            s++;
        return s;
    }
    return NULL;
}

/*
 * Get first next kludge line
 */
char *kludge_get(Textlist * tl, char *name, Textline ** ptline)
{
    static Textline *last_kludge;
    char *r;

    last_kludge = tl->first;

    while (last_kludge) {
        r = kludge_from_tline(last_kludge, name);
        if (r != NULL) {
            if (ptline)
                *ptline = last_kludge;
            last_kludge = last_kludge->next;

            return r;
        }

        last_kludge = last_kludge->next;
    }

    /* Not found */
    if (ptline)
        *ptline = NULL;
    return NULL;
}
