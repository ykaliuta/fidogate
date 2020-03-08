/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Toss FTN NetMail/EchoMail
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

#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#define PROGRAM 	"ftntoss"
#define CONFIG		DEFAULT_CONFIG_MAIN

/*
 * Prototypes
 */
void addtoseenby_init(void);
void zonegate_init(void);
void deleteseenby_init(void);
void deletepath_init(void);
int lon_to_kludge(Textlist *, char *, LON *);
void lon_to_kludge_sorted(Textlist *, char *, LON *);
int toss_echomail(Message *, MsgBody *, LON *, LON *, LON *, LON *);
void kludge_to_lon(Textlist *, LON *);
int node_eq_echo(Node *, Node *);
int node_eq_echo_pa(Node *, Node *);
int lon_search_echo(LON *, Node *);
int is_local_addr(Node *, char);
void do_seenby(LON *, LON *, LON *, LON *, int, Node *);
void do_path(LON *);
int do_check_path(LON *);
int do_bad_msg(Message *, MsgBody *);
Node *lon_first(LON *);
void do_4dpoint(LON *, LON *, Node *, Node *);
int do_echomail(Packet *, Message *, MsgBody *);
int do_unknown_area(char *, Message *, MsgBody *);
#ifdef DO_NOT_TOSS_NETMAIL
void add_via(Textlist *, Node *);
#endif                          /* DO_NOT_TOSS_NETMAIL */
void change_addr(Node *, Node *);
void do_rewrite(Message *);
int do_remap(Message *);
int check_empty(MsgBody *);
#ifndef SPYES
int do_netmail(Packet *, Message *, MsgBody *);
#else
int do_netmail(Packet *, Message *, MsgBody *, int);
#endif                          /* !SPYES */
int unpack(FILE *, Packet *);
int unpack_file(char *);
void prog_signal(int);
void short_usage(void);
void usage(void);

extern int authorized_new;
static char *areas_bbs = NULL;

void areafix_init(int);
int areafix_auth_check(Node *, char *, char);
int cmd_new_int(Node *, char *, char *);
int areafix_check_forbidden_area(char *);

/*
 * Command line options
 */
char g_flag = 'n';              /* Processing grade */
char t_flag = FALSE;            /* Insecure tossing enabled */
char n_flag = FALSE;            /* Accept EchoMail messages not */
                    /* addressed to our AKA         */
char *O_flag = NULL;            /* output packet directory */
char s_flag = FALSE;            /* Strip CRASH, HOLD attribute */
int maxmsg = 0;                 /* Process maxmsg messages */
char x_flag = FALSE;            /* Exit after maxmsg messages */
char l_flag = FALSE;            /* Create lock file */
char p_flag = FALSE;            /* -p --passthru */

static char in_dir[MAXPATH];    /* Input directory */
static char must_exit = FALSE;  /* Flag for -x operation */
static int msg_count = 0;       /* Counter for -m, -x operation */
static int severe_error = OK;   /* ERROR: exit after error */
static int signal_exit = FALSE; /* Flag: TRUE if signal received */

short int int_uplinks = FALSE;  /* Flag: TRUE if uplinks initialised */

/*
 * Config options
 */
char kill_empty = FALSE;        /* config: KillEmpty */
char kill_unknown = FALSE;      /* config: KillUnknown */
char kill_routed = FALSE;       /* config: KillRouted */
char kill_insecure = FALSE;     /* config: KillInsecure */
#ifdef FTN_ACL
char kill_readonly = FALSE;     /* config: KillReadonly */
#endif                          /* FTN_ACL */
char kill_circular = FALSE;     /* config: KillCircular */
char log_netmail = FALSE;       /* config: LogNetMail */
char check_path = FALSE;        /* config: CheckPath */
char dupe_check = FALSE;        /* config: DupeCheck */
char kill_dupe = FALSE;         /* config: KillDupe */
char kill_nomsgid = FALSE;      /* config: KillNoMSGID */
char kill_old = FALSE;          /* config: KillOld */
char echomail4d = FALSE;        /* config: EchoMail4D */
char no_empty_path = FALSE;     /* config: NoEmptyPath */
char add_other_aka = FALSE;     /* config: AddOtherAKAs */
#ifdef FTN_ACL
char uplink_can_be_readonly = FALSE;    /* config: UplinkCanBeReadonly */
#endif                          /* FTN_ACL */
char traffic_statistics = FALSE;    /* config: TrafficStatistics */
char *traffic_statistics_file = NULL;   /* config: TrafficStatisticsFile */
char *autocreate_line = "";     /* config: AutoCreateLine */
#ifndef ACTIVE_LOOKUP
char autocreate_ng = FALSE;     /* config: AutoCreateNG */
#endif                          /* !ACTIVE_LOOKUP */
#ifdef DO_NOT_TOSS_NETMAIL
char no_rewrite = FALSE;        /* config: NoRewrite */
#endif                          /* DO_NOT_TOSS_NETMAIL */

short check_point_origin_addr = FALSE;
/* Values checking for old messages */
static double max_history = 14; /* Max. number of days entry stays
                                   in MSGID history database */
static time_t max_sec = 0;
static time_t now_sec = 0;
static time_t exp_sec = 0;

/*
 * Global stat counts
 */
static long msgs_in = 0;        /* Input messages */
static long msgs_netmail = 0;   /* Output messages NetMail */
static long msgs_echomail = 0;  /* Output messages EchoMail */
static long msgs_routed = 0;    /* Routed EchoMail messages */
static long msgs_insecure = 0;  /* Insecure EchoMail messages */
#ifdef FTN_ACL
static long msgs_readonly = 0;  /* EchoMail messages to readonly areas */
#endif                          /* FTN_ACL */
static long msgs_unknown = 0;   /* Unknown EchoMail area messages */
static long msgs_empty = 0;     /* Empty NetMail messages */
static long msgs_path = 0;      /* Circular path */
static long msgs_dupe = 0;      /* Dupe */

static long pkts_in = 0;        /* Input packets */
static long pkts_bytes = 0;     /* Input bytes */

static Textlist notify;

/*
 * AddToSeenBy list
 */
typedef struct st_addtoseenby {
    char *area;                 /* Area pattern */
    LON add;                    /* Nodes to add to SEEN-BY */
    struct st_addtoseenby *next;
} AddToSeenBy;

static AddToSeenBy *addto_first = NULL;
static AddToSeenBy *addto_last = NULL;
static bool strict;

/*
 * Init AddToSeenBy list from config file
 */
void addtoseenby_init(void)
{
    char *s, *parea, *pnodes;
    AddToSeenBy *p;

    for (s = cf_get_string("AddToSeenBy", TRUE);
         s && *s; s = cf_get_string("AddToSeenBy", FALSE)) {
        BUF_COPY(buffer, s);
        parea = xstrtok(buffer, " \t");
        pnodes = xstrtok(NULL, "\n");

        if (!parea || !pnodes)
            continue;

        p = (AddToSeenBy *) xmalloc(sizeof(AddToSeenBy));
        p->next = NULL;
        p->area = strsave(parea);
        lon_init(&p->add);
        lon_add_string(&p->add, pnodes);

        if (addto_first)
            addto_last->next = p;
        else
            addto_first = p;
        addto_last = p;
    }
}

/*
 * DeleteSeenBy list
 */
typedef struct st_deleteseenby {
    LON deleteseenby;           /* Nodes to delete from SEEN-BY */
    struct st_deleteseenby *next;
} DeleteSeenBy;

static DeleteSeenBy *deleteseenby_first = NULL;
static DeleteSeenBy *deleteseenby_last = NULL;

/*
 * Init DeleteSeenBy list from config file
 */
void deleteseenby_init(void)
{
    char *s;
    DeleteSeenBy *p;

    for (s = cf_get_string("DeleteSeenBy", TRUE);
         s && *s; s = cf_get_string("DeleteSeenBy", FALSE)) {
        p = (DeleteSeenBy *) xmalloc(sizeof(DeleteSeenBy));
        p->next = NULL;
        lon_init(&p->deleteseenby);
        lon_add_string(&p->deleteseenby, s);

        if (deleteseenby_first)
            deleteseenby_last->next = p;
        else
            deleteseenby_first = p;
        deleteseenby_last = p;
    }
}

/*
 * DeletePath list
 */
typedef struct st_deletepath {
    LON deletepath;             /* Nodes to delete from PATH */
    struct st_deletepath *next;
} DeletePath;

