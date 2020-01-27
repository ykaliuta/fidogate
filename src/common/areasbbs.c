/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Function for processing AREAS.BBS EchoMail distribution file.
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
#include <fcntl.h>

/*
 * Number of old AREAS.BBS to keep as AREAS.Onn
 */
#define N_HISTORY	5

static char *areasbbs_1stline = NULL;
static AreasBBS *areasbbs_list = NULL;
static AreasBBS *areasbbs_last = NULL;
static char *areasbbs_filename = NULL;
static int areasbbs_changed_flag = FALSE;

/*
 * Remove area from areas.bbs
 */
void areasbbs_remove(AreasBBS * cur, AreasBBS * prev)
{
    if (!cur)
        return;

    if (prev)
        prev->next = cur->next;
    else
        areasbbs_list = cur->next;
    if (areasbbs_last == cur)
        areasbbs_last = prev;
}

void areasbbs_remove1(AreasBBS * cur)
{
    AreasBBS *p1, *p2;

    if (!cur)
        return;

    p1 = NULL;
    p2 = areasbbs_list;
    while (p2) {
        if (cur == p2) {
            areasbbs_remove(p2, p1);
            return;
        }
        p1 = p2;
        p2 = p2->next;
    }
}

/*
 * Alloc and init new AreasBBS struct
 */
AreasBBS *areasbbs_new(void)
{
    AreasBBS *p;

    p = (AreasBBS *) xmalloc(sizeof(AreasBBS));

    /* Init */
    p->next = NULL;
    p->flags = 0;
    p->dir = NULL;
    p->area = NULL;
    p->zone = -1;
    node_invalid(&p->addr);
    p->lvl = -1;
    p->key = NULL;
    p->desc = NULL;
    p->state = NULL;
    lon_init(&(p->passive));
    p->time = 0;
    p->expire_n = 0;
    p->expire_t = 0;
    p->msgs_in = 0;
    p->msgs_out = 0;
    p->msgs_dupe = 0;
    p->msgs_routed = 0;
    p->msgs_insecure = 0;
#ifdef FTN_ACL
    p->msgs_readonly = 0;
#endif                          /* FTN_ACL */
    p->msgs_path = 0;
    p->msgs_size = 0;
    lon_init(&p->nodes);
    p->uplinks = 1;

    return p;
}

/*
 * Add nodes from string to list of nodes
 */
static int areasbbs_add_string(LON * lon, LON * lon_passive, char *p)
{
    Node node, old;
    int ret;

    old.zone = cf_zone();
    old.net = old.node = old.point = -1;
    lon->size = 0;

    ret = OK;
    for (; p; p = xstrtok(NULL, " \t\r\n")) {
        if ('P' == *p) {
            lon_add(lon_passive, &node);
            p++;
        }
        if (asc_to_node_diff(p, &node, &old) == OK) {
            old = node;
            lon_add(lon, &node);
        } else {
            ret = ERROR;
            break;
        }
    }

    return ret;
}

/*
 *
 */
int areasbbs_add_passive(LON * lon, char *p)
{
    Node node, old;
    int ret;
    char *p2;

    old.zone = cf_zone();
    old.net = old.node = old.point = -1;

    ret = OK;
    while (p) {
        p2 = strchr(p, ',');
        if (p2)
            *p2++ = '\0';
        if (asc_to_node_diff(p, &node, &old) == OK) {
            old = node;
            lon_add(lon, &node);
        } else {
            ret = ERROR;
            break;
        }
        p = p2;
    }

    return ret;
}

/*
 * Create new AreasBBS struct for line from AREAS.BBS
 */
