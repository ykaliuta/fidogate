/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Route FTN NetMail/EchoMail
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

#include <utime.h>
#include <signal.h>

#define PROGRAM 	"ftnroute"
#define CONFIG		DEFAULT_CONFIG_MAIN

typedef struct st_nodelist {
    Node node;
    char mode;
    struct st_nodelist *next;
} nodelist;

/*
 * Prototypes
 */
short int repack_mode = FALSE;
int do_routing(char *, FILE *, Packet *);
int do_move(char *, FILE *, PktDesc *);
int do_cmd(PktDesc *, Routing *, Node *);
int do_packet(char *, FILE *, Packet *, PktDesc *);
void add_via(Textlist *, Node *);
int do_file(char *);
int do_repack(char *, char *, int);
void prog_signal(int);
void short_usage(void);
void usage(void);
int do_cmd_mkroute(PktDesc *, MkRoute *);
short int hi_init(char *);
int nodelist_read(void);
void hubroute_write(void);
nodelist *save_node(int, int, int, char);
int troute_scan(char *, int);

/*
 * Command line options
 */
int g_flag = 0;
int hist_init = FALSE;
char *nl_file = NULL;
nodelist *nl_list = NULL;
nodelist *rou_list = NULL;
nodelist *tru_list = NULL;
int zone = 0, net = 0, node = 0;

static char in_dir[MAXPATH];

static int severe_error = OK;   /* ERROR: exit after error */

static int signal_exit = FALSE; /* Flag: TRUE if signal received */

static bool strict;

#define MODE_PVT	1
#define MODE_HOLD	2
#define MODE_DOWN	3
#define MODE_NORMAL	4
#define MODE_HUB	5
#define MODE_HOST	6
#define MODE_ZONE	7

int troute_scan(char *file, int flag)
{
    FILE *fp;
    char *s;
    Node addr;
    nodelist *p, *last = NULL;
    int node, mode, stat = FALSE;

    debug(5, "reading %s file", file);

    fp = fopen(file, "r");
    if (!fp) {
        fglog("$ERROR: reading %s", file);
        return ERROR;
    }

    while ((s = fgets(buffer, sizeof(buffer), fp))) {
        if (*s == ';' || *s == '\r' || *s == '\0')
            continue;

        s = strtok(buffer, " \t");
        asc_to_node(s, &addr, FALSE);
        node = addr.node;
        mode = MODE_HUB;

        for (; s; s = strtok(NULL, " \t"), mode = MODE_NORMAL) {
            if (s && *s > 47 && *s < 58) {
                if (node == 0)
                    node = atoi(s);
                if ((p = save_node(addr.zone, addr.net, node, mode))) {
                    if (stat)
                        last->next = p;
                    else {
                        if (flag)
                            tru_list = p;
                        else
                            rou_list = p;
                        stat = TRUE;
                    }

                    last = p;
                }
                node = 0;
            }
        }
    }
    fclose(fp);

    return OK;
}

nodelist *save_node(int zone, int net, int node, char mode)
{
    nodelist *p;

    debug(9, "%d:%d/%d mode=%d", zone, net, node, mode);

    p = (nodelist *) xmalloc(sizeof(nodelist));

    node_clear(&p->node);
    p->next = NULL;
    p->node.zone = zone;
    p->node.net = net;
    p->node.node = node;
    p->mode = mode;

    return p;
}

char *f_read(char *buffer, int len, FILE * fp)
{
    char *p;
    long cf_lineno = 0;

    while (fgets(buffer, len, fp)) {
        cf_lineno++;
        strip_crlf(buffer);
        for (p = buffer; *p && is_space(*p); p++) ; /* Skip white spaces */
        if (*p != ';')
            return p;
    }
    return NULL;
}

 /*
  * Test nodelist mtime and update DB if need
  */