static DeletePath *deletepath_first = NULL;
static DeletePath *deletepath_last = NULL;

/*
 * Init DeletePath list from config file
 */
void deletepath_init(void)
{
    char *s;
    DeletePath *p;

    for (s = cf_get_string("DeletePath", TRUE);
         s && *s; s = cf_get_string("DeletePath", FALSE)) {
        p = (DeletePath *) xmalloc(sizeof(DeletePath));
        p->next = NULL;
        lon_init(&p->deletepath);
        lon_add_string(&p->deletepath, s);

        if (deletepath_first)
            deletepath_last->next = p;
        else
            deletepath_first = p;
        deletepath_last = p;
    }
}

/*
 * Zonegate list
 */

static ZoneGate *zonegate_first = NULL;
static ZoneGate *zonegate_last = NULL;

/*
 * Init zonegate list
 */
void zonegate_init(void)
{
    char *s;
    ZoneGate *p;

    for (s = cf_get_string("ZoneGate", TRUE);
         s && *s; s = cf_get_string("ZoneGate", FALSE)) {
        p = (ZoneGate *) xmalloc(sizeof(ZoneGate));
        p->next = NULL;
        lon_init(&p->seenby);
        lon_add_string(&p->seenby, s);
        if (p->seenby.first) {
            p->node = p->seenby.first->node;
            lon_remove(&p->seenby, &p->node);
        } else
            node_invalid(&p->node);

        if (zonegate_first)
            zonegate_last->next = p;
        else
            zonegate_first = p;
        zonegate_last = p;
    }
}

/*
 * Create new SEEN-BY or ^APATH line from LON
 */
#define MAX_LENGTH 76

int lon_to_kludge(Textlist * tl, char *text, LON * lon)
{
    LNode *p;
    Node old;
    char *s = NULL;
    int n = 0;

    BUF_COPY(buffer, text);
    node_invalid(&old);
    old.zone = cf_zone();

    for (p = lon->first; p; p = p->next)
        if (p->node.point == 0 || echomail4d) { /* Normally, no 4D Points */
            p->node.zone = cf_zone();   /* No zone, use current one */
            s = node_to_asc_diff(&p->node, &old);
            old = p->node;

            if (strlen_zero(buffer) + strlen_zero(s) + 1 > MAX_LENGTH) {
                BUF_APPEND(buffer, "\r\n");
                tl_append(tl, buffer);

                node_invalid(&old);
                old.zone = cf_zone();

                s = node_to_asc_diff(&p->node, &old);
                old = p->node;
                BUF_COPY3(buffer, text, " ", s);
            } else
                BUF_APPEND2(buffer, " ", s);

            n = TRUE;
        }

    BUF_APPEND(buffer, "\r\n");
    tl_append(tl, buffer);

    return n;
}

void lon_to_kludge_sorted(Textlist * tl, char *text, LON * lon)
{
    Node old;
    char *s = NULL;
    int i;

    lon_sort(lon, FALSE);

    BUF_COPY(buffer, text);
    node_invalid(&old);
    old.zone = cf_zone();

    for (i = 0; i < lon->size; i++)
        if (lon->sorted[i]->point == 0 || echomail4d) { /* Normally, no 4D Points */
            lon->sorted[i]->zone = cf_zone();   /* No zone, use current one */
            s = node_to_asc_diff(lon->sorted[i], &old);
            old = *lon->sorted[i];

            if (strlen_zero(buffer) + strlen_zero(s) + 1 > MAX_LENGTH) {
                BUF_APPEND(buffer, "\r\n");
                tl_append(tl, buffer);

                node_invalid(&old);
                old.zone = cf_zone();

                s = node_to_asc_diff(lon->sorted[i], &old);
                old = *lon->sorted[i];
                BUF_COPY3(buffer, text, " ", s);
            } else
                BUF_APPEND2(buffer, " ", s);
        }

    BUF_APPEND(buffer, "\r\n");
    tl_append(tl, buffer);

    return;
}

/*
 * Toss EchoMail, writing message to packet for each downlink
 */
int toss_echomail(Message * msg, MsgBody * body, LON * seenby, LON * path,
                  LON * nodes, LON * passive)
{
    LNode *p;
    FILE *fp;
    Textlist save = { NULL, NULL, 0 };
    char is_saved;

    for (p = nodes->first; p; p = p->next) {
        is_saved = FALSE;

        if (lon_search(passive, &(p->node)))
            continue;

        debug(7, "toss_echomail(): message for %s", znfp1(&p->node));

        /* Check for msg addressed to zonegate */
        if (zonegate_first) {
            ZoneGate *pz;

            for (pz = zonegate_first; pz; pz = pz->next)
                if (node_eq(&p->node, &pz->node)) {
                    debug(7, "toss_echomail(): message is for zonegate, "
                          "stripping SEEN-BYs");
                    save = body->seenby;
                    tl_init(&body->seenby);
                    is_saved = TRUE;
                    lon_to_kludge_sorted(&body->seenby,
                                         "SEEN-BY:", &pz->seenby);
                    break;
                }
        }

        /* Rewrite message header */
        msg->node_to = p->node;
#ifdef BEST_AKA
        cf_set_best(msg->node_to.zone, msg->node_to.net, msg->node_to.node);
#endif                          /* BEST_AKA */
        msg->node_from = cf_n_addr();
        /* Open packet file, EchoMail, Normal */
        fp = outpkt_open(cf_addr(), &p->node, g_flag, 'e', 'n', FALSE);
        if (fp == NULL)
            return severe_error = ERROR;
        /* Write message header and body */
        if (pkt_put_msg_hdr(fp, msg, FALSE) != OK)
            return severe_error = ERROR;
        if (msg_put_msgbody(fp, body) != OK)
            return severe_error = ERROR;

        if (is_saved) {
            tl_clear(&body->seenby);
            body->seenby = save;
        }

        msgs_echomail++;
    }

    return OK;
}

/*
 * Convert SEEN-BY or ^APATH lines to LON
 */
void kludge_to_lon(Textlist * tl, LON * lon)
{
    Node old, node;
    Textline *l;
    char *p;

    for (l = tl->first; l; l = l->next) {
        p = l->line;
        /* Skip SEEN-BY:, ^APATH: or whatever, copy to buffer[] */
        while (*p && !is_space(*p) && *p != ':')
            p++;
        if (*p == ':')
            p++;
        while (*p && is_space(*p))
            p++;
        BUF_COPY(buffer, p);
        strip_crlf(buffer);
        /* Tokenize and convert to node addresses */
        node_invalid(&old);
        old.zone = cf_zone();
        for (p = strtok(buffer, " \t"); p; p = strtok(NULL, " \t"))
            if (asc_to_node_diff(p, &node, &old) == OK) {
                lon_add(lon, &node);
                old = node;
            }
    }

    return;
}

/*
 * node_eq_echo() --- compare node adresses, ignoring zone
 */
int node_eq_echo(Node * a, Node * b)
{
    if (a == NULL || b == NULL)
        return FALSE;

    return a->net == b->net && a->node == b->node && a->point == b->point;
}

/*
 * node_eq_echo_pa() --- compare node adresses, ignoring zone and point
 *                  if a->point == 0.
 */
int node_eq_echo_pa(Node * a, Node * b)
{
    if (a == NULL || b == NULL)
        return FALSE;

    return
        a->net == b->net && a->node == b->node &&
        (a->point == 0 || a->point == b->point);
}

/*
 * Search node in SEEN-BY list, ignoring zone
 */
int lon_search_echo(LON * lon, Node * node)
{
    LNode *p;

    for (p = lon->first; p; p = p->next)
        if (node_eq_echo(&p->node, node))
            return TRUE;

    return FALSE;
}

/*
 * Check for local address
 */
int is_local_addr(Node * node, char only3d)
{
    Node *n;
    char found = FALSE;

    for (n = cf_addr_trav(TRUE); n; n = cf_addr_trav(FALSE))
        if (only3d ? node_eq_echo(node, n) : node_eq(node, n)) {
            found = TRUE;
            break;
        }

    return found;
}

/*
 * Add nodes to SEEN-BY
 */
