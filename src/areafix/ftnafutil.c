/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Utility program for Areafix.
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
#include "getopt.h"

#define PROGRAM		"ftnafutil"
#define CONFIG		DEFAULT_CONFIG_MAIN

/*
 * Prototypes
 */
void areafix_init(int);
void areafix_auth_cmd(void);
char *areafix_areasbbs(void);
void areafix_set_areasbbs(char *name);
char *areafix_name(void);
Node *areafix_auth_node(void);
void areafix_set_changed(void);

int do_mail(Node *, char *, char *, AreaUplink *);
int do_mail_notify(Node *, char *, char *, int);
int do_areasbbs(int);
int do_cmd(char **);
void short_usage(void);
void usage(void);
#ifdef ACTIVE_LOOKUP
void rm_group(char *, Node *);
#endif                          /* ACTIVE_LOOKUP */
short int rulesup(char *);

/*
 * Global vars
 */
static int n_flag = FALSE;

static Textlist req;
static char areafix = TRUE;

time_t no_traffic_tmout = 0;
time_t request_tmout = 0;

extern char *areas_bbs;

char *fix_name;

/*
 * Subscribe to an area
 */
int do_mail(Node * node, char *area, char *s, AreaUplink * upl)
{
    char *to;

    if (!upl || !upl->password) {
        fglog("ERROR: no uplink password for %s, can't send request",
              znfp1(node));
        return ERROR;
    }

    to = upl->robotname && *upl->robotname ? upl->robotname : areafix_name();

    /* Send Areafix message */
    if (areafix) {
        fix_name = cf_get_string("AreaFixName", TRUE);
    } else
        fix_name = cf_get_string("FileFixName", TRUE);

    tl_appendf(&req, "%s,%s,%s,%s,%s%s",
               znfp1(node),
               to,
               fix_name ? fix_name : areafix_name(), upl->password, s, area);

    return TRUE;
}

#ifdef ACTIVE_LOOKUP
/*
 * Remove newsgroup
 */
void rm_group(char *area, Node * uplink)
{
    Active *p;
    Area *ar;

#ifndef SN
    active_init();
#endif

    cf_set_best(uplink->zone, uplink->net, uplink->node);
    if ((ar = areas_lookup(area, NULL, cf_addr()))) {
        if ((p = active_lookup(ar->group))) {
#ifndef SN
            debug(7, "Found: %s stat = %s, have %d article", p->group, p->flag,
                  p->art_h - p->art_l);
#else
            debug(7, "Found: %s stat = %s", p->group, p->flag);
#endif
            if (cf_get_string("AutoRemoveNG", TRUE)) {
                BUF_COPY2(buffer, "%N/ngoper remove ", ar->group);
                if (0 != run_system(buffer))
                    fglog("ERROR: can't remove newsgroup (rc != 0)");
            } else
                debug(8, "config: AutoRemoveNG not defuned");
        } else
            debug(8, "newsgroup %s not found in areas", ar->group);
    } else
        debug(8, "newsgroup for area %s not found in areas", area);
}
#endif                          /* ACTIVE_LOOKUP */

int do_mail_notify(Node * node, char *area, char *s, int days)
{
    char *to;

    to = "Sysop";

    /* Send Areafix message */
    tl_appendf(&req, "%s,%s,%s,Notify,"
               "You are unsubscribed from area %s\r\n"
               "    Reason: %s %i day(s)",
               znfp1(node),
               to, fix_name ? fix_name : areafix_name(), area, s, days);

    return TRUE;
}

/*
 * Process areas.bbs
 */
#define DO_DELETE	 0
#define DO_LISTGWLINKS	 2
#define DO_UNRESUBSCRIBE 3
#define DO_RESUBSCRIBE	 4
#define DO_EXPIRE	 5
#define DO_LISTAREATIME	 6