int nodelist_read(void)
{
    FILE *fp;
    char *p = NULL;
    char *s = NULL;
    char mode = 0;
    nodelist *p1, *nl_last = NULL;
    int zonegate = FALSE;
    int proc = FALSE;

    struct stat statnl, statdb;

    for (p = cf_get_string("NlFile", TRUE); p;
         p = cf_get_string("NlFile", FALSE)) {
        nl_file = p;

        if ((stat(nl_file, &statnl)) != 0) {
            fglog("$ERROR: can't stat file %s, %s", nl_file, strerror(errno));
            continue;
        }
        if ((stat(cf_p_hubroutedb(), &statdb)) == 0)
            if (statnl.st_mtime < statdb.st_mtime)
                continue;

        proc = TRUE;

        fp = fopen(nl_file, "r");
        if (!fp) {
            fglog("$ERROR: reading %s", nl_file);
            continue;
        }

        while (f_read(buffer, sizeof(buffer), fp)) {
            if (buffer[0] == ',' && (p = xstrtok(buffer + 1, ","))) {
                node = atoi(p);
                if (net == 0 || zonegate == TRUE) {
                    net = node;
                    node = 0;
                    zonegate = TRUE;
                }
                mode = MODE_NORMAL;
            } else if ((p = xstrtok(buffer, ",")) && (s = xstrtok(NULL, ","))) {
                if (!stricmp(p, "Zone")) {
                    zone = atoi(s);
                    net = node = 0;
                    mode = MODE_ZONE;
                } else if (!stricmp(p, "Host") || !stricmp(p, "Region")) {
                    net = atoi(s);
                    node = 0;
                    mode = MODE_HOST;
                    zonegate = FALSE;
                } else if (!stricmp(p, "Hub")) {
                    node = atoi(s);
                    mode = MODE_HUB;
                } else if (!stricmp(p, "Hold")) {
                    node = atoi(s);
                    mode = MODE_HOLD;
                } else if (!stricmp(p, "Down")) {
                    node = atoi(s);
                    mode = MODE_DOWN;
                } else if (!stricmp(p, "Pvt")) {
                    node = atoi(s);
                    mode = MODE_PVT;
                }
            }
            if ((p1 = save_node(zone, net, node, mode))) {
                if (nl_list)
                    nl_last->next = p1;
                else
                    nl_list = p1;
                nl_last = p1;
            }
        }
        fclose(fp);
    }
    if (!nl_file) {
        fglog("ERROR: can't find nodelist file in global config");
        return ERROR;
    }

    if (proc)
        hubroute_write();

    return OK;
}

/*
 * Write nodelist data to DB.
 */
void hubroute_write(void)
{
    nodelist *s, *s1;
    Node uplink, old, link;

    node_clear(&uplink);
    node_clear(&old);
    node_clear(&link);

    unlink(BUF_COPY2(buffer, cf_p_hubroutedb(), ".dir"));
    unlink(BUF_COPY2(buffer, cf_p_hubroutedb(), ".pag"));
    unlink(cf_p_hubroutedb());
    debug(9, "init database %s", cf_p_hubroutedb());
    hi_init(cf_p_hubroutedb());

    for (s = nl_list; s; s = s->next) {
        link = s->node;

        if (s->mode == MODE_HUB || s->mode == MODE_HOST || s->mode == MODE_ZONE) {
            uplink = link;
            sprintf(buffer, "%d$%s", s->mode, old.zone ? znfp1(&old) :
                    znfp2(&link));
            hi_write_avail(znfp2(&link), buffer);

            if (s->mode == MODE_HOST || s->mode == MODE_ZONE)
                old = link;
        } else {
            sprintf(buffer, "%d$%s", s->mode, znfp2(&uplink));
            hi_write_avail(znfp1(&link), buffer);
        }
    }
    hi_close();

    for (s = nl_list; s; s = s1) {
        s1 = s->next;
        xfree(s);
    }
}

/*
 * Route packet
 */