void do_seenby(LON * seenby, LON * nodes, LON * new, LON * passive, int cup,
               Node * from)
    /* seenby --- nodes in SEEN-BY lines */
    /* nodes  --- nodes in AREAS.BBS */
    /* new    --- new nodes added */
{
    LNode *p;
    int j = 1;

    for (p = nodes->first; p; p = p->next, j++)
        if (!lon_search_echo(seenby, &p->node)) {
            if (j < cup || node_eq(from, &p->node))
                continue;

            if ((NULL != passive) && !lon_search(passive, &(p->node))) {
                lon_add(seenby, &p->node);
                if (new)
                    lon_add(new, &p->node);
            }
        }

    return;
}

/*
 * Add current address to ^APATH
 */
void do_path(LON * path)
{
    if (path->last && node_eq_echo(&path->last->node, cf_addr()))
        /* Already there */
        return;

    lon_add(path, cf_addr());
    return;
}

/*
 * Check ^APATH for circular path
 */
int do_check_path(LON * path)
{
    LNode *p;

    for (p = path->first; p; p = p->next)
        if (p->next)            /* Don't check last node in list */
            if (is_local_addr(&p->node, TRUE))
                return ERROR;

    return OK;
}

/*
 * Save bad message
 */
int do_bad_msg(Message * msg, MsgBody * body)
{
    FILE *fp;

    /* Open packet file, EchoMail, Normal */
    fp = outpkt_open(&msg->node_from, &msg->node_to, 'b', 'a', 'd', TRUE);
    if (fp == NULL)
        return severe_error = ERROR;
    /* Write message header and body */
    if (pkt_put_msg_hdr(fp, msg, FALSE) != OK)
        return severe_error = ERROR;
    if (msg_put_msgbody(fp, body) != OK)
        return severe_error = ERROR;

    return OK;
}

/*
 * First node in LON
 */
Node *lon_first(LON * lon)
{
    if (lon->first)
        return &lon->first->node;

    return NULL;
}

/*
 * Special SEEN-BY / PATH processing if message originates from a 4d point
 */
void do_4dpoint(LON * seenby, LON * path, Node * from, Node * addr)
{
    /*
     * If there is only one node in SEEN-BY, which is the same address
     * (2d) as the sender, then replace it with the sender 4d address.
     */
    if (seenby->size == 1 && node_eq_echo_pa(lon_first(seenby), from))
        *lon_first(seenby) = *from;

    /*
     * Same for PATH
     */
    if (path->size == 1 && node_eq_echo_pa(lon_first(path), from))
        *lon_first(path) = *from;

    /*
     * Now make sure that also our current AKA is in SEEN-BY
     */
    if (!lon_search_echo(seenby, cf_addr()))
        lon_add(seenby, cf_addr());

    return;
}

/*
 * SEEN-BY remove processing
 */
void do_deleteseenby(LON * seenby)
{
    LNode *p;
    LON *ln;

    ln = &deleteseenby_first->deleteseenby;

    lon_debug(9, "do_deleteseenby(): List of SEEN-BYs for removing: ",
              ln, FALSE);

    for (p = ln->first; p; p = p->next) {
        if (lon_search(seenby, &p->node)) {
            debug(7, "do_deleteseenby(): Found SEEN-BY for removing: %s",
                  znf1(&p->node));
            lon_remove(seenby, &p->node);
        }
    }
}

/*
 * PATH remove processing
 */
void do_deletepath(LON * path)
{
    LNode *p;
    LON *ln;

    ln = &deleteseenby_first->deleteseenby;

    lon_debug(9, "do_deletepath(): List of PATH for removing: ", ln, FALSE);

    for (p = ln->first; p; p = p->next) {
        if (lon_search(path, &p->node)) {
            debug(7, "do_deletepath(): Found PATH for removing: %s",
                  znf1(&p->node));
            lon_remove(path, &p->node);
        }
    }
}

/*
 * Unknown area
 */
int do_unknown_area(char *areaname, Message * msg, MsgBody * body)
{

    fglog("unknown area %s from %s", areaname, znfp1(&msg->node_from));
    msgs_unknown++;
    if (!kill_unknown)
        return do_bad_msg(msg, body);
    return OK;
}

/*
 * Process EchoMail message
 */
