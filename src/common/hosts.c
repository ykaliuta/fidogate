/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Process hostname <-> node aliases from hosts file
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
static Host *hosts_parse_line(char *);
static int hosts_do_file(char *);

/*
 * Hosts list
 */
static Host *host_list = NULL;
static Host *host_last = NULL;

/*
 * Read list of hosts from CONFIGDIR/HOSTS file.
 *
 * Format:
 *     NODE	NAME	[-options]
 *
 * Options:
 *     -p	Addresses with pX point address
 *     y	dito, old compatibility
 *     -d	Host down
 */
static Host *hosts_parse_line(char *buf)
{
    Host *p;

    char *f, *n, *o;
    Node node;

    f = strtok(buf, " \t");     /* FTN address */
    n = strtok(NULL, " \t");    /* Internet address */
    if (f == NULL || n == NULL)
        return NULL;

    if (strieq(f, "include")) {
        hosts_do_file(n);
        return NULL;
    }

    if (asc_to_node(f, &node, FALSE) == ERROR) {
        fglog("hosts: illegal FTN address %s", f);
        return NULL;
    }

    p = (Host *) xmalloc(sizeof(Host));
    p->next = NULL;
    p->node = node;
    p->flags = 0;
    if (!strcmp(n, "-"))        /* "-" == registered, but no name */
        p->name = NULL;
    else {
        if (n[strlen(n) - 1] == '.') {  /* FQDN in HOSTS */
            n[strlen(n) - 1] = 0;
            p->name = strsave(n);
        } else {                /* Add domain */
            p->name = strsave2(n, cf_hostsdomain());
        }
    }

    for (o = strtok(NULL, " \t");   /* Options */
         o; o = strtok(NULL, " \t")) {
        if (!strcmp(o, "y")) {
            /* y == -p */
            p->flags |= HOST_POINT;
        }
        if (!strcmp(o, "-p")) {
            /* -p */
            p->flags |= HOST_POINT;
        }
        if (!strcmp(o, "-d")) {
            /* -d */
            p->flags |= HOST_DOWN;
        }
#ifdef AI_1
        if (!strcmp(o, "-a")) {
            /* -a */
            p->flags |= HOST_ADDR;
        }
#endif
    }

    debug(15, "hosts: %s %s %02x", znfp1(&p->node),
          p->name ? p->name : "-", p->flags);

    return p;
}

static int hosts_do_file(char *name)
{
    FILE *fp;
    Host *p;

    debug(14, "Reading hosts file %s", name);

    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if (!fp)
        return ERROR;

    while (cf_getline(buffer, BUFFERSIZE, fp)) {
        p = hosts_parse_line(buffer);
        if (!p)
            continue;

        /* Put into linked list */
        if (host_list)
            host_last->next = p;
        else
            host_list = p;
        host_last = p;
    }

    fclose(fp);

    return OK;
}

void hosts_init(void)
{
    hosts_do_file(cf_p_hosts());
}

/*
 * Lookup node/host in host_list
 *
 * Parameters:
 *     node, NULL     --- lookup by FTN address
 *     NULL, name     --- lookup by Internet address
 */
Host *hosts_lookup(Node * node, char *name)
{
    Host *p;

    /*
     * FIXME: the search method should use hashing or similar
     */

    for (p = host_list; p; p = p->next) {
        if (node)
            if (node->zone == p->node.zone &&
                node->net == p->node.net &&
                node->node == p->node.node &&
                (node->point == p->node.point || p->node.point == 0))
                return p;
        if (name && p->name && !stricmp(name, p->name))
            return p;
    }

    return NULL;
}

void hosts_free(void)
{
    Host *p, *n;

    for (p = host_list; p; p = n) {
        n = p->next;

        xfree(p->name);
        xfree(p);
    }
}
