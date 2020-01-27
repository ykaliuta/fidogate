/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Active group
 *
 *****************************************************************************
 * Copyright (C) 2001-2002
 *
 *    Dmitry Fedotov            FIDO:      2:5030/1229
 *				Internet:  dyff@users.sourceforge.net
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

#ifdef ACTIVE_LOOKUP
#ifndef SN
/*
 * Prototypes
 */
static char *get_active_name(void);
static Active *active_parse_line(char *);

/* Groups linked list */
static Active *active_list = NULL;
static Active *active_last = NULL;

static char *get_active_name(void)
{
    static char active_name[MAXPATH];

    BUF_COPY3(active_name, cf_p_newsvardir(), "/", "active");
    if (check_access(active_name, CHECK_FILE) != TRUE) {
        return FALSE;
    }
    return active_name;
}

static Active *active_parse_line(char *line)
{
    Active *p;
    char *s;

    p = (Active *) xmalloc(sizeof(Active));
    p->next = NULL;
    s = xstrtok(line, " ");
    p->group = strsave(s);
    s = xstrtok(NULL, " ");
    p->art_h = atoi(s);
    s = xstrtok(NULL, " ");
    p->art_l = atoi(s);
    s = xstrtok(NULL, " ");
    if (!strncmp(s, "=", 1)) {
        p->group = strsave(s++);
        p->flag = "=";
    } else
        p->flag = strsave(s);

    return p;
}

short active_init(void)
{

    FILE *active;
    Active *p;
    char *name;

    name = get_active_name();
    if (name == FALSE)
        return ERROR;

    debug(14, "Reading active file %s", name);

    active = fopen(name, R_MODE);
    if (active != NULL) {
        while (fgets(buffer, BUFFERSIZE, active)) {
            strip_crlf(buffer);
            p = active_parse_line(buffer);
            if (p) {
                if (active_list)
                    active_last->next = p;
                else
                    active_list = p;
                active_last = p;
            }
        }
        fclose(active);
    } else {
        fglog("$ERROR: open news active file %s failed", name);
        return ERROR;
    }

    return OK;
}

Active *active_lookup(char *group)
{
    Active *p;
    /*
     * Inefficient search, but order is important!
     */
    for (p = active_list; p; p = p->next) {
        if (group && !strcmp(group, p->group))
            return p;
    }
    return FALSE;
}
#else
Active *active_lookup(char *group)
{
    Active *p;
    char sndir[MAXPATH];

    BUF_COPY3(sndir, cf_p_newsspooldir(), "/", group);
    if (check_access(sndir, CHECK_DIR) == TRUE) {
        p = (Active *) xmalloc(sizeof(Active));
        p->next = NULL;
        p->group = strsave(group);
        BUF_APPEND(sndir, "/.outgoing");
        if (check_access(sndir, CHECK_DIR) == TRUE)
            p->flag = "y";
        else
            p->flag = "n";
        return p;
    }
    return FALSE;
}
#endif                          /* SN */
#endif                          /* ACTIVE_LOOKUP */