int do_echomail(Packet * pkt, Message * msg, MsgBody * body)
{
    AreasBBS *area;
    AddToSeenBy *addto;
    LON seenby, path, new;
    int ret;
    Node *node;
#ifdef SECURITY
    Node node2;
#endif                          /* SECURITY */
    char areaname[BUFSIZ];
    char autocreate_cmd[BUFSIZ];
#ifndef ACTIVE_LOOKUP
    char autocreate_ng_cmd[MAXPATH];
    Area *autocreate_area;
#endif                          /* !ACTIVE_LOOKUP */
    Passwd *pwd;

    /*
     * Lookup area and set zone
     */
    BUF_COPY(areaname, body->area + 5);
    strip_crlf(areaname);
    debug(5, "EchoMail AREA: %s", areaname);

#ifdef SECURITY
    /*
     * Security checks
     */
    node_invalid(&node2);

    /* No Origin or MSGID address => bad */
    if (node_eq(&node2, &msg->node_orig)) {
        /* No origin line address => bad message */
        fglog("bad echomail (no origin addr) area %s from %s",
              areaname, znfp1(&msg->node_from));
        ++msgs_insecure;
        if (!kill_insecure)
            return do_bad_msg(msg, body);
        return OK;
    }

    if (check_point_origin_addr == TRUE &&
        0 != pkt->from.point && !node_eq(&msg->node_orig, &pkt->from)) {
        /* forged origin line address => bad mesage */
        fglog("bad echomail (forged origin addr) area %s from %s",
              areaname, znfp1(&msg->node_from));
        ++msgs_insecure;
        if (!kill_insecure)
            return do_bad_msg(msg, body);
        return OK;
    }
#endif                          /* !SECURITY */

    if (NULL != (pwd = passwd_lookup("packet", &msg->node_from)) &&
        stricmp(pkt->passwd, pwd->passwd)) {
        fglog("Insecure echomail packet from %s, area %s (%s pkt password)",
              znfp1(&msg->node_from), areaname,
              ('\0' == *(pkt->passwd)) ? "no" : "bad");
        ++msgs_insecure;
        if (!kill_insecure)
            return do_bad_msg(msg, body);
        return OK;
    }

    if (NULL == (area = areasbbs_lookup(areaname))) {

        AreaUplink *a;

        areafix_init(TRUE);

        areafix_auth_check(&msg->node_from, NULL, FALSE);

        if (!authorized_new) {
            fglog
                ("node %s not authorized to create area %s (config restriction)",
                 znfp1(&msg->node_from), areaname);
            /* Unknown area */
            do_unknown_area(areaname, msg, body);
            return OK;
        }

        if (areafix_check_forbidden_area(areaname)) {
            fglog("area %s is forbidden to create", areaname);
            /* Unknown area */
            do_unknown_area(areaname, msg, body);
            return OK;
        }

        if (!int_uplinks) {
            uplinks_init();
            int_uplinks = TRUE;
        }
        a = uplinks_line_get(TRUE, &msg->node_from);
        if (a != NULL && a->options != NULL)
            BUF_COPY5(autocreate_cmd, areaname, " ", autocreate_line, " ",
                      a->options);
        else
            BUF_COPY3(autocreate_cmd, areaname, " ", autocreate_line);

        if (OK != cmd_new_int(&msg->node_from, autocreate_cmd, NULL)) {
            fglog("can't create area %s from %s (cmd_new() returned ERROR)",
                  areaname, znfp1(&msg->node_from));
            do_unknown_area(areaname, msg, body);
            return OK;
        }

        if (NULL == (area = areasbbs_lookup(areaname))) {
            fglog("can't create area %s from %s (not found after creation)",
                  areaname, znfp1(&msg->node_from));
            do_unknown_area(areaname, msg, body);
            return OK;
        }

        fglog("created area %s from %s", areaname, znfp1(&msg->node_from));

        /*
         * Create newsgroup if needed.
         */

#ifndef ACTIVE_LOOKUP
        if (autocreate_ng) {
            areas_init();

            if (NULL !=
                (autocreate_area = areas_lookup(areaname, NULL, &pkt->to))) {
                BUF_COPY2(autocreate_ng_cmd, "%N/ngoper create ",
                          autocreate_area->group);
                ret = run_system(autocreate_ng_cmd);
                if (0 != ret)
                    fglog("can't create newsgroup (rc != 0)");
            } else {
                fglog("can't create newsgroup (not found in areas)");
            }
        }
#endif                          /* !ACTIVE_LOOKUP */
    }

    /* Set address for this area */
    if (area->zone != -1)
        cf_set_zone(area->zone);
    if (area->addr.zone != -1)
        cf_set_curr(&area->addr);

    (area->msgs_in)++;

    /*
     * Dupe check
     */
    if (dupe_check) {
        char *p, *msgid;
        Textline *tl;

        if ((p = kludge_get(&body->kludge, "MSGID", &tl))) {
            /* ^AMSGID */
            p = tl->line;
            if (*p == '\001')
                p++;
            BUF_COPY3(buffer, area->area, " ", p);
            strip_crlf(buffer);
            msgid = buffer;
            /* Replace char illegal for DBZ */
            for (p = msgid; *p; p++)
                if (*p == '\t' || *p == '\r' || *p == '\n')
                    *p = '_';
        } else {
            /* If KillNoMSGID ... */
            if (kill_nomsgid) {
                fglog("no ^AMSGID treated as dupe from %s(%s): %s",
                      znfp1(&msg->node_from), znfp2(&pkt->from), area->area);
                msgs_dupe++;
                (area->msgs_dupe)++;

                if (!kill_dupe)
                    return do_bad_msg(msg, body);
                return OK;
            }

            /* No ^AMSGID, use sender, date and checksum */
            if (msg_parse_origin(body->origin, &msg->node_orig) == ERROR) {
                fglog("invalid * Origin treated as dupe from %s(%s): %s",
                      znfp1(&msg->node_from), znfp2(&pkt->from), area->area);
                msgs_dupe++;
                (area->msgs_dupe)++;
                if (!kill_dupe)
                    return do_bad_msg(msg, body);
                return OK;
            }

            /* Compute CRC for strings from, to, subject */
            crc32_init();
            crc32_compute((unsigned char *)msg->name_from,
                          strlen(msg->name_from));
            crc32_compute((unsigned char *)msg->name_to, strlen(msg->name_to));
            crc32_compute((unsigned char *)msg->subject, strlen(msg->subject));

            str_printf(buffer, sizeof(buffer), "%s NOMSGID: %s %s %08lx",
                       area->area, znfp1(&msg->node_orig),
                       date("%y%m%d %H%M%S", &msg->date), crc32_value());
            msgid = buffer;
        }

        /* Check for old message (date < NOW - MaxHistory) */
        if (kill_old) {
            if (msg->date < exp_sec) {
                fglog
                    ("message too old, treated as dupe: %s origin %s(%s) date %s",
                     area->area, znfp1(&msg->node_orig), znfp2(&pkt->from),
                     date(DATE_FTS_0001, &msg->date));
                msgs_dupe++;
                (area->msgs_dupe)++;
                if (!kill_dupe)
                    return do_bad_msg(msg, body);
                return OK;
            }
        }

        /* Check for existence in MSGID history */
        if (hi_test(msgid)) {
            /* Dupe! */
            fglog("dupe from %s(%s): %s", znfp1(&msg->node_from),
                  znfp2(&pkt->from), msgid);
            msgs_dupe++;
            (area->msgs_dupe)++;
            if (!kill_dupe)
                return do_bad_msg(msg, body);
            return OK;
        } else {
            /* Put into MSGID history */
#ifdef INSECURE_DONT_PUT_INTO_DUPE_DB
            if (node_eq(cf_addr(), &msg->node_from) ||
                lon_search(&area->nodes, &msg->node_from))
#endif                          /* INSECURE_DONT_PUT_INTO_DUPE_DB */
                if (hi_write(msg->date, msgid) == ERROR)
                    return ERROR;
        }
    }

    /*
     * Lookup area in AddToSeenBy list
     */
    for (addto = addto_first; addto; addto = addto->next)
        if (wildmatch(area->area, addto->area, TRUE))
            break;

    /*
     * Check that this message is addressed to one of our AKAs
     */
    if (!n_flag) {
        if (!node_eq(&msg->node_to, cf_addr()) &&
            !is_local_addr(&msg->node_to, FALSE)) {
            /* Routed EchoMail */
            fglog("routed echomail area %s from %s to %s", area->area,
                  znfp1(&msg->node_from), znfp2(&msg->node_to));
            msgs_routed++;
            (area->msgs_routed)++;
            if (!kill_routed)
                return do_bad_msg(msg, body);
            return OK;
        }
    }

    /*
     * Check that origin is listed in AREAS.BBS
     */
    if (!t_flag) {
        if (!node_eq(cf_addr(), &msg->node_from) &&
            !lon_search(&area->nodes, &msg->node_from)) {
            /* Insecure EchoMail */
            fglog("insecure echomail area %s from %s", area->area,
                  znfp1(&msg->node_from));
            msgs_insecure++;
            (area->msgs_insecure)++;
            if (!kill_insecure)
                return do_bad_msg(msg, body);
            return OK;
        }
    }

#ifdef FTN_ACL
    /*
     * Check if link is read only
     */
    if (uplink_can_be_readonly || !lon_is_uplink(&(area->nodes), area->uplinks,
                                                 &(msg->node_from)))
        if (ftnacl_isreadonly(&msg->node_from, area->area, TYPE_ECHO)) {
            fglog("echomail to read only area %s from %s", area->area,
                  znfp1(&msg->node_from));

            tl_appendf(&notify, "%s,Sysop,Toss Daemon,insecure echomail,\r\n\
\r\n\tYour message to area %s was delete. This\r\n\
area have status read-only\r\n\r\n\
    Your Message:\r\n\r\n\
		From: %s\r\n\
		To: %s\r\n\
		Subject: %s\r\n\r\n", znfp1(&msg->node_from), area->area, nf1(&msg->node_from), znfp1(&pkt->to), msg->subject);

            msgs_readonly++;
            (area->msgs_readonly)++;
#ifdef FTN_ACL
            if (!kill_readonly)
#endif                          /* FTN_ACL */
                return do_bad_msg(msg, body);
#ifdef FTN_ACL
            return OK;
#endif                          /* FTN_ACL */
        }
#endif                          /* FTN_ACL */

    /*
     * SEEN-BY / ^APATH processing
     */
    lon_init(&seenby);
    lon_init(&path);
    lon_init(&new);
    kludge_to_lon(&body->seenby, &seenby);
    kludge_to_lon(&body->path, &path);

    /* Before */
    lon_debug(9, "SEEN-BY: ", &seenby, FALSE);
    lon_debug(9, "Path   : ", &path, FALSE);

    /* Special processing if message is from a 4d point */
    do_4dpoint(&seenby, &path, &msg->node_from, cf_addr());

    /* Make sure that sender/recipient are in SEEN-BY */
    if (!lon_search_echo(&seenby, &msg->node_from))
        lon_add(&seenby, &msg->node_from);
    if (!lon_search_echo(&seenby, &msg->node_to))
        lon_add(&seenby, &msg->node_to);

#ifdef SECURITY
    /* Make sure that sender is in PATH */
    if (NULL == path.last) {
        fglog("WARNING: packet hasn't ^APATH, was added %s", nf1(&pkt->from));
        lon_add(&path, &pkt->from);
    } else if (!node_eq_echo(&path.last->node, &pkt->from)) {
        fglog
            ("WARNING: last addr ^APATH (%s) isn't eq to addr in pkt header(%s)",
             znfp1(&path.last->node), znfp2(&pkt->from));
        lon_add(&path, &pkt->from);
    }
#endif                          /* SECURITY */

    /* Add nodes not already in SEEN-BY to seenby and new */
    do_seenby(&seenby, &area->nodes, &new, &(area->passive), area->uplinks,
              &msg->node_from);

    /* Add extra nodes to SEEN-BY */
    if (addto)
        do_seenby(&seenby, &addto->add, NULL, NULL, 0, &msg->node_from);

    /* Add all AKAs for the current zone, if AddOtherAKA is set */
    if (add_other_aka) {
        short z;

        z = cf_zone();
        for (node = cf_addr_trav(FIRST); node; node = cf_addr_trav(NEXT))
            if (node->zone == z && !lon_search_echo(&seenby, node))
                lon_add(&seenby, node);
    }

    /* Delete nodes from SEEN-BY */
    if (deleteseenby_first)
        do_deleteseenby(&seenby);

    /* If not passthru area and not from our own address (point gateway
     * setup with Address==Uplink), add our own address to new          */
    if (!p_flag &&
        !(area->flags & AREASBBS_PASSTHRU) &&
        !node_eq(&msg->node_from, cf_addr()))
        lon_add(&new, cf_addr());
    else if (!p_flag &&
             !node_eq(&msg->node_from, cf_addr()) &&
             lon_search(&area->nodes, cf_addr()))
        lon_add(&new, cf_addr());

    /* Add our address to end of ^APATH, if not already there */
    do_path(&path);

    /* Delete nodes from PATH */
    if (deletepath_first)
        do_deletepath(&path);

    /* After */
    lon_debug(9, "SEEN-BY: ", &seenby, FALSE);
    lon_debug(9, "Path   : ", &path, FALSE);
    lon_debug(9, "New    : ", &new, FALSE);

    /*
     * Check for circular ^APATH
     */
    if (check_path && do_check_path(&path) == ERROR) {
        /* Circular ^APATH EchoMail */
        fglog("circular path echomail area %s from %s to %s",
              area->area, znfp1(&msg->node_from), znfp2(&msg->node_to));
        msgs_path++;
        (area->msgs_path)++;

        lon_delete(&seenby);
        lon_delete(&path);
        lon_delete(&new);

        if (!kill_circular)
            return do_bad_msg(msg, body);
        return OK;
    }

    if (!lon_is_uplink(&(area->nodes), area->uplinks, &(msg->node_from)) &&
        areasbbs_isstate(area->state, 'W')) {
        fglog("Area %s have status W, rename to bad", area->area);
        lon_delete(&seenby);
        lon_delete(&path);
        lon_delete(&new);

        tl_appendf(&notify, "%s,Sysop,Toss Daemon,insecure echomail,\r\n\
\r\n\tYour message to area %s was delete. This\r\n\
area have status 'W' (no traffic from uplink)\r\n\r\n\
    Your Message:\r\n\r\n\
	From: %s\r\n\
	To: %s\r\n\
	Subject: %s\r\n\r\n", znfp1(&msg->node_from), area->area, nf1(&msg->node_from), znfp1(&pkt->to), msg->subject);

        if (!kill_circular)
            return do_bad_msg(msg, body);
        return OK;

    }

    if (NULL != area->state) {
        if (areasbbs_isstate(area->state, 'U') ||
            areasbbs_isstate(area->state, 'W') ||
            areasbbs_isstate(area->state, 'F')) {
            fglog("setting state 'S' for area %s", area->area);
            areasbbs_chstate(&(area->state), "UWF", 'S');
        } else if (areasbbs_isstate(area->state, 'P'))
            return do_bad_msg(msg, body);
    }
    area->time = time(NULL);
    areasbbs_changed();

    /*
     * Create new SEEN-BY and ^APATH lines
     */
    tl_clear(&body->seenby);
    tl_clear(&body->path);
    lon_to_kludge_sorted(&body->seenby, "SEEN-BY:", &seenby);
    ret = lon_to_kludge(&body->path, "\001PATH:", &path);

    if (no_empty_path && ret == 0)
        tl_clear(&body->path);

    /*
     * Send message to all downlinks in new
     */
    ret = toss_echomail(msg, body, &seenby, &path, &new, &(area->passive));

    (area->msgs_out)++;
    (area->msgs_size) +=
        sizeof(body) + sizeof(msg) + sizeof(seenby) + sizeof(path);

    lon_delete(&seenby);
    lon_delete(&path);
    lon_delete(&new);

    return ret;
}