int do_routing(char *name, FILE * fp, Packet * pkt)
{
    PktDesc *desc;
    Routing *r;
    MkRoute *r1;
    LNode *p;
    Node match;

    desc = parse_pkt_name(name, &pkt->from, &pkt->to);
    if (desc == NULL)
        return ERROR;

    debug(2, "Source packet: from=%s to=%s grade=%c type=%c flav=%c",
          znfp1(&desc->from), znfp2(&desc->to),
          desc->grade, desc->type, desc->flav);

    /*
     * Search for matching routing commands
     */
    if (desc->type == TYPE_NETMAIL)
        for (r1 = mkroute_first; r1; r1 = r1->next) {
            if (do_cmd_mkroute(desc, r1))
                goto ready;
        }

    /*
     * Search for matching routing commands
     */
    for (r = routing_first; r; r = r->next)
        if (desc->type == r->type)
            for (p = r->nodes.first; p; p = p->next)
                if (node_match(&desc->to, &p->node)) {
                    match = p->node;
                    debug(4, "routing: type=%c cmd=%c flav=%c flav_new=%c "
                          "match=%s",
                          r->type, r->cmd, r->flav, r->flav_new,
                          s_znfp_print(&match, TRUE));

                    if (do_cmd(desc, r, &match))
                        goto ready;

                    break;      /* Inner for loop */
                }

 ready:
    /*
     * Write contents of this packet to output packet
     */
    debug(2, "Target packet: from=%s to=%s grade=%c type=%c flav=%c",
          znfp1(&desc->from), znfp2(&desc->to),
          desc->grade, desc->type, desc->flav);

    if (node_eq(&desc->to, &pkt->to))
        fglog("packet for %s (%s)", znfp1(&pkt->to), flav_to_asc(desc->flav));
    else
        fglog("packet for %s via %s (%s)", znfp1(&pkt->to),
              znfp2(&desc->to), flav_to_asc(desc->flav));

    return desc->move_only ? do_move(name, fp, desc)
        : do_packet(name, fp, pkt, desc);
}

/*
 * Move packet instead of copying (for SENDMOVE command)
 */
int do_move(char *name, FILE * fp, PktDesc * desc)
{
    long n;

    fclose(fp);

    n = outpkt_sequencer();
    outpkt_outputname(buffer, pkt_get_outdir(),
                      desc->grade, desc->type, desc->flav, n, "pkt");

    debug(5, "Rename %s -> %s", name, buffer);
    if (rename(name, buffer) == ERROR) {
        fglog("$ERROR: can't rename %s -> %s", name, buffer);
        return ERROR;
    }

    /* Set a/mtime to current time after renaming */
    if (utime(buffer, NULL) == ERROR) {
#ifndef __CYGWIN32__            /* Some problems with utime() here */
        fglog("$WARNING: can't set time of %s", buffer);
#endif
#if 0
        return ERROR;
#endif
    }

    return OK;
}

/*
 * Exec routing command
 */
int do_cmd(PktDesc * desc, Routing * r, Node * match)
{
    int ret = FALSE;

    switch (r->cmd) {
    case CMD_SEND:
        if (desc->flav == FLAV_NORMAL) {
            debug(4, "send %c %s", r->flav, znfp1(&desc->to));
            desc->flav = r->flav;
            desc->move_only = FALSE;
            /*
             * Special SEND syntax:
             *   send 1:2/3  ==  route 1:2/3.0 1:2/3.*
             */
            if (match->point == EMPTY)
                desc->to.point = 0;
            ret = TRUE;
        }
        break;

    case CMD_SENDMOVE:
        if (desc->flav == FLAV_NORMAL) {
            debug(4, "sendmove %c %s", r->flav, znfp1(&desc->to));
            desc->flav = r->flav;
            desc->move_only = TRUE;
            ret = TRUE;
        }
        break;

    case CMD_ROUTE:
        if (desc->flav == FLAV_NORMAL) {
            debug(4, "route %c %s -> %s", r->flav,
                  znfp1(&desc->to), znfp2(&r->nodes.first->node));
            desc->flav = r->flav;
            desc->to = r->nodes.first->node;
            desc->move_only = FALSE;
            ret = TRUE;
        }
        break;

    case CMD_CHANGE:
        if (desc->flav == r->flav) {
            debug(4, "change %c -> %c %s", r->flav, r->flav_new,
                  znfp1(&desc->to));
            desc->flav = r->flav_new;
            desc->move_only = FALSE;
            ret = TRUE;
        }
        break;

    default:
        debug(2, "unknown routing command, strange");
        break;
    }

    /*
     * Set all -1 values to 0
     */
    if (desc->to.zone == EMPTY || desc->to.zone == WILDCARD)
        desc->to.zone = 0;
    if (desc->to.net == EMPTY || desc->to.net == WILDCARD)
        desc->to.net = 0;
    if (desc->to.node == EMPTY || desc->to.node == WILDCARD)
        desc->to.node = 0;
    if (desc->to.point == EMPTY || desc->to.point == WILDCARD)
        desc->to.point = 0;

    return ret;
}

