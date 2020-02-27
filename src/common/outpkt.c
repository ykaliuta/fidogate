/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Output packet handling for ftntoss and ftroute.
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
 * struct for maintaining output packet files
 */
typedef struct st_outpkt {
    char *tmpname;              /* Temporary name */
    char *outname;              /* Final name */
    Node from;                  /* From address */
    Node to;                    /* To   address */
    char flav;                  /* Flavor: N, H, C, D */
    char type;                  /* N=NetMail, E=EchoMail */
    char grade;                 /* N=Normal, ... */
    char bad;                   /* TRUE=bad packet, FALSE=normal packet */
    FILE *fp;                   /* FILE pointer, if opened */
    long n;                     /* Number of messages written to packet */

    struct st_outpkt *next;
} OutPkt;

static OutPkt *outpkt_first = NULL;
static OutPkt *outpkt_last = NULL;

static int outpkt_maxopen = MAXOPENFILES;   /* Max # */
static int outpkt_nopen = 0;    /* Currently opened files */

/*
 * Prototypes
 */
static OutPkt *outpkt_new(Node *, Node *);
static void outpkt_names(OutPkt *, int, int, int, int);
static FILE *outpkt_fopen(char *, char *);
static int outpkt_close_ln(void);

/*
 * Allocate new OutPkt and add to linked list
 */
OutPkt *outpkt_new(Node * from, Node * to)
{
    OutPkt *p = (OutPkt *) xmalloc(sizeof(OutPkt));

    /* Initialize */
    p->tmpname = NULL;
    p->outname = NULL;
    p->from = *from;
    p->to = *to;
    p->flav = 0;
    p->type = 0;
    p->grade = 0;
    p->bad = FALSE;
    p->fp = NULL;
    p->n = 1;
    p->next = NULL;

    /* Put into linked list */
    if (outpkt_first)
        outpkt_last->next = p;
    else
        outpkt_first = p;
    outpkt_last = p;

    return p;
}

/*
 * Write name of output packet file to buf
 */
char *outpkt_outputname(char *buf,
                        char *dir, int grade, int type, int flav,
                        long n, char *ext)
{
    str_printf(buf, BUFFERSIZE, "%s/%c%c%c%05lx.%s",
               dir, grade, type, flav, n, ext);
    return buf;
}

/*
 * Get output packet sequence number
 */
long outpkt_sequencer(void)
{
    return sequencer(cf_p_seq_toss()) & 0x000fffff;
}

/*
 * Set OutPkt tmpname and outname
 */
void outpkt_names(OutPkt * p, int grade, int type, int flav, int bad)
{
    long n = outpkt_sequencer();

    /* Set fields */
    p->flav = flav;
    p->type = type;
    p->grade = grade;
    p->bad = bad;

    /* Names */
    outpkt_outputname(buffer,
                      bad ? pkt_get_baddir() : pkt_get_outdir(),
                      grade, type, flav, n, "pkt");
    p->outname = strsave(buffer);
    outpkt_outputname(buffer,
                      bad ? pkt_get_baddir() : pkt_get_outdir(),
                      grade, type, flav, n, "tmp");
    p->tmpname = strsave(buffer);
}

/*
 * Set number open max open packet files
 */
void outpkt_set_maxopen(int n)
{
    outpkt_maxopen = n;
}

/*
 * Close packet file with least number of messages
 */
static int outpkt_close_ln(void)
{
    OutPkt *p, *pmin;
    long min;
    int ret = OK;

    pmin = NULL;
    min = -1;

    /* Search for least n */
    for (p = outpkt_first; p; p = p->next)
        if (p->fp)
            if (min == -1 || min >= p->n) {
                pmin = p;
                min = p->n;
            }

    /* Close it */
    if (pmin) {
        debug(3, "Close %s", pmin->tmpname);
        if (fclose(pmin->fp) == ERROR)
            ret = ERROR;
        outpkt_nopen--;
        pmin->fp = NULL;
    } else
        ret = ERROR;

    return ret;
}

/*
 * Open file for output packet. If the max number of open files
 * exceeded, this function closes an old packet file. NULL is
 * returned when this doesn't suceed, either.
 */
static FILE *outpkt_fopen(char *name, char *mode)
{
    FILE *fp;

    while (outpkt_nopen >= outpkt_maxopen)
        /*
         * Must close another packet file first
         */
        if (outpkt_close_ln() == ERROR)
            return NULL;

    if ((fp = fopen(name, mode))) {
        debug(3, "Open %s, mode %s", name, mode);
        outpkt_nopen++;
        return fp;
    } else
        debug(3, "Open %s failed", name);

    return NULL;
}