static AreasBBS *areasbbs_parse_line(char *line)
{
    AreasBBS *p;
    char *dir, *tag, *nl, *o2;

    dir = xstrtok(line, " \t\r\n");
    tag = xstrtok(NULL, " \t\r\n");
    if (!dir || !tag)
        return NULL;

    /* New areas.bbs entry */
    p = areasbbs_new();

    if (*dir == '#') {
        p->flags |= AREASBBS_PASSTHRU;
        dir++;
    }
    p->dir = strsave(dir);
    p->area = strsave(tag);

    /*
     * Options:
     *
     *     -a Z:N/F.P    alternate AKA for this area
     *     -z ZONE       alternate zone AKA for this area
     *     -l LVL        Areafix access level
     *     -k KEY        Areafix access key
     *     -d DESC       Area description text
     *     -#            Passthru
     +     -r            Read-only for new downlinks
     *     -p LIST       List of write-only (passive) links
     *     -s STAT   Area status
     *     -u NUM    Uplinks number
     */
    nl = xstrtok(NULL, " \t\r\n");
    while (nl && *nl == '-') {
        if (streq(nl, "-a")) {  /* -a Z:N/F.P */
            o2 = xstrtok(NULL, " \t\r\n");
            asc_to_node(o2, &p->addr, FALSE);
        }

        if (streq(nl, "-z")) {  /* -z ZONE */
            o2 = xstrtok(NULL, " \t\r\n");
            p->zone = atoi(o2);
        }

        if (streq(nl, "-l")) {  /* -l LVL */
            o2 = xstrtok(NULL, " \t\r\n");
            p->lvl = atoi(o2);
        }

        if (streq(nl, "-k")) {  /* -k KEY */
            o2 = xstrtok(NULL, " \t\r\n");
            p->key = strsave(o2);
        }

        if (streq(nl, "-d")) {  /* -d DESC */
            o2 = xstrtok(NULL, " \t\r\n");
            p->desc = strsave(o2);
        }

        if (streq(nl, "-s")) {  /* -s STATE */
            o2 = xstrtok(NULL, " \t\r\n");
            p->state = strsave(o2);
        }

        if (streq(nl, "-#")) {  /* -# */
            p->flags |= AREASBBS_PASSTHRU;
        }

        if (streq(nl, "-r")) {  /* -r */
            p->flags |= AREASBBS_READONLY;
        }

        if (streq(nl, "-p")) {  /* -p LIST */
            o2 = xstrtok(NULL, " \t\r\n");
            areasbbs_add_passive(&(p->passive), o2);
        }

        if (streq(nl, "-t")) {  /* -t TIME */
            o2 = xstrtok(NULL, " \t\r\n");
            p->time = atol(o2);
        }

        if (streq(nl, "-e")) {  /* -e DAYS */
            o2 = xstrtok(NULL, " \t\r\n");
            p->expire_n = atoi(o2);
        }
        if (streq(nl, "-n")) {  /* -n DAYS */
            o2 = xstrtok(NULL, " \t\r\n");
            p->expire_t = atoi(o2);
        }
        if (streq(nl, "-u")) {  /* -u NUM */
            o2 = xstrtok(NULL, " \t\r\n");
            p->uplinks = atoi(o2);
        }
        nl = xstrtok(NULL, " \t\r\n");
    }

    areasbbs_add_string(&(p->nodes), &(p->passive), nl);

    if (p->zone == -1)
        p->zone = p->nodes.first ? p->nodes.first->node.zone : 0;

    return p;
}

/*
 * Read area distribution list from AREAS.BBS file
 *
 * Format:
 *    [#$]DIR AREA [-options] Z:N/F.P Z:N/F.P ...
 */
int areasbbs_init(char *name)
{
    FILE *fp;
    AreasBBS *p;

    if (!name)
        return ERROR;

    debug(14, "Reading %s file", name);

    areasbbs_filename = name;
    areasbbs_changed_flag = FALSE;

    fp = fopen_expand_name(name, R_MODE, FALSE);
    if (!fp) {
        if (errno == ENOENT) {
            debug(14, "No file %s, assuming empty", name);
            return OK;
        } else {
            return ERROR;
        }
    }

    /*
     * 1st line is special
     */
    if (fgets(buffer, BUFFERSIZE, fp)) {
        strip_crlf(buffer);
        areasbbs_1stline = strsave(buffer);
    }

    /*
     * The following lines are areas and linked nodes
     */
    while (fgets(buffer, BUFFERSIZE, fp)) {
        strip_crlf(buffer);
        p = areasbbs_parse_line(buffer);
        if (!p)
            continue;

        debug(15, "areas.bbs: %s %s Z%d", p->dir, p->area, p->zone);

        /*
         * Put into linked list
         */
        if (areasbbs_list)
            areasbbs_last->next = p;
        else
            areasbbs_list = p;
        areasbbs_last = p;
    }

    fclose(fp);

    return OK;
}