int do_cmd_mkroute(PktDesc * desc, MkRoute * r1)
{
    int ret = FALSE;

    switch (r1->cmd) {
    case CMD_XROUTE:
        if (node_match(&desc->from, &((r1->links).first->node)) &&
            node_match(&desc->to, &((r1->links).first->next->node))) {
            debug(4, "xroute: cmd=%c flav=%c match_from=%s, match_to=%s",
                  r1->cmd, r1->flav,
                  s_znfp_print(&((r1->links).first->node), TRUE),
                  s_znfp_print(&((r1->links).first->next->node), TRUE));
            desc->to = r1->uplink;
            desc->flav = r1->flav;
            desc->move_only = FALSE;
            ret = TRUE;
        }
        break;

    case CMD_HOSTROUTE:
        if (lon_search_wild(&r1->links, &desc->to)) {
            debug(4, "hostroute %c %s", r1->flav, znfp1(&desc->to));
            desc->flav = r1->flav;
            desc->to.node = 0;
            desc->to.point = 0;
            desc->move_only = FALSE;
            ret = TRUE;
        }
        break;

    case CMD_HUBROUTE:
        if (lon_search_wild(&r1->links, &desc->to)) {
            char *p, *p1;
            Node node;

            if (!hist_init && nodelist_read()) {
                hi_init(cf_p_hubroutedb());
                hist_init = TRUE;
            }
            if ((p = hi_fetch(znfp1(&desc->to), 1))) {
                p1 = xstrtok(p, " $");
                if (*p1 && znfp_parse_partial(p, &node)) {
                    desc->to = node;
                    desc->to.point = 0;
                    desc->move_only = FALSE;
                    ret = TRUE;
                }
            } else {
                fglog("ERROR: hub for node=%s not found", znfp1(&desc->to));
                break;
            }

            desc->move_only = FALSE;
            ret = TRUE;
        }
        break;

    case CMD_BOSSROUTE:
        if (desc->flav == r1->flav && lon_search_wild(&r1->links, &desc->to)) {
            debug(4, "route %c %s -> boss", r1->flav, znfp1(&desc->to));
            desc->to.point = 0;
            desc->move_only = FALSE;
            ret = TRUE;
        }
        break;
    }

    /*
     * Set all -1 values to 0
     */
    if (desc->to.zone == EMPTY || desc->to.zone == WILDCARD)
        desc->to.zone = 0;
    if (desc->to.net == EMPTY || desc->to.net == WILDCARD)
        desc->to.net = 0;
    if (desc->to.node == EMPTY || desc->to.node == WILDCARD)
        desc->to.node = 0;
    if (desc->to.point == EMPTY || desc->to.point == WILDCARD)
        desc->to.point = 0;

    return ret;

}

/*
 * Add our ^AVia line
 */
void add_via(Textlist * list, Node * gate)
{
    if (!repack_mode)
#ifndef FTS_VIA
        tl_appendf(list, "\001Via FIDOGATE/%s %s, %s\r",
                   PROGRAM, znf1(gate), date(DATE_VIA, NULL));
#else
        tl_appendf(list, "\001Via %s @%s FIDOGATE/%s\r",
                   znf1(gate), date(DATE_VIA, NULL), PROGRAM);
#endif                          /* FTS_VIA */
}