/*
 * Add our ^AVia line
 */
#ifndef DO_NOT_TOSS_NETMAIL
void add_via(Textlist * list, Node * gate)
{
#ifndef FTS_VIA
    BUF_COPY5(buffer, "Via FIDOGATE/", PROGRAM, znf1(gate),
              date(DATE_VIA, NULL), "\r");
    tl_append(list, buffer);
#else
    BUF_COPY5(buffer, "Via ", znf1(gate), " @", date(DATE_VIA, NULL),
              " FIDOGATE/");
    BUF_APPEND2(buffer, PROGRAM, "\r");
    tl_append(list, buffer);
#endif                          /* !FTS_VIA */
}

#endif                          /* !DO_NOT_TOSS_NETMAIL */

/*
 * Change address according to rewrite pattern
 */
void change_addr(Node * node, Node * newpat)
{
    if (newpat->zone != -1)
        node->zone = newpat->zone;
    if (newpat->net != -1)
        node->net = newpat->net;
    if (newpat->node != -1)
        node->node = newpat->node;
    if (newpat->point != -1)
        node->point = newpat->point;
}

/*
 * Perform REWRITE commands
 */
void do_rewrite(Message * msg)
{
    Rewrite *r;

    for (r = rewrite_first; r; r = r->next) {
        /* From */
        if (node_match(&msg->node_from, &r->from))
            if (r->type == CMD_REWRITE || (r->type == CMD_REWRITE_FROM &&
                                           wildmatch(msg->name_from, r->name,
                                                     TRUE))) {
                fglog("rewrite(from): %s @ %s -> %s", msg->name_from,
                      znfp1(&msg->node_from), znfp2(&r->to));

                change_addr(&msg->node_from, &r->to);
                break;
            }
        /* To */
        if (node_match(&msg->node_to, &r->from))
            if (r->type == CMD_REWRITE || (r->type == CMD_REWRITE_TO &&
                                           wildmatch(msg->name_to, r->name,
                                                     TRUE))) {
                fglog("rewrite(to): %s @ %s -> %s", msg->name_from,
                      znfp1(&msg->node_from), znfp2(&r->to));

                change_addr(&msg->node_from, &r->to);
                break;
            }
    }
}

/*
 * Perform REMAP commands
 *
 * Return == TRUE: remapped to 0:0/0.0, kill message
 */
int do_remap(Message * msg)
{
    Remap *r;
    Node node;
    char kill = FALSE;

    for (r = remap_first; r; r = r->next)
        if ((r->type == CMD_REMAP_TO &&
             node_match(&msg->node_to, &r->from) &&
             wildmatch(msg->name_to, r->name, TRUE))
            ||
            (r->type == CMD_REMAP_FROM &&
             node_match(&msg->node_from, &r->from) &&
             wildmatch(msg->name_from, r->name, TRUE))
            ) {
            node = msg->node_to;
            change_addr(&msg->node_to, &r->to);

            if (msg->node_to.zone == 0 && msg->node_to.net == 0 &&
                msg->node_to.node == 0 && msg->node_to.point == 0)
                kill = TRUE;

            if (!node_eq(&node, &msg->node_to)) {
                if (r->type == CMD_REMAP_TO)
                    fglog("remapto: %s @ %s -> %s", msg->name_to,
                          znfp1(&node),
                          !kill ? znfp2(&msg->node_to) : "KILLED");
                if (r->type == CMD_REMAP_FROM)
                    fglog("remapfrom: %s @ %s -> %s", msg->name_from,
                          znfp1(&msg->node_from),
                          !kill ? znfp2(&msg->node_to) : "KILLED");
            }

            break;
        }

    return kill;
}

/*
 * Check for empty NetMail
 *
 * An empty NetMail is a message comprising only ^A kludges and empty lines.
 */
int check_empty(MsgBody * body)
{
    Textline *pl;

    if (body->rfc.n)
        return FALSE;
    if (body->tear || body->origin)
        return FALSE;
    if (body->seenby.n)
        return FALSE;

    for (pl = body->body.first; pl; pl = pl->next)
        if (pl->line[0] && pl->line[0] != '\r')
            return FALSE;

    return TRUE;
}

/*
 * Process NetMail message
 */
