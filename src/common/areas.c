/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Area <-> newsgroups conversion
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
 * Prototypes
 */
static Area *area_build(Area *, char *, char *);
static void areas_init_xlate(void);
static int areas_do_file(char *);

/* Areas linked list */
static Area *area_list = NULL;
static Area *area_last = NULL;

/* Area <-> newsgroup char translation table */
static char areas_x_a[256];     /* Area -> newsgroup */
static char areas_x_g[256];     /* Newsgroup -> area */

/*
 * Initialize translation tables from config option "AreasXlate"
 */
static void areas_init_xlate(void)
{
    char *cf;
    unsigned char *x_a, *x_g, *p, *q;

    for (cf = cf_get_string("AreasXlate", TRUE);
         cf && *cf; cf = cf_get_string("AreasXlate", FALSE)) {
        /* Chars in area name */
        x_a = (unsigned char *)xstrtok(cf, " \t");
        /* Chars in newsgroup name */
        x_g = (unsigned char *)xstrtok(NULL, " \t");
        if (!x_a || !x_g)
            return;

        /* Fill table */
        p = x_a;
        q = x_g;
        while (*p || *q) {
            if (*p)
                areas_x_a[*p] = *q;
            if (*q)
                areas_x_g[*q] = *p;
            if (*p)
                p++;
            if (*q)
                q++;
        }
    }
}

/*
 * Default max/limit msg size
 */
static long areas_def_maxsize = MAXMSGSIZE; /* config.h */
static long areas_def_limitsize = 0;    /* default unlimited */

void areas_maxmsgsize(long int sz)
{
    areas_def_maxsize = sz;
}

long areas_get_maxmsgsize(void)
{
    return areas_def_maxsize;
}

void areas_limitmsgsize(long int sz)
{
    areas_def_limitsize = sz;
}

long areas_get_limitmsgsize(void)
{
    return areas_def_limitsize;
}

/*
 * Read list of areas from VARDIR/AREAS file.
 *
 * Format:
 *     AREA    NEWSGROUP    [-options]
 *
 * Options:
 *     -a Z:N/F.P       use alternate address for this area
 *     -z ZONE          use AKA for zone Z for this area
 *     -o ORIGIN        use alternate origin line for this area
 *     -d DISTRIBUTION  use Distribution: DISTRIBUTION for this newsgroup
 *     -l               only local xpostings allowed
 *     -x               no xpostings allowed
 *     -g               no messages from other gates FTN->Usenet
 *     -8               use ISO 8-bit umlauts
 *     -H               AREA/NEWSGROUP match entire hierarchy, names are
 *                      translated automatically
 *     -R LVL           ^ARFC header level
 *     -m MAXSIZE       set MaxMsgSize for this area (0 = infinity)
 *     -L LIMITSIZE     set LimitMsgSize for this area (0 = infinity)
 *     -X "Xtra: xyz"	add extra RFC header (multiple -X are allowed)
 *     -8               convert to 8bit iso-8859-1 characters
 *     -Q               convert to quoted-printable iso-8859-1 characters
 *     -C def:in:out    charset mapping setting
 *     -b               convert to base64, enabled by default
 *     -nh              do not encode or wrap headers
 */
Area *areas_parse_line(char *buf)
{
    Area *p;
    char *a, *g, *o;

    a = xstrtok(buf, " \t");    /* FTN area */
    g = xstrtok(NULL, " \t");   /* Newsgroup */
    if (a == NULL || g == NULL)
        return NULL;

    /* Check for include command */
    if (strieq(a, "include")) {
        areas_do_file(g);
        return NULL;
    }

    /* Create new areas entry */
    p = (Area *) xmalloc(sizeof(Area));
    p->next = NULL;
    p->area = strsave(a);
    p->group = strsave(g);
    p->zone = cf_defzone();
    node_invalid(&p->addr);
    p->origin = NULL;
    p->distribution = NULL;
    p->flags = 0;
    p->rfc_lvl = -1;
    p->maxsize = -1;
    p->limitsize = -1;
    tl_init(&p->x_hdr);
    p->charset = NULL;
    p->encoding = MIME_DEFAULT;

    /* Options */
    for (o = xstrtok(NULL, " \t"); o; o = xstrtok(NULL, " \t")) {
        if (!strcmp(o, "-a"))
            /* -a Z:N/F.P */
            if ((o = xstrtok(NULL, " \t")))
                asc_to_node(o, &p->addr, FALSE);
        if (!strcmp(o, "-z"))
            /* -z ZONE */
            if ((o = xstrtok(NULL, " \t")))
                p->zone = atoi(o);
        if (!strcmp(o, "-o"))
            /* -o ORIGIN */
            if ((o = xstrtok(NULL, " \t")))
                p->origin = strsave(o);
        if (!strcmp(o, "-d"))
            /* -d DISTRIBUTION */
            if ((o = xstrtok(NULL, " \t")))
                p->distribution = strsave(o);
        if (!strcmp(o, "-l"))
            p->flags |= AREA_LOCALXPOST;
        if (!strcmp(o, "-x"))
            p->flags |= AREA_NOXPOST;
        if (!strcmp(o, "-g"))
            p->flags |= AREA_NOGATE;
        if (!strcmp(o, "-H"))
            p->flags |= AREA_HIERARCHY;
        if (!strcmp(o, "-!"))
            p->flags |= AREA_NO;
        if (!strcmp(o, "-R"))
            /* -R lvl */
            if ((o = xstrtok(NULL, " \t")))
                p->rfc_lvl = atoi(o);
        if (!strcmp(o, "-m"))
            /* -m MAXMSGSIZE */
            if ((o = xstrtok(NULL, " \t")))
                p->maxsize = atol(o);
        if (!strcmp(o, "-L"))
            /* -L LIMITMSGSIZE */
            if ((o = xstrtok(NULL, " \t")))
                p->limitsize = atol(o);
        if (!strcmp(o, "-X"))
            /* -X "Xtra: xyz" */
            if ((o = xstrtok(NULL, " \t")))
                tl_append(&p->x_hdr, o);
        if (!strcmp(o, "-8"))
            p->encoding = MIME_8BIT;

        if (!strcmp(o, "-Q"))
            p->encoding = MIME_QP;

        if (!strcmp(o, "-b"))
            p->encoding = MIME_B64;

        if (!strcmp(o, "-C"))
            /* -C DEF:IN:OUT */
            if ((o = xstrtok(NULL, " \t")))
                p->charset = strsave(o);

        if (!strcmp(o, "-nh"))
            p->flags |= AREA_HEADERS_PLAIN;
    }
    /* Value not set or error */
    if (p->maxsize < 0)
        p->maxsize = areas_def_maxsize;
    if (p->limitsize < 0)
        p->limitsize = areas_def_limitsize;

    debug(15, "areas: %s %s Z=%d A=%s R=%d S=%ld",
          p->area, p->group, p->zone,
          p->addr.zone != -1 ? znfp1(&p->addr) : "", p->rfc_lvl, p->maxsize);

    return p;
}

