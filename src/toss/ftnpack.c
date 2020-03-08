/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Pack output packets of ftnroute for Binkley outbound (ArcMail)
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

#define BUF_LAST(d)			str_last  (d,sizeof(d))

#define PROGRAM 	"ftnpack"
#define CONFIG		DEFAULT_CONFIG_MAIN

/*
 * Command line options
 */
int g_flag = 0;
int pkt_flag = FALSE;           /* -P --pkt */
long maxarc = 0;                /* -m --maxarc SIZE */

static char in_dir[MAXPATH];
static char out_dir[MAXPATH];
static char file_attach_dir[MAXPATH];

static int severe_error = OK;   /* ERROR: exit after error */

static int signal_exit = FALSE; /* Flag: TRUE if signal received */

int bundle_disp;

static bool strict;

/* "noarc" packer program */
static ArcProg noarc = {
    PACK_ARC, "noarc", NULL, NULL
};

/* packer programs linked list */
static ArcProg *arcprog_first = &noarc;
static ArcProg *arcprog_last = &noarc;

/* packing commands linked list */
static Packing *packing_first = NULL;
static Packing *packing_last = NULL;

static long last_arcmail_chars = 35;

/*
 * Prototypes
 */
int str_last(char *, size_t);
int parse_pack(char *);
ArcProg *parse_arc(char *);
void new_arc(int);
Packing *packing_parse_line(char *);
int packing_do_file(char *);
void packing_init(char *);
char *arcmail_name(Node *, char *);
char *packing_pkt_name(char *, char *);
int arcmail_search(char *, long, long);
int do_arcmail(char *, Node *, Node *, PktDesc *, FILE *, char *, char *, long);
int do_noarc(char *, Node *, PktDesc *, FILE *, char *);
void set_zero(Node *);
int do_pack(PktDesc *, char *, FILE *, Packing *);
int do_dirpack(PktDesc *, char *, FILE *, Packing *);
int do_packing(char *, FILE *, Packet *);
int do_packet(FILE *, Packet *, PktDesc *);
void add_via(Textlist *, Node *);
int do_file(char *);
void prog_signal(int);
void short_usage(void);
void usage(void);

/*
 * Parse PACKING commands
 */
int parse_pack(char *s)
{
    if (!stricmp(s, "netmail"))
        return TYPE_NETMAIL;
    if (!stricmp(s, "echomail"))
        return TYPE_ECHOMAIL;
    if (!stricmp(s, "pack"))
        return PACK_NORMAL;
    if (!stricmp(s, "rpack"))
        return PACK_ROUTE;
    if (!stricmp(s, "fpack"))
        return PACK_FLO;
    if (!stricmp(s, "dirpack"))
        return PACK_DIR;
    if (!stricmp(s, "dirmove"))
        return PACK_MOVE;
    if (!stricmp(s, "arc"))
        return PACK_ARC;
    if (!stricmp(s, "prog"))
        return PACK_PROG;
    if (!stricmp(s, "progn"))
        return PACK_PROGN;
    if (!stricmp(s, "lastarcmailchars"))
        return PACK_LAC;
    if (!stricmp(s, "unarc"))
        return 0;

    return ERROR;
}

/*
 * Parse archiver/program name
 */
ArcProg *parse_arc(char *s)
{
    ArcProg *p;

    for (p = arcprog_first; p; p = p->next)
        if (!stricmp(p->name, s))
            return p;
    return NULL;
}

/*
 * Define new archiver/program
 */
void new_arc(int cmd)
{
    char *name, *prog;
    ArcProg *a;

    name = xstrtok(NULL, " \t");
    prog = xstrtok(NULL, " \t");
    if (!name || !prog) {
        fglog("packing: missing argument for arc/prog definition");
        return;
    }

    /* Create new entry and put into list */
    a = (ArcProg *) xmalloc(sizeof(ArcProg));
    a->pack = cmd;
    a->name = strsave(name);
    a->prog = strsave(prog);
    a->next = NULL;

    if (arcprog_first)
        arcprog_last->next = a;
    else
        arcprog_first = a;
    arcprog_last = a;

    debug(15, "packing: pack=%c name=%s prog=%s", a->pack, a->name, a->prog);
}

/*
 * Read PACKING config file
 */