#ifndef SPYES
int do_netmail(Packet * pkt, Message * msg, MsgBody * body)
#else
int do_netmail(Packet * pkt, Message * msg, MsgBody * body, int forwarded)
#endif                          /* !SPYES */
{
    FILE *fp;
    char flav;

    if (log_netmail)
#ifndef SPYES
        fglog("MAIL: %s @ %s -> %s @ %s",
              msg->name_from, znfp1(&msg->node_from),
              msg->name_to, znfp2(&msg->node_to));
#else
    {
        if (forwarded)
            fglog("FORWARDED MAIL: %s @ %s -> %s @ %s",
                  msg->name_from, znfp1(&msg->node_from),
                  msg->name_to, znfp2(&msg->node_to));
        else
            fglog("MAIL: %s @ %s -> %s @ %s",
                  msg->name_from, znfp1(&msg->node_from),
                  msg->name_to, znfp2(&msg->node_to));
    }
#endif                          /* !SPYES */

    /*
     * Check for file attach
     */
    if (msg->attr & MSG_FILE) {
        fglog("file attach %s", msg->subject);
    }

    /*
     * Check for empty NetMail message addressed to one of our AKAs
     */
    if (kill_empty && check_empty(body)) {
        if (is_local_addr(&msg->node_to, FALSE)) {
            fglog("killing empty msg from %s @ %s",
                  msg->name_from, znfp1(&msg->node_from));
            msgs_empty++;

            return OK;
        }
    }

    /*
     * Rewrite from/to addresses according to ROUTING rules
     */
#ifdef DO_NOT_TOSS_NETMAIL
    if (!no_rewrite)
        do_rewrite(msg);
#else
    do_rewrite(msg);
#endif                          /* DO_NOT_TOSS_NETMAIL */

    /*
     * Remap to address according to ROUTING rules
     */
#ifdef DO_NOT_TOSS_NETMAIL
    if (!no_rewrite && do_remap(msg))
#else
    if (do_remap(msg))
#endif                          /* DO_NOT_TOSS_NETMAIL */
    {
        /* Kill this message, remapped to 0:0/0.0 */
        return OK;
    }

    /*
     * Write to output packet
     */
    cf_set_best(msg->node_to.zone, msg->node_to.net, msg->node_to.node);

    /* Get outbound flavor from msg attributes */
    flav = 'n';
    if (!s_flag) {
        if (msg->attr & MSG_HOLD)
            flav = 'h';
        if (msg->attr & MSG_CRASH)
            flav = 'c';
    }

#ifdef DO_NOT_TOSS_NETMAIL
    pkt_outdir(cf_p_netmaildir(), NULL);

    /* dirty hack for more compatible with other soft */
    fp = outpkt_open(cf_addr(), &msg->node_to, 'a', 'a', 'a', FALSE);
#else
    /* Open output packet */
    fp = outpkt_open(cf_addr(), &msg->node_to, g_flag, 'n', flav, FALSE);
#endif                          /* DO_NOT_TOSS_NETMAIL */

    if (fp == NULL)
        return severe_error = ERROR;

    /* Add ftntoss ^AVia line */
#ifndef DO_NOT_TOSS_NETMAIL
    add_via(&body->via, cf_addr());
#endif                          /* !DO_NOT_TOSS_NETMAIL */

    /* Write message header and body */
    if (pkt_put_msg_hdr(fp, msg, TRUE) != OK)
        return severe_error = ERROR;
    if (msg_put_msgbody(fp, body) != OK)
        return severe_error = ERROR;

#ifdef DO_NOT_TOSS_NETMAIL
    pkt_outdir(O_flag ? O_flag : cf_p_toss_route(), NULL);
#endif                          /* DO_NOT_TOSS_NETMAIL */

    msgs_netmail++;

    return OK;
}

/*
 * Read and process FTN packets
 */
int unpack(FILE * pkt_file, Packet * pkt)
{
    Message msg = { 0 };        /* Message header */
    Textlist tl;                /* Textlist for message body */
    MsgBody body;               /* Message body of FTN message */
#ifdef SPYES
    Spy *spy;                   /* Spy info if avaliable */
    char old_subject[MSG_MAXSUBJ];
#endif                          /* SPYES */
    int type;

    /*
     * Initialize
     */
    tl_init(&tl);
    msg_body_init(&body);

    /*
     * Read packet
     */
    type = pkt_get_int16(pkt_file);
    if (type == ERROR) {
        if (feof(pkt_file)) {
            fglog("$WARNING: premature EOF reading input packet");
            TMPS_RETURN(OK);
        }

        fglog("ERROR: reading input packet");
        TMPS_RETURN(ERROR);
    }

    while ((type == MSG_TYPE) && !xfeof(pkt_file)) {
        /*
         * Read message header
         */
        msg.node_from = pkt->from;
        msg.node_to = pkt->to;

        if (pkt_get_msg_hdr(pkt_file, &msg, strict) == ERROR) {
            fglog("ERROR: reading input packet");
            TMPS_RETURN(ERROR);
        }

        /*
         * Read message body
         */
#ifdef OLD_TOSS

        type = pkt_get_body(pkt_file, &tl);

        if (type == ERROR) {
            if (feof(pkt_file)) {
                fglog("$WARNING: premature EOF reading input packet");
            } else {
                fglog("ERROR: reading input packet");
                TMPS_RETURN(ERROR);
            }
        }
        msgs_in++;
        msg_count++;

        /*
         * Parse message body
         */
        if (msg_body_parse(&tl, &body) == -2)
            fglog("ERROR: parsing message body");
#else
        if (pkt_get_body_parse(pkt_file, &body, &msg.node_from, &msg.node_to) !=
            OK) {
            fglog("ERROR: parsing message body");
            return ERROR;
        }
        msgs_in++;
        msg_count++;
#endif
        /* Retrieve address information from kludges for NetMail */
        if (body.area == NULL) {
            /* Don't use point address from packet for Netmail */
            msg.node_from.point = 0;
            msg.node_to.point = 0;
            /* Retrieve complete address from kludges */
            kludge_pt_intl(&body, &msg, TRUE);
            msg.node_orig = msg.node_from;

#ifndef SPYES
            debug(5, "NetMail: %s -> %s",
                  znfp1(&msg.node_from), znfp2(&msg.node_to));
            if (do_netmail(pkt, &msg, &body) == ERROR)
                TMPS_RETURN(ERROR);
#else
            if (((spy = spyes_lookup(&msg.node_from)) != NULL) ||
                ((spy = spyes_lookup(&msg.node_to)) != NULL)) {
                debug(5, "NetMail spyes: %s -> %s, forwarded to %s",
                      znfp1(&msg.node_from),
                      znfp2(&msg.node_to), znfp3(&spy->forward_node));
                if (do_netmail(pkt, &msg, &body, FALSE) == ERROR)
                    return ERROR;
                BUF_COPY(old_subject, msg.subject);
                str_printf(msg.subject, MSG_MAXSUBJ, "[FWD] from %s to %s: %s",
                           znfp1(&msg.node_from),
                           znfp2(&msg.node_to), old_subject);
                msg.node_to = spy->forward_node;
                if (do_netmail(pkt, &msg, &body, TRUE) == ERROR)
                    return ERROR;
            } else {
                debug(5, "NetMail: %s -> %s",
                      znfp1(&msg.node_from), znfp2(&msg.node_to));
                if (do_netmail(pkt, &msg, &body, FALSE) == ERROR)
                    return ERROR;
            }
#endif                          /* !SPYES */
        } else {
            /* Specially for echomail */
            msg.node_from = pkt->from;
            msg.node_to = pkt->to;

            /* Try to get address from Origin or MSGID */
            if (OK != msg_parse_origin(body.origin, &msg.node_orig) &&
                OK != msg_parse_msgid(kludge_get(&body.kludge, "MSGID", NULL),
                                      &msg.node_orig)) {
                node_invalid(&msg.node_orig);
            }

            debug(5, "EchoMail: %s -> %s (orig: %s)",
                  znfp1(&msg.node_from),
                  znfp2(&msg.node_to), znfp3(&msg.node_orig));
            if (do_echomail(pkt, &msg, &body) == ERROR)
                TMPS_RETURN(ERROR);
        }

        /*
         * Exit if signal received
         */
        if (signal_exit) {
            outpkt_close();
            msg_count = 0;
            TMPS_RETURN(severe_error = ERROR);
        }

        /*
         * Check for number of messages exceeding maxmsg
         */
        if (maxmsg && msg_count >= maxmsg) {
            if (x_flag)
                must_exit = TRUE;
            else
                outpkt_close();
            msg_count = 0;
        }

        tmps_freeall();
    } /**while(type == MSG_TYPE)**/

    TMPS_RETURN(OK);
}

/*
 * Unpack one packet file
 */