static int areas_do_file(char *name)
{
    FILE *fp;
    Area *p;

    debug(14, "Reading areas file %s", name);

    fp = xfopen(name, R_MODE_T);

    while (cf_getline(buffer, BUFFERSIZE, fp)) {
        p = areas_parse_line(buffer);
        if (!p)
            continue;

        /*
         * Put into linked list
         */
        if (area_list)
            area_last->next = p;
        else
            area_list = p;
        area_last = p;
    }

    fclose(fp);

    return OK;
}

void areas_init(void)
{
    areas_init_xlate();
    areas_do_file(cf_p_areas());
}

/*
 * Lookup area/newsgroup in area_list
 *
 * Parameters:
 *     area, NULL     --- lookup by area
 *     NULL, group    --- lookup by group
 */
Area *areas_lookup(char *area, char *group, Node * aka)
{
    Area *p;

    /*
     * Inefficient search, but order is important!
     */
    for (p = area_list; p; p = p->next) {
        if (area && (!aka || node_eq(&p->addr, aka))) {
            if (p->flags & AREA_HIERARCHY) {
                if (0 == strlen(p->area)
                    || !strnicmp(area, p->area, strlen(p->area)))
                    return p->flags & AREA_NO ? NULL : area_build(p, area,
                                                                  group);
            } else {
                if (!stricmp(area, p->area))
                    return p->flags & AREA_NO ? NULL : p;
            }
        }

        if (group && group[0] == p->group[0]) {
            if (p->flags & AREA_HIERARCHY) {
                if (!strnicmp(group, p->group, strlen(p->group)))
                    return p->flags & AREA_NO ? NULL
                        : area_build(p, area, group);
            } else {
                if (!stricmp(group, p->group))
                    return p->flags & AREA_NO ? NULL : p;
            }
        }
    }

    return NULL;
}

/*
 * Build area/newsgroup name from hierarchy matching pattern
 */
static Area *area_build(Area * pa, char *area, char *group)
{
    static char bufa[MAXPATH], bufg[MAXPATH];
    static Area ret;
    char *p, *q, *end;

    *bufa = *bufg = 0;

    ret = *pa;
    ret.next = NULL;
    ret.area = bufa;
    ret.group = bufg;

    /* AREA -> Newsgroup */
    if (area) {                 /* Was searching for area */
        BUF_COPY(bufa, area);
        BUF_COPY(bufg, pa->group);
        p = bufg + strlen(bufg);
        end = bufg + sizeof(bufg) - 1;
        q = area + strlen(pa->area);

        for (; *q && p < end; q++, p++)
            if (areas_x_a[(unsigned char)*q])
                *p = areas_x_a[(unsigned char)*q];
            else
                *p = tolower(*q);
        *p = 0;
    }

    /* Newsgroup -> AREA */
    if (group) {                /* Was searching for newsgroup */
        BUF_COPY(bufa, pa->area);
        BUF_COPY(bufg, group);
        p = bufa + strlen(bufa);
        end = bufa + sizeof(bufa) - 1;
        q = group + strlen(pa->group);

        for (; *q && p < end; q++, p++)
            if (areas_x_g[(unsigned char)*q])
                *p = areas_x_g[(unsigned char)*q];
            else
                *p = toupper(*q);
        *p = 0;
    }

    return &ret;
}
