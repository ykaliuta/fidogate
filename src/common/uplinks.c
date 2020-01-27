/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Read area uplinks list from file. The format is as follows:
 *	ROBOT_TYPE   AREAS   Z:N/F.P   ROBOT_NAME    PASSWORD
 *
 *****************************************************************************
 * Copyright (C) 2000
 *
 * Oleg Derevenetz	     FIDO:	2:5025/3.4
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
 * Local prototypes
 */
static AreaUplink *uplinks_parse_line(char *);
static int uplinks_do_file(char *);
void uplinks_lookup_save(int, char *, const AreaUplink *);
int is_wildcard(char *);
#ifdef DEBUG
static int anodeeq(Node *, Node *);
static int uplinks_check_dups(int, Node *);
#endif                          /* DEBUG */

/*
 * Uplinks list
 */
static AreaUplink *uplinks_list = NULL;
static AreaUplink *uplinks_last = NULL;
static AreaUplink *upll_ap = NULL;
static AreaUplink *upll_ap_last = NULL;

/*
 * Read list of uplinks from CONFIGDIR/UPLINKS file.
 *
 * Format:
 *     ROBOT_TYPE   AREAS   NODE   ROBOT_NAME
 */
static AreaUplink *uplinks_parse_line(char *buf)
{
    AreaUplink *p;
    char *t, *a, *n, *f, *w;
    Node uplink;
    char *opt;

    t = xstrtok(buf, " \t");    /* Robot type (af-AreaFix, ff-FileFix) */
    a = xstrtok(NULL, " \t");   /* Areas */
    n = xstrtok(NULL, " \t");   /* Uplink */
    f = xstrtok(NULL, " \t");   /* Robot name */
    w = xstrtok(NULL, " \t");   /* Password */
    opt = xstrtok(NULL, "");    /* Options */
    if (t == NULL || a == NULL)
        return NULL;
    if (strieq(t, "include")) {
        uplinks_do_file(a);
        return NULL;
    }
    if (n == NULL || f == NULL || w == NULL)
        return NULL;

    if (asc_to_node(n, &uplink, FALSE) == ERROR) {
        fglog("uplinks: illegal FTN address %s", n);
        return NULL;
    }

#ifdef DEBUG
    if (uplinks_check_dups((!strcmp(t, "af")) ? TRUE : FALSE, &uplink)) {
        fglog("uplinks: duplicate uplink entry %s", n);
        return NULL;
    }
#endif                          /* DEBUG */
    p = (AreaUplink *) xmalloc(sizeof(AreaUplink));
    p->next = NULL;
    p->areafix = (!strcmp(t, "af")) ? TRUE : FALSE;
    p->areas = strsave(a);
    p->uplink = uplink;
    p->robotname = strsave(f);
    p->password = strsave(w);
    p->options = strsave(opt);

    debug(15, "uplinks: %s %s %s %s %s %s", p->areafix ? "af" : "ff",
          p->areas, znfp1(&p->uplink), p->robotname, p->password, p->options);

    return p;
}

static int uplinks_do_file(char *name)
{
    FILE *fp;
    AreaUplink *p;

    debug(14, "Reading uplinks file %s", name);

    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if (fp == NULL)
        return ERROR;

    while (cf_getline(buffer, BUFFERSIZE, fp)) {
        p = uplinks_parse_line(buffer);
        if (p == NULL)
            continue;

        /*
         * Put into linked list
         */
        if (uplinks_list)
            uplinks_last->next = p;
        else
            uplinks_list = p;
        uplinks_last = p;
    }

    fclose(fp);

    return OK;
}

#ifdef DEBUG
static int uplinks_check_dups(int areafix, Node * uplink)
{
    AreaUplink *a;

    for (a = uplinks_list; a; a = a->next) {
        if ((a->areafix == areafix) && anodeeq(uplink, &a->uplink))
            return TRUE;
    }

    return FALSE;
}
#endif                          /* DEBUG */

void uplinks_init(void)
{
    uplinks_do_file(cf_p_uplinks());
}

void uplinks_lookup_save(int afix, char *area, const AreaUplink * a)
{
    AreaUplink *list;

    list = (AreaUplink *) xmalloc(sizeof(AreaUplink));
    list->next = NULL;
    list->areafix = afix;
    list->areas = strsave(area);
    list->uplink = a->uplink;
    list->robotname = a->robotname;
    list->password = a->password;
    list->options = a->options;

    if (!upll_ap)
        upll_ap = list;
    else
        upll_ap_last->next = list;
    upll_ap_last = list;
}

void uplinks_lookup_free(void)
{
    AreaUplink *p, *s;

    for (p = upll_ap; p; p = s) {
        s = p->next;
        xfree(p->areas);
        p->next = NULL;
        xfree(p);
    }
    upll_ap = NULL;
}

/*
 * Lookup uplink for area
 *
 * Parameters:
 *     areafix --- robot type (areafix or filefix)
 *     area    --- lookup by area name
 *
 */
AreaUplink *uplinks_lookup(int areafix, char *area)
{
    const AreaUplink *a;
    char *t, *n, *f;
    int iswc;
    FILE *fp1;

    iswc = is_wildcard(area);

    for (a = uplinks_list; a; a = a->next) {
        if (a->areafix != areafix)
            continue;
        t = strsave(a->areas);
        for (n = strtok(t, ","); n; n = strtok(NULL, ",")) {
            if (*n == '/' || *n == '%' || *n == '.') {
                if ((fp1 = fopen_expand_name(n, R_MODE_T, FALSE))) {
                    while (cf_getline(buffer, BUFFERSIZE, fp1)) {
                        if (!(f = xstrtok(buffer, " \t")))
                            continue;

                        if (*f == '!') {
                            if (wildmatch(area, (char *)(&f[1]), TRUE + iswc)
                                && !iswc)
                                break;
                        } else {
                            if (wildmatch(area, f, TRUE + iswc)) {
                                uplinks_lookup_save(areafix, f, a);
                                if (!iswc) {
                                    fclose(fp1);
                                    return upll_ap;
                                }
                            }
                        }
                    }
                    fclose(fp1);
                }
            } else {
                if (*n == '!') {
                    if (wildmatch(area, (char *)(&n[1]), TRUE + iswc) && !iswc)
                        break;
                } else {
                    if (wildmatch(area, n, TRUE + iswc)) {
                        uplinks_lookup_save(areafix, n, a);
                        if (!iswc)
                            return upll_ap;
                    }
                }
            }
        }
        xfree(t);
    }

    return upll_ap;
}

AreaUplink *uplinks_line_get(int areafix, Node * uplink)
{
    AreaUplink *p1;

    for (p1 = uplinks_list; p1; p1 = p1->next) {
        if (p1->areafix != areafix)
            continue;
        if (node_eq(&p1->uplink, uplink))
            return p1;
    }
    return NULL;
}

AreaUplink *uplinks_first(void)
{
    return uplinks_list;
}

/*
 * anodeeq() --- compare node adresses
 *
 * Compare node FTN addresses.
 *
 */
#ifdef DEBUG
static int anodeeq(Node * a, Node * b)
{
    return a->zone == b->zone && a->net == b->net && a->node == b->node;
}
#endif

void uplinks_free(void)
{
    AreaUplink *p, *s;

    for (p = uplinks_list; p; p = s) {
        s = p->next;
        xfree(p->areas);
        xfree(p->robotname);
        xfree(p->password);
        xfree(p->options);
        p->next = NULL;
        xfree(p);
    }
}
