/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Read user name aliases from file. The alias.users format is as follows:
 *	username    Z:N/F.P    Full Name
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

#define RFC2FTN 'r'
#define FTN2RFC 'f'
#define ALL	'a'

/*
 * Local prototypes
 */
#if 0
static int anodeeq(Node *, Node *);
#endif
static Alias *alias_parse_line(char *);

/*
 * Alias list
 */
static Alias *alias_list = NULL;
static Alias *alias_last = NULL;
static char type = ALL;

/*
 * Read list of aliases from CONFIGDIR/ALIASES file.
 *
 * Format:
 *     ALIAS	NODE	"FULL NAME"
 */
static Alias *alias_parse_line(char *buf)
{

    Alias *p = NULL;
    char *u, *n, *f;
    Node node;
    char *un, *ud;

    u = xstrtok(buf, " \t");
    if (u) {                    /* User name */
        n = xstrtok(NULL, " \t");   /* FTN node */
        f = xstrtok(NULL, " \t");   /* Full name */

        if (!stricmp(u, "rfc2ftn"))
            type = RFC2FTN;
        else if (!stricmp(u, "ftn2rfc"))
            type = FTN2RFC;
        else if (n != NULL) {
            if (strieq(u, "include")) {
                alias_do_file(n);
                return NULL;
            }
            if (f != NULL) {
                if (asc_to_node(n, &node, FALSE) == ERROR) {
                    fglog("hosts: illegal FTN address %s", n);
                    return NULL;
                }
                p = (Alias *) xmalloc(sizeof(Alias));
                p->next = NULL;
                p->node = node;
                un = xstrtok(u, "@");   /* User name */
                ud = xstrtok(NULL, " \t");  /* User domain */
                p->username = strsave(un);
                p->userdom = ud ? strsave(ud) : NULL;
                p->fullname = strsave(f);
                p->type = type;

                if (p->userdom)
                    debug(15, "aliases: %s@%s %s %s %c", p->username,
                          p->userdom, znfp1(&p->node), p->fullname, p->type);
                else
                    debug(15, "aliases: %s %s %s %c", p->username,
                          znfp1(&p->node), p->fullname, p->type);
            }
        }
    }
    return p;
}

void alias_do_file(char *name)
{

    FILE *fp;
    Alias *p;

    debug(14, "Reading aliases file %s", name);

    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if (fp) {

        while (cf_getline(buffer, BUFFERSIZE, fp)) {
            p = alias_parse_line(buffer);
            if (p) {
                /*
                 * Put into linked list
                 */
                if (alias_list)
                    alias_last->next = p;
                else
                    alias_list = p;
                alias_last = p;
            }
        }
        fclose(fp);
    }

    return;
}

/*
 * Lookup alias in alias_list
 *
 * Parameters:
 *     node, username, NULL     --- lookup by FTN node and username
 *     node, NULL    , fullname --- lookup by FTN node and fullname
 *
 * The lookup for node is handled somewhat special: if node->point !=0,
 * then the alias matching the complete address including point will
 * be found. If not, then the alias comparison ignores the point address.
 * e.g.:
 *     mj    2:2452/110.1    "Martin Junius"
 * An alias_lookup(2:2452/110.1, "mj", NULL) as well as
 * alias_lookkup(2:2452/110, "mj", NULL) will return this alias entry.
 */
Alias *alias_lookup(Node * node, char *username)
{
    Alias *a;

    for (a = alias_list; a; a = a->next) {
        if (a->type != FTN2RFC && username &&
            !stricmp(a->username, username) &&
            (!node || node_eq(node, &a->node)))
            return a;
    }
    return NULL;
}

Alias *alias_lookup_strict(Node * node, char *fullname)
{
    Alias *a;

    for (a = alias_list; a; a = a->next) {
        if (a->type != RFC2FTN && fullname &&
            (wildmatch(fullname, a->fullname, TRUE))
            && node_eq(node, &a->node))
            return a;
    }

    return NULL;
}

Alias *alias_lookup_userdom(RFCAddr * rfc)
{
    Alias *a;

    if (rfc) {
        for (a = alias_list; a; a = a->next) {
            if (a->type != FTN2RFC && a->userdom &&
                (!stricmp(a->username, rfc->user) &&
                 !stricmp(a->userdom, rfc->addr)))
                return a;
        }
    }
    return NULL;
}

/*
 * anodeeq() --- compare node adresses
 *
 * Special point treatment: if a->point != 0 and b->point !=0, compare the
 * complete FTN address including point. If not, compare ignoring the point
 * address.
 */
#if 0
static int anodeeq(Node * a, Node * b)
                        /* Sender/receiver address */
                        /* ALIASES address */
{
    return a->point && b->point
        ?
        a->zone == b->zone && a->net == b->net && a->node == b->node &&
        a->point == b->point
        : a->zone == b->zone && a->net == b->net && a->node == b->node;
}
#endif