int unpack_file(char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;
    TIMEINFO ti;
    long pkt_size;

    /* Update time info for old messages */
    GetTimeInfo(&ti);
    now_sec = ti.time;
    max_sec = 24L * 3600L * max_history;
    exp_sec = now_sec - max_sec;
    if (exp_sec < 0)
        exp_sec = 0;
    debug(4, "now=%ld max=%ld, old < %ld",
          (long)now_sec, (long)max_sec, (long)exp_sec);

    /* Open packet and read header */
    pkt_file = fopen(pkt_name, R_MODE);
    if (!pkt_file) {
        fglog("$ERROR: can't open packet %s", pkt_name);
        rename_bad(pkt_name);
        TMPS_RETURN(OK);
    }
    if (pkt_get_hdr(pkt_file, &pkt) == ERROR) {
        fglog("ERROR: reading header from %s", pkt_name);
        fclose(pkt_file);
        rename_bad(pkt_name);
        TMPS_RETURN(OK);
    }

    /* Unpack it */
    pkt_size = check_size(pkt_name);
    fglog("packet %s (%ldb) from %s to %s", pkt_name, pkt_size,
          znfp1(&pkt.from), znfp2(&pkt.to));
    pkts_in++;
    pkts_bytes += pkt_size;

    if (unpack(pkt_file, &pkt) == ERROR) {
        fglog("ERROR: processing %s", pkt_name);
        fclose(pkt_file);
        rename_bad(pkt_name);
        TMPS_RETURN(severe_error);
    }

    fclose(pkt_file);

    if (unlink(pkt_name)) {
        fglog("$ERROR: can't unlink %s", pkt_name);
        rename_bad(pkt_name);
        TMPS_RETURN(ERROR);
    }

    TMPS_RETURN(OK);
}

/*
 * Function called on SIGINT
 */