/*
 * Output AREAS.BBS, format short sorted list of downlink
 */
int areasbbs_print(FILE * fp)
{
    AreasBBS *p;

    fprintf(fp, "%s\n", areasbbs_1stline);

    for (p = areasbbs_list; p; p = p->next) {
        if (p->flags & AREASBBS_PASSTHRU)
            fprintf(fp, "#");
        fprintf(fp, "%s %s ", p->dir, p->area);
        if (p->zone != -1)
            fprintf(fp, "-z %d ", p->zone);
        if (p->addr.zone != -1)
            fprintf(fp, "-a %s ", znfp1(&p->addr));
        if (p->lvl != -1)
            fprintf(fp, "-l %d ", p->lvl);
        if (p->key)
            fprintf(fp, "-k %s ", p->key);
        if (0 < p->passive.size) {
            fprintf(fp, "-p ");
            lon_print_passive(&(p->passive), fp);
            fprintf(fp, " ");
        }
        fprintf(fp, "-t %lu ", (unsigned long)(p->time));
        if (p->expire_n)
            fprintf(fp, "-e %d ", p->expire_n);
        if (p->expire_t)
            fprintf(fp, "-n %d ", p->expire_t);
        if (p->desc)
            fprintf(fp, "-d \"%s\" ", p->desc);
        if (p->state)
            fprintf(fp, "-s %s ", p->state);
        if (p->uplinks > 1)
            fprintf(fp, "-u %d ", p->uplinks);
        lon_print_sorted(&p->nodes, fp, p->uplinks);
        fprintf(fp, "\n");
    }

    return ferror(fp);
}

/*
 * Return areasbbs_list
 */
AreasBBS *areasbbs_first(void)
{
    return areasbbs_list;
}

/*
 * Rewrite AREAS.BBS if changed
 */
int areasbbs_rewrite(void)
{
    char old[MAXPATH], new[MAXPATH];
    int i, ovwr;
    FILE *fp;

    if (!areasbbs_changed_flag) {
        debug(4, "AREAS.BBS not changed");
        return OK;
    }

    /*
     * Base name
     */
    if (!areasbbs_filename) {
        fglog("$ERROR: unable to rewrite areas.bbs");
        return ERROR;
    }

    str_expand_name(buffer, MAXPATH, areasbbs_filename);
    ovwr = strlen(buffer) - 3;  /* 3 = extension "bbs" */
    if (ovwr < 0)               /* Just to be sure */
        ovwr = 0;

    /*
     * Write new one as AREAS.NEW
     */
    BUF_COPY(new, buffer);
    str_copy(new + ovwr, sizeof(new) - ovwr, "new");
    debug(4, "Writing %s", new);

    if ((fp = fopen(new, W_MODE)) == NULL) {
        fglog("$ERROR: can't open %s for writing AREAS.BBS", new);
        return ERROR;
    }
    if (areasbbs_print(fp) == ERROR) {
        fglog("$ERROR: writing to %s", new);
        fclose(fp);
        unlink(new);
        return ERROR;
    }
    if (fclose(fp) == ERROR) {
        fglog("$ERROR: closing %s", new);
        unlink(new);
        return ERROR;
    }

    /*
     * Renumber saved AREAS.Onn
     */
    BUF_COPY(old, buffer);
    sprintf(old + ovwr, "o%02d", N_HISTORY);
    debug(4, "Removing %s", old);
    unlink(old);
    for (i = N_HISTORY - 1; i >= 1; i--) {
        BUF_COPY(old, buffer);
        sprintf(old + ovwr, "o%02d", i);
        BUF_COPY(new, buffer);
        sprintf(new + ovwr, "o%02d", i + 1);
        debug(4, "Renaming %s -> %s", old, new);
        rename(old, new);
    }

    /*
     * Rename AREAS.BBS -> AREAS.O01
     */
    BUF_COPY(old, buffer);
    str_copy(old + ovwr, sizeof(old) - ovwr, "bbs");
    BUF_COPY(new, buffer);
    str_copy(new + ovwr, sizeof(new) - ovwr, "o01");
    debug(4, "Renaming %s -> %s", old, new);
    rename(old, new);

    /*
     * Rename AREAS.NEW -> AREAS.BBS
     */
    BUF_COPY(old, buffer);
    str_copy(old + ovwr, sizeof(old) - ovwr, "new");
    BUF_COPY(new, buffer);
    str_copy(new + ovwr, sizeof(new) - ovwr, "bbs");
    debug(4, "Renaming %s -> %s", old, new);
    rename(old, new);

    fglog("%s changed", buffer);

    return OK;
}