/*
 * Read and process packets, writing messages to output packet
 */
int do_packet(char *pkt_name, FILE * pkt_file, Packet * pkt, PktDesc * desc)
{
    Message msg = { 0 };        /* Message header */
    Textlist tl;                /* Textlist for message body */
    MsgBody body;               /* Message body of FTN message */
    int type, ret;
    FILE *fp;

    /*
     * Initialize
     */
    tl_init(&tl);
    msg_body_init(&body);

    /*
     * Open output packet
     */
    cf_set_zone(desc->to.zone);

    fp = outpkt_open(&desc->from, &desc->to,
                     desc->grade, desc->type, desc->flav, FALSE);
    if (fp == NULL) {
        fclose(pkt_file);
        TMPS_RETURN(ERROR);
    }

    /*
     * Read message from input packet and write to output packet
     */
    type = pkt_get_int16(pkt_file);
    ret = OK;

    while (type == MSG_TYPE && !xfeof(pkt_file)) {
        /*
         * Read message header
         */
        msg.node_from = pkt->from;
        msg.node_to = pkt->to;
        if (pkt_get_msg_hdr(pkt_file, &msg, strict) == ERROR) {
            fglog("ERROR: reading input packet");
            ret = ERROR;
            break;
        }

        if (msg.attr & MSG_DIRECT && !node_eq(&desc->to, &pkt->to)) {
            fclose(fp);
            debug(1, "routing: direct (%s)", znfp1(&pkt->to));
            desc->to = pkt->to;
            return do_move(pkt_name, pkt_file, desc);
        }

        /*
         * Read & parse message body
         */
        if (pkt_get_body_parse(pkt_file, &body, &msg.node_from, &msg.node_to) !=
            OK) {
            fglog("ERROR: parsing message body");
            fclose(pkt_file);
            TMPS_RETURN(ERROR);
        }

        if (body.area == NULL) {
            /*
             * NetMail
             */
            /* Retrieve address from kludges */
            kludge_pt_intl(&body, &msg, TRUE);
            /* Write message header and body */
            if (pkt_put_msg_hdr(fp, &msg, TRUE) != OK) {
                ret = severe_error = ERROR;
                break;
            }
            if (msg_put_msgbody(fp, &body) != OK) {
                ret = severe_error = ERROR;
                break;
            }
        } else {
            /*
             * EchoMail
             */
            /* Write message header and body */
            if (pkt_put_msg_hdr(fp, &msg, FALSE) != OK) {
                ret = severe_error = ERROR;
                break;
            }
            if (msg_put_msgbody(fp, &body) != OK) {
                ret = severe_error = ERROR;
                break;
            }
        }

        /*
         * Exit if signal received
         */
        if (signal_exit) {
            ret = severe_error = ERROR;
            break;
        }

        tmps_freeall();
    } /**while(type == MSG_TYPE)**/

    if (fclose(pkt_file) == ERROR) {
        fglog("$ERROR: can't close packet %s", pkt_name);
        TMPS_RETURN(severe_error = ERROR);
    }

    if (ret == OK && *pkt_name != '\0')
        if (unlink(pkt_name)) {
            fglog("$ERROR: can't unlink packet %s", pkt_name);
            TMPS_RETURN(ERROR);
        }

    TMPS_RETURN(ret);
}

/*
 * Process one packet file
 */
int do_file(char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;

    /*
     * Open packet and read header
     */
    pkt_file = fopen(pkt_name, R_MODE);
    if (!pkt_file) {
        fglog("$ERROR: can't open packet %s", pkt_name);
        TMPS_RETURN(ERROR);
    }

    if (pkt_get_hdr(pkt_file, &pkt) == ERROR) {
        fglog("ERROR: reading header from %s", pkt_name);
        TMPS_RETURN(ERROR);
    }

    /*
     * Route it
     */
    if (do_routing(pkt_name, pkt_file, &pkt) == ERROR) {
        fglog("ERROR: in processing %s", pkt_name);
        TMPS_RETURN(ERROR);
    }

    TMPS_RETURN(OK);
}