void prog_signal(int signum)
{
    char *name = "";

    signal_exit = TRUE;

    switch (signum) {
    case SIGHUP:
        name = " by SIGHUP";
        break;
    case SIGINT:
        name = " by SIGINT";
        break;
    case SIGQUIT:
        name = " by SIGQUIT";
        break;
    default:
        name = "";
        break;
    }

    fglog("KILLED%s: exit forced", name);
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] [packet ...]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] [packet ...]\n\n", PROGRAM);
    fprintf(stderr, "\
options: -d --no-dupecheck            disable dupe check\n\
         -g --grade G                 processing grade\n\
         -I --in-dir DIR              set input packet directory\n\
         -O --out-dir DIR             set output packet directory\n\
         -l --lock-file               create lock file while processing\n\
         -t --insecure                insecure tossing (no AREAS.BBS check)\n\
         -n --toss-all                toss all EchoMail messages\n\
         -p --passthru                make all areas passthru\n\
         -r --routing-file NAME       read routing file\n\
         -s --strip-attribute         strip crash, hold message attribute\n\
         -m --maxmsg N                close output after N msgs\n\
         -x --maxmsg-exit             close output and exit after -m msgs\n\
         -M --maxopen N               set max # of open packet files\n\
         -b --areas-bbs NAME          use alternate AREAS.BBS\n\
\n\
	 -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config NAME             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n\
	 -w --wait [TIME]             wait for areas.bbs and history\n\
	                              lock to be released\n");
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    char *p;
    char *I_flag = NULL, *r_flag = NULL, *M_flag = NULL;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    char d_flag = FALSE;
    char *pkt_name;
    int w_flag = FALSE;
    time_t toss_start, toss_delta;
    AreasBBS *area;
    char bbslock[MAXPATH];

    int option_index;
    static struct option long_options[] = {
        {"no-dupecheck", 1, 0, 'd'},    /* Disable dupe check */
        {"grade", 1, 0, 'g'},   /* grade */
        {"in-dir", 1, 0, 'I'},  /* Set inbound packets directory */
        {"lock-file", 0, 0, 'l'},   /* Create lock file while processing */
        {"out-dir", 1, 0, 'O'}, /* Set packet directory */
        {"insecure", 0, 0, 't'},    /* Insecure */
        {"toss-all", 0, 0, 'n'},    /* Toss all EchoMail */
        {"routing-file", 1, 0, 'r'},    /* Set routing file */
        {"strip-attribute", 0, 0, 's'}, /* Strip attribute */
        {"maxmsg", 1, 0, 'm'},  /* Close after N messages */
        {"maxmsg-exit", 0, 0, 'x'}, /* Exit after maxmsg messages */
        {"maxopen", 1, 0, 'M'}, /* Set max # open packet files */
        {"areas-bbs", 1, 0, 'b'},
        {"passthru", 0, 0, 'p'},

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"uplink-addr", 1, 0, 'u'}, /* Set FIDO uplink address */
        {"wait", 1, 0, 'w'},
        {0, 0, 0, 0}
    };

    /* Log name */

    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();

    /* Parse options */
    while ((c = getopt_long(argc, argv, "dg:O:I:ltnr:sm:xM:b:pvhc:w:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftntoss options *****/
        case 'd':
            d_flag = TRUE;
            break;
        case 'g':
            g_flag = *optarg;
            break;
        case 'I':
            I_flag = optarg;
            break;
        case 'l':
            l_flag = TRUE;
            break;
        case 'O':
            O_flag = optarg;
            break;
        case 't':
            t_flag = TRUE;
            break;
        case 'n':
            n_flag = TRUE;
            break;
        case 'r':
            r_flag = optarg;
            break;
        case 's':
            s_flag = TRUE;
            break;
        case 'm':
            maxmsg = atoi(optarg);
            break;
        case 'x':
            x_flag = TRUE;
            break;
        case 'M':
            M_flag = optarg;
            break;
        case 'b':
            areas_bbs = optarg;
            break;
        case 'p':
            p_flag = TRUE;
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

    /*
     * Read config file
     */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /*
     * Process config options
     */
    if (a_flag)
        cf_set_addr(a_flag);
    if (u_flag)
        cf_set_uplink(u_flag);

    cf_debug();

    /*
     * Process optional config statements
     */
    if (cf_get_string("KillEmpty", TRUE) || cf_get_string("KillBlank", TRUE)) {
        debug(8, "actual: killempty true");
        kill_empty = TRUE;
    }
    if (cf_get_string("KillUnknown", TRUE)) {
        kill_unknown = TRUE;
    }
    if (cf_get_string("KillRouted", TRUE)) {
        kill_routed = TRUE;
    }
    if (cf_get_string("KillInsecure", TRUE)) {
        kill_insecure = TRUE;
    }
#ifdef FTN_ACL
    if (cf_get_string("KillReadonly", TRUE)) {
        kill_readonly = TRUE;
    }
#endif                          /* FTN_ACL */
    if (cf_get_string("KillCircular", TRUE)) {
        kill_circular = TRUE;
    }
    if (cf_get_string("LogNetMail", TRUE) || cf_get_string("Track", TRUE)) {
        debug(8, "actual lognetmail true");
        log_netmail = TRUE;
    }
    if (cf_get_string("CheckPath", TRUE)) {
        check_path = TRUE;
    }
    if (cf_get_string("DupeCheck", TRUE)) {
        if (d_flag) {
            debug(8, "DupeCheck disabled from command line!");
            dupe_check = FALSE;
        } else {
            dupe_check = TRUE;
        }
    }
    if (cf_get_string("KillNoMSGID", TRUE)) {
        kill_nomsgid = TRUE;
    }
    if (cf_get_string("KillDupe", TRUE) || cf_get_string("KillDupes", TRUE)) {
        debug(8, "actual KillDupe true");
        kill_dupe = TRUE;
    }
    if (!maxmsg && (p = cf_get_string("MaxMsg", TRUE))) {
        maxmsg = atoi(p);
        debug(8, "actual: MaxMsg %d", maxmsg);

    }
    if (!M_flag && (p = cf_get_string("MaxOpenFiles", TRUE))) {
        M_flag = p;
        debug(8, "actual MaxOpenFiles %s", M_flag);
    }
    if (M_flag)
        outpkt_set_maxopen(atoi(M_flag));
    if (cf_get_string("KillOld", TRUE)) {
        kill_old = TRUE;
    }
    if ((p = cf_get_string("MaxHistory", TRUE))) {
        max_history = atof(p);
        if (max_history < 0)
            max_history = 0;
        debug(8, "actual MaxHistory %g", max_history);
    }
    if (cf_get_string("TossEchoMail4D", TRUE)) {
        echomail4d = TRUE;
    }
    if (cf_get_string("NoEmptyPath", TRUE)) {
        no_empty_path = TRUE;
    }
    if (cf_get_string("AddOtherAKA", TRUE) ||
        cf_get_string("AddOtherAKAs", TRUE)) {
        debug(8, "actual AddOtherAKA true");
        add_other_aka = TRUE;
    }
#ifdef FTN_ACL
    if (cf_get_string("UplinkCanBeReadonly", TRUE)) {
        uplink_can_be_readonly = TRUE;
    }
#endif                          /* FTN_ACL */
    if (cf_get_string("TrafficStatistics", TRUE)) {
        traffic_statistics = TRUE;
    }
    if ((p = cf_get_string("TrafficStatisticsFile", TRUE))) {
        traffic_statistics_file = p;
    }
    if ((p = cf_get_string("AutoCreateLine", TRUE))) {
        autocreate_line = p;
    }
#ifndef ACTIVE_LOOKUP
    if (cf_get_string("AutoCreateNG", TRUE)) {
        autocreate_ng = TRUE;
    }
#endif                          /* !ACTIVE_LOOKUP */
    if (cf_get_string("CheckPointOriginAddr", TRUE)) {
        check_point_origin_addr = FALSE;
    }
#ifdef DO_NOT_TOSS_NETMAIL
    if (cf_get_string("NoRewrite", TRUE)) {
        no_rewrite = TRUE;
    }
#endif                          /* DO_NOT_TOSS_NETMAIL */
    strict = (cf_get_string("FTNStrictPktCheck", TRUE) != NULL);

    zonegate_init();
    addtoseenby_init();
    deleteseenby_init();
    deletepath_init();
#ifdef SPYES
    spyes_init();
#endif                          /* SPYES */
#ifdef FTN_ACL
    ftnacl_init();
#endif                          /* FTN_ACL */

    tl_init(&notify);

    /*
     * Process local options
     */
    BUF_EXPAND(in_dir, I_flag ? I_flag : cf_p_pinbound());
    pkt_outdir(O_flag ? O_flag : cf_p_toss_toss(), NULL);
    pkt_baddir(O_flag ? O_flag : cf_p_toss_bad(), NULL);

    /*
     * Get name of areas.bbs file from config file
     */
    if (!areas_bbs)
        areas_bbs = cf_get_string("AreasBBS", TRUE);
    if (!areas_bbs) {
        fprintf(stderr, "%s: no areas.bbs specified\n", PROGRAM);
        exit_free();
        return EX_USAGE;
    }

    routing_init(r_flag ? r_flag : cf_p_routing());
    BUF_COPY2(bbslock, areas_bbs, ".lock");
    if (lock_path(bbslock, w_flag ? w_flag : NOWAIT) == ERROR) {
        exit_free();
        return EXIT_BUSY;
    }
    areasbbs_init(areas_bbs);
    passwd_init();

    /* Install signal/exit handlers */
    signal(SIGHUP, prog_signal);
    signal(SIGINT, prog_signal);
    signal(SIGQUIT, prog_signal);

    /* Start time */
    toss_start = time(NULL);

    c = EXIT_OK;

    if (optind >= argc) {
        /* process packet files in directory */
        dir_sortmode(DIR_SORTMTIME);
        if (dir_open(in_dir, "*.pkt", TRUE) == ERROR) {
            fglog("$ERROR: can't open directory %s", in_dir);
            unlock_path(bbslock);
            exit_free();
            return EX_OSERR;
        }

        /* Lock file */
        if (l_flag)
            if (lock_program(PROGRAM, NOWAIT) == ERROR) {
                /* Already busy */
                exit_free();
                return EXIT_BUSY;
            }

        /* Open history */
        if (dupe_check) {
            if (lock_program(cf_p_lock_history(), w_flag ? w_flag : NOWAIT) ==
                ERROR) {

                /* Already busy, exit */
                if (l_flag) {
                    unlock_program(PROGRAM);
                    unlock_path(bbslock);
                }
                exit_free();
                return EXIT_BUSY;
            }
            hi_init_history();
        }

        for (pkt_name = dir_get(TRUE); pkt_name; pkt_name = dir_get(FALSE)) {
            if (unpack_file(pkt_name) == ERROR) {
                c = EXIT_ERROR;
                break;
            }
            if (must_exit) {
                c = EXIT_CONTINUE;
                break;
            }
        }

        /* Close history */
        if (dupe_check) {
            unlock_program(cf_p_lock_history());
            hi_close();
        }

        dir_close();

        /* Lock file */
        if (l_flag)
            unlock_program(PROGRAM);
    } else {
        /* Lock file */
        if (l_flag)
            if (lock_program(PROGRAM, NOWAIT) == ERROR) {
                /* Already busy */
                exit_free();
                return EXIT_BUSY;
            }

        /* Open history */
        if (dupe_check) {
            if (lock_program(cf_p_lock_history(), w_flag ? w_flag : NOWAIT) ==
                ERROR) {
                /* Already busy, exit */
                if (l_flag) {
                    unlock_program(PROGRAM);
                    unlock_path(bbslock);
                }

                exit_free();
                return EXIT_BUSY;
            }
            hi_init_history();
        }

        /* Process packet files on command line */
        for (; optind < argc; optind++) {
            if (unpack_file(argv[optind]) == ERROR) {
                c = EXIT_ERROR;
                break;
            }
            if (must_exit) {
                c = EXIT_CONTINUE;
                break;
            }
        }

        /* Close history */
        if (dupe_check) {
            unlock_program(cf_p_lock_history());
            hi_close();
        }

        /* Lock file */
        if (l_flag)
            unlock_program(PROGRAM);
    }

    outpkt_close();

    /* Stop time */
    toss_delta = time(NULL) - toss_start;
    if (toss_delta <= 0)
        toss_delta = 1;

    if (traffic_statistics) {
        area = areasbbs_first();
        for (area = areasbbs_first(); NULL != area; area = area->next) {
            if (0 != area->msgs_in) {
                if (traffic_statistics_file)
                    log_file(traffic_statistics_file);

                if (area->msgs_in == area->msgs_out) {
                    fglog("area %-35s: msgs in: %-3u out: %-3u size: %li",
                          area->area, area->msgs_in, area->msgs_out,
                          area->msgs_size);
                } else {
#ifndef FTN_ACL
                    fglog
                        ("area %-35s: msgs in: %-3u out: %-3u size: %li killed: %u/%u/%u/%u",
                         area->area, area->msgs_in, area->msgs_out,
                         area->msgs_size, area->msgs_routed,
                         area->msgs_insecure, area->msgs_dupe, area->msgs_path);
#else
                    fglog
                        ("area %-35s: msgs in: %-3u out: %-3u size: %li killed: %u/%u/%u/%u/%u",
                         area->area, area->msgs_in, area->msgs_out,
                         area->msgs_size, area->msgs_routed,
                         area->msgs_insecure, area->msgs_dupe, area->msgs_path,
                         area->msgs_readonly);
#endif                          /* !FTN_ACL */
                }
                log_program(PROGRAM);
            }
        }
    }

    if (pkts_in)
        fglog("pkts processed: %ld, %ld Kbyte in %ld s, %.2f Kbyte/s",
              pkts_in, pkts_bytes / 1024, (long)toss_delta,
              (double)pkts_bytes / 1024. / toss_delta);

    if (msgs_in) {
        fglog("msgs processed: %ld in, %ld out (%ld mail, %ld echo)",
              msgs_in, msgs_netmail + msgs_echomail, msgs_netmail,
              msgs_echomail);
        fglog("msgs processed: %ld in %ld s, %.6f msgs/s", msgs_in,
              (long)toss_delta, (double)msgs_in / toss_delta);
    }

    if (msgs_unknown || msgs_routed || msgs_insecure || msgs_empty)
        fglog
            ("msgs killed:    %ld empty, %ld unknown, %ld routed, %ld insecure",
             msgs_empty, msgs_unknown, msgs_routed, msgs_insecure);
#ifdef FTN_ACL
    if (msgs_readonly)
        fglog("msgs killed:    %ld to read only areas", msgs_readonly);
#endif                          /* FTN_ACL */
    if (dupe_check && msgs_dupe)
        fglog("msgs killed:    %ld dupe", msgs_dupe);
    if (check_path && msgs_path)
        fglog("msgs killed:    %ld circular path", msgs_path);

    if (notify.n > 0)
        send_request(&notify);

    if (ERROR == areasbbs_rewrite())
        fglog("error while rewriting areas.bbs");

    unlock_path(bbslock);

    exit_free();
    return c;
}