/*
 * Return FILE for output packet
 */
FILE *outpkt_open(Node * from, Node * to,
                  int grade, int type, int flav, int bad)
{
    Packet pkt;
    OutPkt *p;
    Passwd *pwd;

    /*
     * Search for existing packet
     */
    for (p = outpkt_first; p; p = p->next)
        if (node_eq(from, &p->from) && node_eq(to, &p->to) &&
            flav == p->flav && type == p->type && grade == p->grade
            && bad == p->bad) {
            p->n++;
            /* If file exists, but isn't currently opened, reopen it */
            if (!p->fp)
                p->fp = outpkt_fopen(p->tmpname, A_MODE);
            return p->fp;
        }

    /*
     * Doesn't exist, create a new one
     */
    p = outpkt_new(from, to);
    outpkt_names(p, grade, type, flav, bad);

    /* Open file and write header */
    p->fp = outpkt_fopen(p->tmpname, W_MODE);
    if (p->fp == NULL) {
        fglog("$failed to open packet %s", p->tmpname);
        return NULL;
    }

    debug(2, "New packet %s (%s): %s -> %s",
          p->outname, p->tmpname, znfp1(&p->from), znfp2(&p->to));

    pkt.from = p->from;
    pkt.to = p->to;
    pkt.time = time(NULL);
    /* Password */
    pwd = passwd_lookup("packet", to);
    BUF_COPY(pkt.passwd, pwd ? pwd->passwd : "");

    /* Rest is filled in by pkt_put_hdr() */
    if (pkt_put_hdr(p->fp, &pkt) == ERROR) {
        fglog("$Can't write to packet file %s", p->tmpname);
        fclose(p->fp);
        p->fp = NULL;
        return NULL;
    }

    /* o.k., return the FILE */
    return p->fp;
}

/*
 * Close all output packets
 */
void outpkt_close(void)
{
    OutPkt *p, *pn;

    for (p = outpkt_first; p;) {
        pn = p->next;

        /* Reopen, if already closed */
        if (!p->fp)
            p->fp = outpkt_fopen(p->tmpname, A_MODE);

        if (p->fp) {
            /* Terminating 0-word */
            pkt_put_int16(p->fp, 0);
            debug(3, "Close %s", p->tmpname);
            if (fclose(p->fp) == ERROR)
                debug(3, "Close failed");

            outpkt_nopen--;

            /* Rename */
            debug(3, "Rename %s -> %s", p->tmpname, p->outname);
            if (rename(p->tmpname, p->outname) == ERROR)
                debug(3, "Rename failed");
        }

        /* Delete OutPkt entry */
        xfree(p->tmpname);
        xfree(p->outname);
        xfree(p);

        p = pn;
    }

    outpkt_first = NULL;
    outpkt_last = NULL;
    outpkt_nopen = 0;           /* Just to be sure ... */

    return;
}

/*
 * Create packet in OUTPKT with netmail message header
 */
int outpkt_netmail(Message * msg, Textlist * tl, char *program, char *origin,
                   char *tearline)
{
    FILE *fp;

    /* If from node is default, use aka for to zone */
    cf_set_best(msg->node_to.zone, msg->node_to.net, msg->node_to.node);

    if (msg->node_from.zone == 0)
        msg->node_from = cf_n_addr();

    /* Open outpkt packet */
    fp = outpkt_open(&msg->node_from, &msg->node_to, '0', '0', '0', FALSE);
    if (!fp)
        return ERROR;

    /* Write message header */
    pkt_put_msg_hdr(fp, msg, TRUE);
    /* Additional kludges */
    fprintf(fp, "\001MSGID: %s %08lx\r",
            znf1(&msg->node_from), sequencer(cf_p_seq_msgid()));
    /* Write message body */
    tl_print_x(tl, fp, "\r");
    /* Additional kludges */
    if (!tearline)
        fprintf(fp, "\r--- FIDOGATE %s\r", version_global());
    else
        fprintf(fp, "\r--- %s\r", tearline);

    if (NULL == msg->area) {
#ifndef FTS_VIA
        fprintf(fp, "\001Via FIDOGATE/%s %s, %s\r",
                program, znf1(&msg->node_from), date(DATE_VIA, NULL));
#else
        fprintf(fp, "\001Via %s @%s FIDOGATE/%s\r",
                znf1(&msg->node_from), date(DATE_VIA, NULL), program);
#endif                          /* FTS_VIA */
    } else {
        fprintf(fp, " * Origin: %s(%s)\r", origin, znfp1(&msg->node_from));
    }

    /* Terminating 0-byte */
    putc(0, fp);

    outpkt_close();

    return OK;
}