int do_repack(char *dir, char *wildcard, int repack_time)
{

    time_t now;
    char *pkt_name;
    char buf[MAXPATH];

    if (repack_time != -1)
        now = time(NULL) - repack_time;
    else
        now = 0;

    dir_sortmode(DIR_SORTMTIME);

    if (dir_open(dir, wildcard, TRUE) == ERROR) {
        return ERROR;
    }

    for (pkt_name = dir_get_mtime(now, TRUE); pkt_name;
         pkt_name = dir_get_mtime(now, FALSE)) {
#ifdef DO_BSY_FILES
        BUF_COPY(buf, pkt_name);
        buf[strlen_zero(buf) - 3] = '\0';
        BUF_APPEND(buf, "bsy");

#ifdef NFS_SAFE_LOCK_FILES
        if (lock_lockfile_nfs(buf, NOWAIT, NULL) == ERROR)
#else
        if (lock_lockfile(buf, NOWAIT) == ERROR)
#endif
            continue;
#endif                          /* DO_BSY_FILES */

        sprintf(buf, "%s/toss/route/m%c%c%05lx.pkt", cf_p_spooldir(),
                TYPE_NETMAIL, FLAV_NORMAL, outpkt_sequencer());

        debug(5, "rename %s -> %s", pkt_name, buf);
        rename(pkt_name, buf);

        if (do_file(buf) == ERROR) {
            dir_close();
            debug(3, "do_file(): error processing");
            return ERROR;
        }
        if (unlink(buf)) {
            debug(1, "$ERROR: can't unlink packet %s", buf);
        }
    }
    dir_close();

    return OK;
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
options: -g --grade G                 processing grade\n\
         -I --in-dir NAME             set input packet directory\n\
         -O --out-dir NAME            set output packet directory\n\
         -l --lock-file               create lock file while processing\n\
         -r --routing-file NAME       read routing file\n\
         -M --maxopen N               set max # of open packet files\n\
	 -p --repack HOURS            repack old mail in outbound\n\
\n\
	 -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config NAME             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n");
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, ret;
    char *p;
    short int l_flag = FALSE;
    char p_flag = FALSE;
    char *I_flag = NULL, *O_flag = NULL, *r_flag = NULL, *M_flag = NULL;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    int repack_time = -1;
    char *pkt_name;
    char pattern[16];
    int aso = FALSE;

    int option_index;
    static struct option long_options[] = {
        {"grade", 1, 0, 'g'},   /* grade */
        {"in-dir", 1, 0, 'I'},  /* Set inbound packets directory */
        {"lock-file", 0, 0, 'l'},   /* Create lock file while processing */
        {"out-dir", 1, 0, 'O'}, /* Set packet directory */
        {"routing-file", 1, 0, 'r'},    /* Set routing file */
        {"maxopen", 1, 0, 'M'}, /* Set max # open packet files */
        {"repack", 1, 0, 'p'},

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"uplink-addr", 1, 0, 'u'}, /* Set FIDO uplink address */
        {0, 0, 0, 0}
    };

    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "g:O:pI:lr:M:vhc:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftnroute options *****/
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
        case 'r':
            r_flag = optarg;
            break;
        case 'M':
            M_flag = optarg;
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
    if (!M_flag && (p = cf_get_string("MaxOpenFiles", TRUE))) {
        M_flag = p;
    }
    if (M_flag)
        outpkt_set_maxopen(atoi(M_flag));

    if (cf_get_string("AmigaStyleOutbound", TRUE) != NULL)
        aso = TRUE;

    strict = (cf_get_string("FTNStrictPktCheck", TRUE) != NULL);
    /*
     * Process local options
     */
    BUF_EXPAND(in_dir, I_flag ? I_flag : cf_p_toss_toss());
    pkt_outdir(O_flag ? O_flag : cf_p_toss_route(), NULL);

    routing_init(r_flag ? r_flag : cf_p_routing());
    passwd_init();

    /* Install signal/exit handlers */
    signal(SIGHUP, prog_signal);
    signal(SIGINT, prog_signal);
    signal(SIGQUIT, prog_signal);

    ret = EXIT_OK;

    if (p_flag) {
        char buf[MAXPATH];
        char *base;
        char *btbase;

        if ((p = cf_get_string("RepackMailTime", TRUE))) {
            repack_time = atoi(p) * 3600;
        }

        btbase = cf_p_btbasedir();

        BUF_COPY(pattern, "*.[codh]ut");
        if (g_flag)
            pattern[0] = g_flag;

        /* Lock file */
        if (l_flag)
            if (lock_program(PROGRAM, NOWAIT) == ERROR) {
                /* Already busy */
                exit_free();
                return EXIT_BUSY;
            }

        repack_mode = TRUE;

        if (aso) {

            if ((base = cf_out_get(0)) == NULL) {
                fglog("$ERROR: can't find ASO outbound directory (%s/..)",
                      btbase);
                exit_free();
                return EX_OSERR;
            } else {
                BUF_COPY3(buf, btbase, "/", base);

                if (do_repack(buf, pattern, repack_time) == ERROR) {
                    debug(1, "error processing %s", buf);
                }
            }
        } else {                /* non-aso */
            for (c = 0; (base = cf_out_get(c)) != NULL; c++) {
                BUF_COPY3(buf, btbase, "/", base);

                if (do_repack(buf, pattern, repack_time) == ERROR) {
                    debug(1, "error processing %s", buf);
                    continue;
                }
                /* Open and read directory */
                if (dir_open(buf, "?????????.pnt", TRUE) == ERROR) {
                    fglog("$ERROR: can't open directory %s", buf);
                    exit_free();
                    return EX_OSERR;
                } else {
                    char *buf2;
                    for (buf2 = dir_get(TRUE); buf2; buf2 = dir_get(FALSE)) {
                        if (do_repack(buf2, pattern, repack_time) == ERROR) {
                            ret = EXIT_ERROR;
                            break;
                        }
                    }
                    dir_close();
                    if (ret == EXIT_ERROR)
                        break;
                }
            }
        }                       /* non-aso */
        /* Lock file */

        if (l_flag)
            unlock_program(PROGRAM);

    } else if (optind >= argc) {
        BUF_COPY(pattern, "????????.pkt");
        if (g_flag)
            pattern[0] = g_flag;

        /* process packet files in directory */
        dir_sortmode(DIR_SORTMTIME);
        if (dir_open(in_dir, pattern, TRUE) == ERROR) {
            fglog("$ERROR: can't open directory %s", in_dir);
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

        repack_mode = FALSE;

        for (pkt_name = dir_get(TRUE); pkt_name; pkt_name = dir_get(FALSE))
            if (do_file(pkt_name) == ERROR) {
                ret = EXIT_ERROR;
                break;
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

        /*
         * Process packet files on command line
         */
        repack_mode = FALSE;

        for (; optind < argc; optind++)
            if (do_file(argv[optind]) == ERROR) {
                ret = EXIT_ERROR;
                break;
            }

        /* Lock file */
        if (l_flag)
            unlock_program(PROGRAM);
    }

    outpkt_close();

    if (p_flag) {
        BUF_COPY4(buffer, cf_p_libexecdir(), "/ftnpack -I ", cf_p_spooldir(),
                  "/toss/route");
        debug(9, "exec: %s", buffer);

        if (verbose) {
            log_file("stdout");
            BUF_APPEND(buffer, " -");
            for (c = verbose; c; c--)
                BUF_APPEND(buffer, "v");
        }

        c = run_system(buffer);
        debug(5, "ftnpack exit (%d)", c);
    }

    if (hist_init)
        hi_close();

    exit_free();
    return ret;
}