int do_areasbbs(int cmd)
{
    AreasBBS *p, *pl;
    LON *lon;
    Node *uplink;
    int n;
    char *state;
    AreaUplink *a;
    int tm;
    int flag;
    LNode *l;
    char *blank_line = "";

    pl = NULL;
    p = areasbbs_first();
    while (p) {
        lon = &p->nodes;
        uplink = lon->first ? &lon->first->node : NULL;
        n = lon->size - 1;
        state = p->state ? p->state : blank_line;

        debug(3, "processing area %s: state=%s uplink=%s #dl=%d",
              p->area, state, uplink ? znfp1(uplink) : "(none)", n);

        flag = FALSE;
        switch (cmd) {
        case DO_DELETE:
            if (!uplink || areasbbs_isstate(state, 'U')) {
                fglog("area %s: no uplink or unsubscribe, deleting", p->area);
                if (!n_flag) {
                    areasbbs_remove(p, pl);
                    areafix_set_changed();
                    p = p->next;
                    continue;
                }
            }
            break;

        case DO_LISTGWLINKS:
            n = lon->size;
            cf_set_zone(p->zone);
            debug(5, "area %s, LON size %d, zone %d", p->area, n, p->zone);
            if (uplink && node_eq(uplink, cf_addr())) {
                /* 1st downlink is gateway, don't include in # of downlinks */
                n--;
                debug(5, "     # downlinks is %d", n);
            }
            printf("%s %s %d\n", p->area, uplink ? znfp1(uplink) : "-", n);
            break;

        case DO_EXPIRE:
            tm = (time(NULL) - p->time) / 86400;
            if ((areasbbs_isstate(state, 'F') ||
                 areasbbs_isstate(state, 'W')) &&
                (0 != p->time) &&
                ((0 != request_tmout && request_tmout < tm) ||
                 (p->expire_t > 0 && p->expire_t < tm))) {
                if (!(a = uplinks_line_get(areafix, uplink))) {
                    fglog("no uplinks record found for %s area %s", p->area,
                          znfp1(uplink));
                    break;
                }

                fglog
                    ("area %s: #dl=%d state=%s, unsubscribing at uplink %s (request timed out in %i day(s))",
                     p->area, n, state, znfp1(uplink), tm);
                if (do_mail(uplink, p->area, "-", a) != ERROR) {
                    while ((l = lon->first->next) != NULL) {
                        do_mail_notify(&l->node, p->area,
                                       "request is timed out in", tm);
                        lon_remove(lon, &l->node);
                    }
                    lon->first->next = NULL;
                    lon->last = lon->first;
                    lon->size = 1;
                    xfree(lon->sorted);
                    lon->sorted = NULL;
                    if (areasbbs_isstate(state, 'F')) {
                        fglog
                            ("area %s: forwarded request is timed out, deleted",
                             p->area);
                        areasbbs_remove(p, pl);
#ifdef ACTIVE_LOOKUP
                        rm_group(p->area, uplink);
#endif                          /* ACTIVE_LOOKUP */
                        flag = TRUE;
                    } else {
#ifdef ACTIVE_LOOKUP
                        rm_group(p->area, uplink);
#endif                          /* ACTIVE_LOOKUP */
                        areasbbs_chstate(&state, "W", 'U');
                        p->state = strsave(state);
                        p->time = time(NULL);
                    }
                    areafix_set_changed();
                }
            }

            else if (areasbbs_isstate(state, 'S') &&
                     (0 != p->time) &&
                     ((0 != no_traffic_tmout && no_traffic_tmout < tm) ||
                      (p->expire_n > 0 && p->expire_n < tm))) {

                if (!(a = uplinks_line_get(areafix, uplink))) {
                    fglog("no uplinks record found for %s area %s", p->area,
                          znfp1(uplink));
                    break;
                }
                fglog
                    ("area %s: #dl=%d state=%s, unsubscribing at uplink %s (no traffic for %i day(s))",
                     p->area, n, state, znfp1(uplink), tm);
                if (do_mail(uplink, p->area, "-", a) != ERROR) {
                    while ((l = lon->first->next) != NULL) {
                        do_mail_notify(&l->node, p->area, "no traffic for", tm);
                        lon_remove(lon, &l->node);
                    }
                    lon->first->next = NULL;
                    lon->last = lon->first;
                    lon->size = 1;
                    xfree(lon->sorted);
                    lon->sorted = NULL;
                    areasbbs_chstate(&state, "S", 'U');
                    p->state = strsave(state);
                    p->time = time(NULL);
                    areafix_set_changed();
                }
            } else if (areasbbs_isstate(state, 'S') && n < 1 &&
                       p->flags & AREASBBS_PASSTHRU) {
                if (!(a = uplinks_line_get(areafix, uplink))) {
                    fglog("no uplinks record found for %s area %s", p->area,
                          znfp1(uplink));
                    break;
                }
                fglog
                    ("area %s: #dl=%d state=%s, unsubscribing at uplink %s (no downlinks)",
                     p->area, n, state, znfp1(uplink));
                if (do_mail(uplink, p->area, "-", a) != ERROR) {
                    lon->first->next = NULL;
                    lon->last = lon->first;
                    lon->size = 1;
                    xfree(lon->sorted);
                    lon->sorted = NULL;
                    areasbbs_chstate(&state, "S", 'U');
                    p->state = strsave(state);
                    p->time = time(NULL);
                    areafix_set_changed();
                }
            }
            break;

        case DO_RESUBSCRIBE:
            if (uplink && (areasbbs_isstate(state, 'S') ||
                           areasbbs_isstate(state, 'F')
                           || areasbbs_isstate(state, 'W')
                           || state == blank_line)) {
                if (!(a = uplinks_line_get(areafix, uplink))) {
                    fglog("no uplinks record found for %s area %s", p->area,
                          znfp1(uplink));
                    break;
                }
                fglog("area %s: #dl=%d state=%s, resubscribing at uplink %s",
                      p->area, n, state, znfp1(uplink));
                p->time = time(NULL);
                areafix_set_changed();
                do_mail(uplink, p->area, "+", a);
            }
            break;

        case DO_UNRESUBSCRIBE:
            if (!(a = uplinks_line_get(areafix, uplink))) {
                fglog("no uplinks record found for %s area %s", p->area,
                      znfp1(uplink));
                break;
            }

            do_mail(uplink, p->area, "-", a);
            if (uplink && (areasbbs_isstate(state, 'S') ||
                           areasbbs_isstate(state, 'F') ||
                           areasbbs_isstate(state, 'W'))) {
                fglog("area %s: #dl=%d state=%s, resubscribing at uplink %s",
                      p->area, n, state, znfp1(uplink));
                if (uplink) {
                    p->time = time(NULL);
                    areafix_set_changed();
                    do_mail(uplink, p->area, "+", a);
                }
            }
            break;
        case DO_LISTAREATIME:
            cf_set_zone(p->zone);
            printf("area %s, state = %s, zone %d, %s",
                   p->area, state, p->zone,
                   p->time ? ctime(&p->time) : "time_none\n");
            break;

        default:
            return ERROR;
        /**NOT REACHED**/
            break;
        }

        /* Next */
        if (!flag)
            pl = p;
        p = p->next;

        tmps_freeall();
    }

    return OK;
}