Packing *packing_parse_line(char *buf)
{
    Packing *r;
    ArcProg *a;
    char *p;
    char *dir;
    Node old, node;
    LON lon;
    int cmd;
    static int type = TYPE_NETMAIL;
    long ma = 0;

    /* Command */
    p = xstrtok(buf, " \t");
    if (!p)
        return NULL;
    if (strieq(p, "include")) {
        p = xstrtok(NULL, " \t");
        packing_do_file(p);
        return NULL;
    }
    if ((cmd = parse_pack(p)) == ERROR) {
        fglog("packing: unknown command %s", p);
        return NULL;
    }
    if (cmd == 0)
        return NULL;

    /* Definition of new archiver/program */
    if (cmd == PACK_ARC || cmd == PACK_PROG || cmd == PACK_PROGN) {
        new_arc(cmd);
        return NULL;
    }

    if (cmd == TYPE_NETMAIL || cmd == TYPE_ECHOMAIL) {
        type = cmd;
        return NULL;
    }
    /* Directory argument */
    if (cmd == PACK_DIR || cmd == PACK_MOVE) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("packing: directory argument missing");
            return NULL;
        }
        dir = strsave(p);
    } else
        dir = NULL;

    if (cmd == PACK_LAC) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("lastarcmailchars: argument missing");
            return NULL;
        }
        /* Parameter for "lastarcmailchars" command */
        last_arcmail_chars = atol(p);
        if (last_arcmail_chars == 0)
            last_arcmail_chars = 9;
        else if (last_arcmail_chars == 1)
            last_arcmail_chars = 15;
        else if (last_arcmail_chars == 2)
            last_arcmail_chars = 35;
        else if (last_arcmail_chars < 0 || last_arcmail_chars > 35)
            last_arcmail_chars = 35;
        debug(15, "packing: last_arcmail_char=%li", last_arcmail_chars);
        return NULL;
    }

    /* Archiver name argument */
    if (cmd != PACK_MOVE) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("packing: archiver name argument missing");
            return NULL;
        }
        if ((a = parse_arc(p)) == NULL) {
            fglog("packing: unknown archiver/program %s", p);
            return NULL;
        }
    } else
        a = NULL;

    /* List of nodes follows */
    node_invalid(&old);
    old.zone = cf_zone();
    lon_init(&lon);

    p = xstrtok(NULL, " \t=");
    if (!stricmp(p, "size")) {
        p = xstrtok(NULL, " \t");
        ma = atol(p) * 1024L;
        p = xstrtok(NULL, " \t");
    }
    if (!ma)
        ma = maxarc;
    if (!p) {
        fglog("packing: node address argument missing");
        return NULL;
    }
    while (p) {
        if (znfp_parse_diff(p, &node, &old) == ERROR) {
            fglog("packing: illegal node address %s", p);
        } else {
            old = node;
            lon_add(&lon, &node);
        }

        p = xstrtok(NULL, " \t");
    }

    /* Create new entry and put into list */
    r = (Packing *) xmalloc(sizeof(Packing));
    r->pack = cmd;
    r->dir = dir;
    r->arc = a;
    r->type = type;
    r->nodes = lon;
    r->next = NULL;
    if (!r->maxarc)
        r->maxarc = ma;

    debug(15, "packing: pack=%c dir=%s arc=%s type=%c, maxarc=%ld",
          r->pack, r->dir ? r->dir : "", (r->arc ? r->arc->name : NULL),
          r->type, r->maxarc);
    lon_debug(15, "packing: nodes=", &r->nodes, TRUE);

    return r;
}

int packing_do_file(char *name)
{
    FILE *fp;
    Packing *r;

    debug(14, "Reading packing file %s", name);

    fp = xfopen(name, R_MODE);

    while (cf_getline(buffer, BUFFERSIZE, fp)) {
        r = packing_parse_line(buffer);
        if (!r)
            continue;

        /* Put into linked list */
        if (packing_first)
            packing_last->next = r;
        else
            packing_first = r;
        packing_last = r;
    }

    fclose(fp);

    return OK;
}

void packing_init(char *name)
{
    packing_do_file(name);
}

/*
 * Return name for ArcMail archive
 */