void areasbbs_changed(void)
{
    areasbbs_changed_flag = TRUE;
}

void areasbbs_not_changed(void)
{
    areasbbs_changed_flag = FALSE;
}

/*
 * Lookup area
 */
AreasBBS *areasbbs_lookup(char *area)
{
    AreasBBS *p;

    /**FIXME: the search method should use hashing or similar**/
    for (p = areasbbs_list; p; p = p->next) {
        if (area && !stricmp(area, p->area))
            return p;
    }

    return NULL;
}

/*
 * Add areas.bbs entry
 */
void areasbbs_add(AreasBBS * p)
{
    /* Put into linked list */
    if (areasbbs_list)
        areasbbs_last->next = p;
    else
        areasbbs_list = p;
    areasbbs_last = p;
}

int areasbbs_isstate(char *state, char st)
{
    if ((NULL == state) || ('\0' == st))
        return FALSE;

    if (NULL != strchr(state, st))
        return TRUE;
    else
        return FALSE;
}

/* Returns:
 *	TRUE  -> state changed
 *	FALSE -> state not changed
 */
int areasbbs_chstate(char **state, char *stold, char stnew)
{
    char *p, *p2;
    int i, j, len;

    if (NULL == state)
        return FALSE;

    /* No state -> just add new state */
    if ((NULL == *state) || ('\0' == **state)) {
        p = xmalloc(2);
        p[0] = stnew;
        p[1] = '\0';
        xfree(*state);
        *state = p;
        return TRUE;
    }

    /* Check if new state already set */
    if (NULL != strchr(*state, stnew))
        return FALSE;

    len = strlen(*state) + 2;   /* '\0' + 1 byte for state */
    p = xmalloc(len);
    memset(p, 0, len);
    p2 = *state;

    /* "old state" not given */
    if ((NULL == stold) || ('\0' == *stold)) {
        BUF_COPY(p, p2);
        j = len - 2;
    } else {
        for (i = 0, j = 0; '\0' != p2[i]; ++i)
            if (NULL == strchr(stold, p2[i]))
                p[j++] = p2[i];
    }

    p[j] = stnew;
    xfree(*state);
    *state = p;
    return TRUE;
}

void areasbbs_free(void)
{
    AreasBBS *p, *n;

    for (p = areasbbs_list; p; p = n) {
        n = p->next;
        xfree(p->area);
        xfree(p->dir);
        xfree(p->key);
        xfree(p->desc);
        xfree(p->state);

        if ((&p->passive)->size > 0)
            lon_delete(&p->passive);
        if ((&p->nodes)->size > 0)
            lon_delete(&p->nodes);
        xfree(p);
    }
    if (areasbbs_1stline)
        xfree(areasbbs_1stline);
}