/*
 * Do command
 */
int do_cmd(char *cmd[])
{
    if (cmd[0]) {
        if (strieq(cmd[0], "delete"))
            return do_areasbbs(DO_DELETE);
        else if (strieq(cmd[0], "resubscribe"))
            return do_areasbbs(DO_RESUBSCRIBE);
        else if (strieq(cmd[0], "unresubscribe"))
            return do_areasbbs(DO_UNRESUBSCRIBE);
        else if (strieq(cmd[0], "listgwlinks")) {
            cf_i_am_a_gateway_prog();
            cf_debug();
            return do_areasbbs(DO_LISTGWLINKS);
        } else if (strieq(cmd[0], "expire")) {
#ifdef ACTIVE_LOOKUP
            areas_init();
#endif                          /* ACTIVE_LOOKUP */
            return do_areasbbs(DO_EXPIRE);
        } else if (strieq(cmd[0], "listareatime"))
            return do_areasbbs(DO_LISTAREATIME);
        else if (strieq(cmd[0], "rulesup"))
            return rulesup(NULL);
        else {
            fprintf(stderr, "%s: illegal command %s\n", PROGRAM, cmd[0]);
            return ERROR;
        }
    }

    return OK;
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] command ...\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] command ...\n\n", PROGRAM);
    fprintf(stderr, "\
options: -n --noaction                don't really do anything ;-)\n\
         -b --areas-bbs NAME          use alternate AREAS.BBS\n\
         -F --filefix                 run as Filefix program (FAREAS.BBS)\n\
         -O --out-dir DIR             set output packet directory\n\
\n\
         -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n\
	 -w --wait [TIME]             wait for areas.bbs lock to be released\n\
\n\
command: delete         delete dead areas (no uplink or downlink)\n\
         expire         delete expired areas\n\
         listgwlinks    list areas with number of ext. links (excl. gateway)\n\
	 resubscribe	subscribe to all areas from uplink\n\
	 unresubscribe  unscribe and subscribe to all areas from uplink\n\
	 listareatime	list areas with time last message\n\
	 rulesup	update echo rules database\n");
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    char *O_flag = NULL;
    int ret = 0;
    int w_flag = FALSE;
    char *p;
    char bbslock[MAXPATH];

    int option_index;
    static struct option long_options[] = {
        {"no-rewrite", 0, 0, 'n'},
        {"areas-bbs", 1, 0, 'b'},
        {"filefix", 0, 0, 'F'},
        {"out-dir", 1, 0, 'O'}, /* Set packet directory */

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"uplink-addr", 1, 0, 'u'}, /* Set FIDO uplink address */
        {"wait", 1, 0, 'w'},
        {0, 0, 0, 0}
    };

    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "nb:FO:vhc:w:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftnaf options *****/
        case 'n':
            n_flag = TRUE;
            break;
        case 'b':
            areafix_set_areasbbs(optarg);
            break;
        case 'F':
            areafix = FALSE;
            break;
        case 'O':
            O_flag = optarg;
            break;

    /***** Common options *****/
        case 'v':
            verbose++;
            break;
        case 'h':
            usage();
            return 0;
            break;
        case 'c':
            c_flag = optarg;
            break;
        case 'a':
            a_flag = optarg;
            break;
        case 'u':
            u_flag = optarg;
            break;
        case 'w':
            if (optarg)
                w_flag = atoi(optarg);
            else
                w_flag = WAIT;
            break;
        default:
            short_usage();
            return EX_USAGE;
            break;
        }

    /* Read config file */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /* Process config options */
    if (a_flag)
        cf_set_addr(a_flag);
    if (u_flag)
        cf_set_uplink(u_flag);
    cf_debug();

    if (areafix) {
        if ((p = cf_get_string("AreaFixNoTrafficTimeout", TRUE))) {
            no_traffic_tmout = atoi(p);
        }
        if ((p = cf_get_string("AreaFixRequestTimeout", TRUE))) {
            request_tmout = atoi(p);
        }
    } else {
        if ((p = cf_get_string("FileFixNoTrafficTimeout", TRUE))) {
            no_traffic_tmout = atoi(p);
        }
        if ((p = cf_get_string("FileFixRequestTimeout", TRUE))) {
            request_tmout = atoi(p);
        }
    }

    /* Process local options */
    pkt_outdir(O_flag ? O_flag : cf_p_outpkt(), NULL);

    /* Common init */
    areafix_init(areafix);

    /* Read UPLINKS */
    uplinks_init();

    /* Read Active file */
#if defined ACTIVE_LOOKUP && !defined SN
    active_init();
#endif                          /*  ACTIVE_LOOKUP */

    tl_init(&req);

    ret = 0;

    /*
     * Process command on command line
     */
    if (optind >= argc) {
        fprintf(stderr, "%s: expecting command\n", PROGRAM);
        short_usage();
    }

    BUF_COPY2(bbslock, areas_bbs, ".lock");
    if (lock_path(bbslock, w_flag ? w_flag : NOWAIT) == ERROR) {
        exit_free();
        return EXIT_BUSY;
    }

    if (areasbbs_init(areafix_areasbbs()) == ERROR) {
        exit_free();
        return EX_OSFILE;
    }

    if (do_cmd(argv + optind) == ERROR)
        ret = EX_DATAERR;

    if (req.n > 0)
        send_request(&req);

    if (ret == 0 && !n_flag)
        if (!areasbbs_rewrite())
            ret = EX_CANTCREAT;

    unlock_path(bbslock);

    exit_free();
    return ret;
}