char *arcmail_name(Node * node, char *dir)
{
    static char buf[MAXPATH];
    static char *wk0[] = { "su0", "mo0", "tu0", "we0", "th0", "fr0", "sa0" };
    char *wk, *base;
    TIMEINFO ti;
    struct tm *tm;
    int d1, d2;
    size_t rest;

    int aso = FALSE;

    if (cf_get_string("AmigaStyleOutbound", TRUE) != NULL)
        aso = TRUE;

    cf_set_zone(node->zone);

    if (dir) {
        /* Passed directory name */
        BUF_COPY(buf, dir);
        if (buf[str_last(buf, sizeof(buf))] != '/')
            BUF_APPEND(buf, "/");
    } else {
        /* Outbound dir + zone dir */
        if (aso)
            base = cf_zones_out(0);
        else
            base = cf_zones_out(node->zone);

        if (base == NULL)
            return NULL;
        BUF_COPY4(buf, cf_p_btbasedir(), "/", base, "/");
    }
    base = buf + strlen(buf);
    rest = sizeof(buf) - strlen(buf);

    /*
     * Get weekday archive extension
     */
    GetTimeInfo(&ti);
    tm = localtime(&ti.time);
    wk = wk0[tm->tm_wday];

    /*
     * Create name of archive file
     */
    if (aso) {
        str_printf(base, rest, "%d.%d.%d.%d.%s", node->zone, node->net,
                   node->node, node->point, wk);
    } else {
        if (node->point > 0) {
            d1 = 0;
            d2 = (cf_main_addr()->point - node->point) & 0xffff;

            if (dir)
                str_printf(base, rest, "%04x%04x.%s", d1, d2, wk);
            else
                str_printf(base, rest, "%04x%04x.pnt/%04x%04x.%s",
                           node->net, node->node, d1, d2, wk);
        } else {
            d1 = (cf_main_addr()->net - node->net) & 0xffff;
            d2 = (cf_main_addr()->node - node->node) & 0xffff;

            str_printf(base, rest, "%04x%04x.%s", d1, d2, wk);
        }
    }

    return buf;
}

/*
 * Return packet name for adding to archive
 */
char *packing_pkt_name(char *dir, char *name)
{
    static char buf[MAXPATH];
    char *p;

#if 0
    /* Return nnnnnnnn.pkt in dir */
    str_printf(buf, sizeof(buf), "%s/%08ld.pkt",
               (dir ? dir : out_dir), sequencer(cf_p_seq_pack));
#endif

    /* Same base name in dir */
    p = strrchr(name, '/');
    if (!p)                     /* Just to be sure */
        p = name;
    else
        p++;
    BUF_COPY3(buf, (dir ? dir : out_dir), "/", p);

    return buf;
}

/*
 * Search for existing ArcMail archives in the outbound and changes
 * name to the latest one. If an empty, truncated ArcMail archive
 * exists, it is deleted and the name is set to the next number.
 */
int arcmail_search(char *name, long max_arc, long psize)
{
    int c, cc;
    char *p;
    long size;
    int is_old;
    static unsigned char C[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    long minarc = 1 * 1024L * 1024L * 1024L;
    int fm = 1, fms = 0, ne = 1, nne = 0, sok = 0;

    p = name + strlen(name) - 1;    /* Position of digit */

    *p = C[last_arcmail_chars];
    if (check_access(name, CHECK_FILE) == TRUE && (size = check_size(name)) == 0) { /* Switch to the next cicle C[last_arcmail_chars] -> C[0] */
        if (unlink(name) == ERROR)
            fglog("$ERROR: can't remove %s", name);
        debug(4, "Removed %s", name);
    }

    cc = C[0];

    for (c = last_arcmail_chars; c >= 0; c--) {
        *p = C[c];

        if (check_access(name, CHECK_FILE) == TRUE) {   /* Found it */
            if ((size = check_size(name)) == ERROR) {
                fglog("$ERROR: can't stat %s", name);
                continue;
            }
            if (size == 0) {    /* Empty archive */
                is_old = check_old(name, 24 * 60 * 60L);    /* > 24 h */

                if (unlink(name) == ERROR)
                    fglog("$ERROR: can't remove %s", name);
                debug(4, "Removed %s", name);
                if (is_old)
                    continue;

                nne = C[c];
                ne = 0;         /* Search for the first unused extension */

                if (cc == C[0])
                    cc = c < last_arcmail_chars ? C[c + 1] : C[c];  /* Next number, mo0 -> mo1 */
            }
            if (max_arc && size >= (max_arc + psize / 2)) { /* Max size exceeded */
                if (cc == C[0])
                    cc = c < last_arcmail_chars ? C[c + 1] : C[c];  /* Next number, mo0 -> mo1 */
            } else if (size) {  /* Search for the first file with size Ok */
                sok = C[c];
                fm = 0;
            }
            if (size && size < minarc) {    /* Search for archive with minimal size, if       */
                /* all extensions are in use (extremal situation) */
                fms = C[c];
                minarc = size;
            }
            if (cc == C[0])
                cc = C[c];
        } else {                /* Search for the first unused extension */
            nne = C[c];
            ne = 0;
        }
    }

    if (!fm)                    /* if file size is Ok */
        cc = sok;
    else if ((cc == C[last_arcmail_chars]) && (*p = cc) && (check_access(name, CHECK_FILE) == TRUE) && ((size = check_size(name)) > max_arc)) { /* If an archive suitable for appending data was not found */
        if (ne)                 /* If no holes in numeration were found, we append data    */
            cc = fms;           /* to the archive with minimal size, which will be bigger  */
        else                    /* than maxarc, (extremal situation)                       */
            cc = nne;           /* If we have found a hole in numeration, we create a      */
    }                           /* new archive                                             */

    *p = cc;                    /* Set archive name digit */

    debug(4, "Archive name %s", name);

    return OK;
}

/*
 * Pack packet to ArcMail archive
 */
int do_arcmail(char *name, Node * arcnode, Node * flonode,
               PktDesc * desc, FILE * file, char *prog, char *dir, long max_arc)
{
    char *arcn, *pktn;
    int ret;

    arcn = arcmail_name(arcnode, dir);
    pktn = packing_pkt_name(NULL, name);
    if (!arcn)
        return ERROR;
    if (arcmail_search(arcn, max_arc, check_size(pktn)) == ERROR)
        return ERROR;

    if (bink_mkdir(arcnode) == ERROR) {
        fglog("ERROR: can't create outbound dir");
        return ERROR;
    }

    debug(4, "Archiving %s for %s arc", name, znfp1(arcnode));
    debug(4, "    Packet  name: %s", pktn);
    debug(4, "    Archive name: %s", arcn);

    /* Rename/copy packet */
    if (desc->type == TYPE_ECHOMAIL) {
        fclose(file);
        /* Simply rename */
        if (rename(name, pktn) == -1) {
            fglog("$ERROR: rename %s -> %s failed", name, pktn);
            return ERROR;
        }
    } else {
        /* Copy and process file attaches */
        if (do_noarc(name, flonode, desc, file, pktn) == ERROR) {
            fglog("ERROR: copying/processing %s -> %s failed", name, pktn);
            return ERROR;
        }
    }

    str_printf(buffer, sizeof(buffer), prog, arcn, pktn);
    debug(4, "Command: %s", buffer);
    ret = run_system(buffer);
    debug(4, "Exit code=%d", ret);
    chmod(arcn, PACKET_MODE);
    if (ret) {
        fglog("ERROR: %s failed, exit code=%d", buffer, ret);
        return ERROR;
    }
    chmod(arcn, PACKET_MODE);
    if (unlink(pktn) == -1)
        fglog("$ERROR: can't remove %s", pktn);
    if (!dir)
        switch (bundle_disp) {
        case 0:
            return bink_attach(flonode, '#', arcn,
                               flav_to_asc(desc->flav), FALSE);
        case 1:
            return bink_attach(flonode, '^', arcn,
                               flav_to_asc(desc->flav), FALSE);
        case 2:
            return bink_attach(flonode, '@', arcn,
                               flav_to_asc(desc->flav), FALSE);
        }

    return OK;
}

/*
 * Write packet contents to .OUT packet in Binkley outbound
 */
int do_noarc(char *name, Node * flonode,
             PktDesc * desc, FILE * pkt_file, char *out_name)
    /* outname --- name of .pkt file, if not NULL */
{
    FILE *fp;
    Message msg = { 0 };
    Textlist tl;
    int type;

    /*
     * Open .OUT packet in Binkley outbound
     */
    cf_set_best(desc->to.zone, desc->to.net, desc->to.node);

    fp = out_name ? pkt_open(out_name, &desc->to, NULL, FALSE)
        : pkt_open(NULL, &desc->to, flav_to_asc(desc->flav), FALSE);
    if (fp == NULL) {
        fglog("ERROR: can't open outbound packet for %s", znfp1(&desc->to));
        fclose(pkt_file);
        TMPS_RETURN(ERROR);
    }

    tl_init(&tl);

    /*
     * Read messages from packet
     */
    type = pkt_get_int16(pkt_file);
    while ((type == MSG_TYPE) && !xfeof(pkt_file)) {
        /* Read message header */
        node_clear(&msg.node_from);
        node_clear(&msg.node_to);
        if (pkt_get_msg_hdr(pkt_file, &msg, strict) != OK) {
            fglog("$ERROR reading input packet %s", name);
            pkt_close();
            fclose(pkt_file);
            TMPS_RETURN(ERROR);
        }
        /* Read message body */
        type = pkt_get_body(pkt_file, &tl);
        if (type == ERROR) {
            fglog("$ERROR: reading input packet %s", name);
            pkt_close();
            fclose(pkt_file);
            TMPS_RETURN(ERROR);
        }

        /* File attaches */
        if (desc->type == TYPE_NETMAIL && (msg.attr & MSG_FILE)) {
            char *fa_name = msg.subject, *p;
            int ret, file_attach_action = 0;
            long sz;

            if ((p = cf_get_string("IgnoreFileAttach", TRUE))) {
                file_attach_action = atoi(p);
            }

            /* File attachments from gateway (runtoss out) */
            if (streq(file_attach_dir, "/")) {
                /* fa_name in subject is complete path */
                if (file_attach_action == 0) {
                    sz = check_size(fa_name);
                    ret = bink_attach(flonode, 0, fa_name,
                                      flav_to_asc(desc->flav), FALSE);
                    if (ret == ERROR)
                        fglog("ERROR: file attach %s for %s failed",
                              fa_name, znfp1(&desc->to));
                    else
                        fglog("file attach %s (%ldb) for %s",
                              fa_name, sz, znfp1(&desc->to));
                } else if (file_attach_action == 1)
                    unlink(fa_name);
            }
            /* File attachments from inbound directory */
            else if (file_attach_dir[0]) {
                if (dir_search(file_attach_dir, fa_name)) {
                    BUF_COPY3(buffer, file_attach_dir, "/", fa_name);
                    if (file_attach_action == 0) {
                        sz = check_size(buffer);
                        ret = bink_attach(flonode, '^', buffer,
                                          flav_to_asc(desc->flav), FALSE);
                        if (ret == ERROR)
                            fglog("ERROR: file attach %s for %s failed",
                                  msg.subject, znfp1(&desc->to));
                        else
                            fglog("file attach %s (%ldb) for %s",
                                  msg.subject, sz, znfp1(&desc->to));
                    } else if (file_attach_action == 1)
                        unlink(buffer);
                } else
                    fglog("file attach %s: no such file", msg.subject);
            } else
                fglog("file attach %s not processed, no -F option", fa_name);
        }

        /* Write message header */
        if (pkt_put_msg_hdr(fp, &msg, FALSE) != OK) {
            fglog("$ERROR: writing packet %s", pkt_name());
            pkt_close();
            fclose(pkt_file);
            TMPS_RETURN(ERROR);
        }
        /* Write message body */
        tl_print(&tl, fp);
        /* Terminating 0-byte */
        putc(0, fp);
        if (ferror(fp) != OK) {
            fglog("$ERROR: writing packet %s", pkt_name());
            pkt_close();
            fclose(pkt_file);
            TMPS_RETURN(ERROR);
        }

        tmps_freeall();
    } /**while**/

    fclose(pkt_file);
    if (pkt_close() != OK) {
        fglog("$ERROR: can't close outbound packet");
        TMPS_RETURN(ERROR);
    }
    if (unlink(name) != OK) {
        fglog("$ERROR: can't remove packet %s", name);
        TMPS_RETURN(ERROR);
    }

    TMPS_RETURN(OK);
}

/*
 * Call program for packet file
 */
int do_prog(char *name, PktDesc * desc, Packing * pack)
{
    int ret;

    debug(4, "Processing %s", name);

    str_printf(buffer, sizeof(buffer), pack->arc->prog, name);
    debug(4, "Command: %s", buffer);
    ret = run_system(buffer);
    debug(4, "Exit code=%d", ret);
    if (ret) {
        fglog("ERROR: %s failed, exit code=%d", buffer, ret);
        return ERROR;
    }
    if (pack->arc->pack != PACK_PROGN)
        if (unlink(name) == ERROR)
            fglog("$ERROR: can't remove %s", name);

    return OK;
}

/*
 * Set all -1 Node components to 0
 */
void set_zero(Node * node)
{
    if (node->zone == -1)
        node->zone = 0;
    if (node->net == -1)
        node->net = 0;
    if (node->node == -1)
        node->node = 0;
    if (node->point == -1)
        node->point = 0;
}

/*
 * Pack packet to ArcMail archive or copy to outbound .OUT
 */
int do_pack(PktDesc * desc, char *name, FILE * file, Packing * pack)
{
    int ret = OK;
    Node arcnode, flonode;

    arcnode = desc->to;
    flonode = desc->to;

    if (pack->pack == PACK_ROUTE) {
        /* New archive node */
        arcnode = pack->nodes.first->node;
        /* New FLO node */
        flonode = pack->nodes.first->node;
    }
    if (pack->pack == PACK_FLO) {
        /* New FLO node */
        flonode = pack->nodes.first->node;
    }

    /* Set all -1 values to 0 */
    set_zero(&arcnode);
    set_zero(&flonode);

    if (node_eq(&arcnode, &flonode) &&
        bink_bsy_create(&flonode, NOWAIT) == ERROR) {
        debug(1, "%s busy, skipping", znfp1(&flonode));
        if (file)
            fclose(file);
/*	bink_bsy_delete(&arcnode); */
        return OK;              /* This is o.k. */
    }

    /* Do the various pack functions */
    if (pack->arc && pack->arc->pack == PACK_ARC) {
        if ((desc->flav != FLAV_CRASH && pack->arc->prog)) {
            if (pack->pack == PACK_ROUTE)
                fglog("archiving packet (%ldb) for %s via %s arc (%s)",
                      check_size(name),
                      znfp1(&desc->to), znfp2(&arcnode), pack->arc->name);
            else if (pack->pack == PACK_FLO)
                fglog("archiving packet (%ldb) for %s via %s flo (%s)",
                      check_size(name),
                      znfp1(&desc->to), znfp2(&flonode), pack->arc->name);
            else
                fglog("archiving packet (%ldb) for %s (%s)",
                      check_size(name), znfp1(&desc->to), pack->arc->name);

            ret = do_arcmail(name, &arcnode, &flonode, desc,
                             file, pack->arc->prog, NULL, pack->maxarc);
        } else {
            fglog("packet (%ldb) for %s (noarc)",
                  check_size(name), znfp1(&desc->to));
            ret = do_noarc(name, &desc->to, desc, file, NULL);
        }
    } else {
        if (file)
            fclose(file);
        fglog("packet (%ldb) for %s (%s)",
              check_size(name), znfp1(&desc->to), pack->arc->name);
        ret = do_prog(name, desc, pack);
    }

    /* Delete BSY file */
    bink_bsy_delete(&arcnode);
    if (!node_eq(&arcnode, &flonode))
        bink_bsy_delete(&flonode);

    return ret;
}

/*
 * Pack packet to ArcMail archive in separate directory
 */
int do_dirpack(PktDesc * desc, char *name, FILE * file, Packing * pack)
{
    int ret = OK;
    Node arcnode, flonode;
    char *pktn;

    arcnode = desc->to;
    flonode = desc->to;

    /* Set all -1 values to 0 */
    set_zero(&arcnode);
    set_zero(&flonode);

    /* Create BSY file(s) */
    if (bink_bsy_create(&arcnode, NOWAIT) == ERROR) {
        debug(1, "%s busy, skipping", znfp1(&arcnode));
        if (file)
            fclose(file);
        return OK;              /* This is o.k. */
    }

    /* dirpack */
    if (pack->pack == PACK_DIR && pack->arc && pack->arc->pack == PACK_ARC) {
        if (pack->arc->prog) {
            fglog("archiving packet (%ldb) for %s (%s) in %s",
                  check_size(name), znfp1(&desc->to), pack->arc->name,
                  pack->dir);

            ret = do_arcmail(name, &arcnode, &flonode, desc,
                             file, pack->arc->prog, pack->dir, pack->maxarc);
        }
    }

    /* dirmove */
    if (pack->pack == PACK_MOVE) {
        fglog("moving packet (%ldb) for %s to %s",
              check_size(name), znfp1(&desc->to), pack->dir);

        pktn = packing_pkt_name(pack->dir, name);
        ret = do_noarc(name, &flonode, desc, file, pktn);
    }

    /* Delete BSY file */
    bink_bsy_delete(&arcnode);

    return ret;
}

/*
 * Pack packet
 */
int do_packing(char *name, FILE * fp, Packet * pkt)
{
    PktDesc *desc;
    PktDesc pktdesc;
    Packing *r;
    LNode *p;
    int ret;

    if (pkt_flag) {
        /* Unknown grade/type for .pkt's files */
        pktdesc.from = pkt->from;
        pktdesc.to = pkt->to;
#ifdef DO_NOT_TOSS_NETMAIL
        pktdesc.grade = 'p';
        pktdesc.type = 'n';
#else
        pktdesc.grade = '-';
        pktdesc.type = '-';
#endif                          /* DO_NOT_TOSS_NETMAIL */
        pktdesc.flav = FLAV_NORMAL;

        desc = &pktdesc;
    } else {
        /* Parse description from filename */
        desc = parse_pkt_name(name, &pkt->from, &pkt->to);
        if (desc == NULL)
            TMPS_RETURN(ERROR);
    }

    debug(2, "Packet: from=%s to=%s grade=%c type=%c flav=%c",
          znfp1(&desc->from), znfp2(&desc->to),
          desc->grade, desc->type, desc->flav);

    /*
     * Search for matching packing commands
     */
    for (r = packing_first; r; r = r->next)
        if (desc->type == r->type) {
            for (p = r->nodes.first; p; p = p->next)
                if (node_match(&desc->to, &p->node)) {
                    debug(3, "packing: pack=%c dir=%s arc=%s",
                          r->pack, r->dir ? r->dir : "",
                          (r->arc ? r->arc->name : NULL));
                    ret = r->dir ? do_dirpack(desc, name, fp, r)
                        : do_pack(desc, name, fp, r);
                    TMPS_RETURN(ret);
                }
        }

    TMPS_RETURN(OK);
}

/*
 * Process one packet file
 */
int do_file(char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;

    /* Open packet and read header */
    pkt_file = fopen(pkt_name, R_MODE);
    if (!pkt_file) {
        fglog("$ERROR: can't open packet %s", pkt_name);
        TMPS_RETURN(severe_error);
    }
    if (pkt_get_hdr(pkt_file, &pkt) == ERROR) {
        fglog("ERROR: reading header from %s", pkt_name);
        TMPS_RETURN(severe_error);
    }

    /* Pack it */
    if (do_packing(pkt_name, pkt_file, &pkt) == ERROR) {
        fglog("ERROR: processing %s", pkt_name);
        TMPS_RETURN(severe_error);
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
options: -B --binkley NAME            set Binkley outbound directory\n\
         -F --file-dir NAME           set directory to search for f/a\n\
         -g --grade G                 processing grade\n\
         -I --in-dir NAME             set input packet directory\n\
         -l --lock-file               create lock file while processing\n\
         -m --maxarc KSIZE            set max archive size (KB)\n\
         -O --out-dir NAME            set output packet directory\n\
         -p --packing-file NAME       read packing file\n\
         -P --pkt                     process .pkt's in input directory\n\
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
    int l_flag = FALSE;
    char *I_flag = NULL, *O_flag = NULL, *B_flag = NULL, *p_flag = NULL;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    char *pkt_name;
    char pattern[16];

    int option_index;
    static struct option long_options[] = {
        {"binkley", 1, 0, 'B'}, /* Binkley outbound base dir */
        {"file-dir", 1, 0, 'F'},    /* Dir to search for file attaches */
        {"grade", 1, 0, 'g'},   /* grade */
        {"in-dir", 1, 0, 'I'},  /* Set inbound packets directory */
        {"lock-file", 0, 0, 'l'},   /* Create lock file while processing */
        {"out-dir", 1, 0, 'O'}, /* Set packet directory */
        {"packing-file", 1, 0, 'p'},    /* Set packing file */
        {"maxarc", 1, 0, 'm'},  /* Set max archive size */
        {"pkt", 0, 0, 'P'},     /* Process .pkt's */

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

    while ((c = getopt_long(argc, argv, "B:F:g:I:O:lp:m:Pvhc:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftnpack options *****/
        case 'B':
            B_flag = optarg;
            break;
        case 'F':
            BUF_COPY(file_attach_dir, optarg);
            break;
        case 'g':
            g_flag = *optarg;
            break;
        case 'I':
            I_flag = optarg;
            break;
        case 'O':
            O_flag = optarg;
            break;
        case 'l':
            l_flag = TRUE;
            break;
        case 'p':
            p_flag = optarg;
            break;
        case 'm':
            maxarc = atol(optarg) * 1024L;
            break;
        case 'P':
            pkt_flag = TRUE;
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
    if (B_flag)
        cf_s_btbasedir(B_flag);
    if (a_flag)
        cf_set_addr(a_flag);
    if (u_flag)
        cf_set_uplink(u_flag);

    cf_debug();

    /*
     * Process optional config statements
     */
    if (maxarc == 0 && (p = cf_get_string("MaxArc", TRUE))) {
        maxarc = atol(p) * 1024L;
    }
    if ((p = cf_get_string("BundleDisp", TRUE))) {
        bundle_disp = atoi(p);
        if ((2 < bundle_disp) || (0 > bundle_disp)) {
            fglog("ERROR: %s: invalid BundleDisp value", CONFIG);
            exit_free();
            return EXIT_ERROR;
        }
    } else
        bundle_disp = 0;

    strict = (cf_get_string("FTNStrictPktCheck", TRUE) != NULL);
    /*
     * Process local options
     */
    BUF_EXPAND(in_dir, I_flag ? I_flag : cf_p_toss_route());
    BUF_EXPAND(out_dir, O_flag ? O_flag : cf_p_toss_route());

    packing_init(p_flag ? p_flag : cf_p_packing());
    passwd_init();

    /* Install signal/exit handlers */
    signal(SIGHUP, prog_signal);
    signal(SIGINT, prog_signal);
    signal(SIGQUIT, prog_signal);

    ret = EXIT_OK;

    if (optind >= argc) {
        if (pkt_flag) {
            BUF_COPY(pattern, "*.pkt");
        } else {
            BUF_COPY(pattern, "????????.pkt");
            if (g_flag)
                pattern[0] = g_flag;
        }

        /* Lock file */
        if (l_flag)
            if (lock_program(PROGRAM, NOWAIT) == ERROR) {
                exit_free();
                return EXIT_BUSY;
            }

        /* Process packet files in directory */
        dir_sortmode(DIR_SORTMTIME);
        if (dir_open(in_dir, pattern, TRUE) == ERROR) {
            fglog("$ERROR: can't open directory %s", in_dir);
            ret = EX_OSERR;
        } else {
            for (pkt_name = dir_get(TRUE); pkt_name; pkt_name = dir_get(FALSE)) {
                if (do_file(pkt_name) == ERROR) {
                    ret = EXIT_ERROR;
                    break;
                }
                tmps_freeall();
            }
            dir_close();
        }

        /* Lock file */
        if (l_flag)
            unlock_program(PROGRAM);
    } else {
        /* Lock file */
        if (l_flag)
            if (lock_program(PROGRAM, NOWAIT) == ERROR) {
                exit_free();
                return EXIT_BUSY;
            }

        /* Process packet files on command line */
        for (; optind < argc; optind++) {
            if (do_file(argv[optind]) == ERROR) {
                ret = EXIT_ERROR;
                break;
            }
            tmps_freeall();
        }

        /* Lock file */
        if (l_flag)
            unlock_program(PROGRAM);
    }

    exit_free();
    return ret;
}
