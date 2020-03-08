/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Functions to read/write packets and messages
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
 * Outbound packets: directory, name, temp. name, file
 */
static char packet_dir[MAXPATH];
static char packet_bad[MAXPATH];
static char packet_name[MAXPATH];
static char packet_tmp[MAXPATH];
static FILE *packet_file = NULL;
static Node packet_node = { -1, -1, -1, -1, "" };

static int packet_bsy = FALSE;

static char *pkt_newname(char *);
static FILE *pkt_open_node(Node *, char *, int);
static FILE *pkt_create(Node *);

/*
 * Set output packet directory
 */
void pkt_outdir(char *dir1, char *dir2)
{
    BUF_EXPAND(packet_dir, dir1);
    if (dir2)
        BUF_APPEND2(packet_dir, "/", dir2);
}

/*
 * Return packet_dir[] name
 */
char *pkt_get_outdir(void)
{
    return packet_dir;
}

/*
 * Set bad packet directory
 */
void pkt_baddir(char *dir1, char *dir2)
{
    BUF_EXPAND(packet_bad, dir1);
    if (dir2)
        BUF_APPEND2(packet_bad, "/", dir2);
}

/*
 * Return packet_bad[] name
 */
char *pkt_get_baddir(void)
{
    return packet_bad;
}

/*
 * Return packet name / temp name
 */
char *pkt_name(void)
{
    return packet_name;
}

char *pkt_tmpname(void)
{
    return packet_tmp;
}

/*
 * Return open status (packet_file != NULL)
 */
int pkt_isopen(void)
{
    return packet_file != NULL;
}

/*
 * Get name for new packet
 */
static char *pkt_newname(char *name)
{
    if (name) {
        BUF_COPY(packet_name, name);
        BUF_COPY(packet_tmp, name);
    } else {
        long n = sequencer(cf_p_seq_pkt());
        str_printf(packet_name, sizeof(packet_name),
                   "%s/%08ld.pkt", packet_dir, n);
        str_printf(packet_tmp, sizeof(packet_tmp),
                   "%s/%08ld.tmp", packet_dir, n);
    }

    return packet_name;
}

/*
 * Open .OUT packet in Binkley outbound
 */
#define MAX_COUNT 50

static FILE *pkt_open_node(Node * node, char *flav, int bsy)
{
    char *out;
    FILE *fp;
    long pos;
    Packet pkt;
    int count;

    /* Name of .OUT file */
    out = bink_find_out(node, flav);
    if (!out)
        return NULL;
    /* Create directory if necessary */
    if (bink_mkdir(node) == ERROR)
        return NULL;

    packet_bsy = bsy;
    if (bsy)
        if (bink_bsy_create(node, WAIT) == ERROR)
            return NULL;

    pkt_newname(out);

    /*
     * Open and lock OUT packet
     */
    count = 0;
    do {
        count++;
        /*
         * Check count and give up, if too many retries
         */
        if (count >= MAX_COUNT) {
            fp = NULL;
            break;
        }

        /* Open OUT file for append, creating empty one if necessary */
        debug(4, "Open OUT file in append mode");
        fp = fopen(out, A_MODE);
        if (fp == NULL) {
            /* If this failed we're out of luck ... */
            fglog("$ERROR: can't open OUT file %s", out);
            break;
        }

        /* Reopen in read/write mode */
        debug(4, "Reopen OUT file in read/write mode");
        fclose(fp);
        fp = fopen(out, RP_MODE);
        if (fp == NULL) {
            /* OUT file deleted in the meantime - retry */
            debug(4, "OUT file deleted, retrying");
            continue;
        }
        chmod(out, PACKET_MODE);

        /* Lock it, waiting for lock to be granted */
        debug(4, "Locking OUT file");
        if (lock_file(fp)) {
            /* Lock error ... */
            fglog("$ERROR: can't lock OUT file %s", out);
            fclose(fp);
            fp = NULL;
            break;
        }

        /* Lock succeeded, but the OUT file may have been deleted */
        if (access(out, F_OK) == -1) {
            debug(4, "OUT file deleted, retrying");
            fclose(fp);
            fp = NULL;
        }
    }
    while (fp == NULL);

    /*
     * fp==NULL is an error in the do ... while loop
     */
    if (fp == NULL) {
        if (bsy)
            bink_bsy_delete(node);
        return NULL;
    }

    /*
     * Test, whether this is a new empty packet, or an already existing one.
     */
    /* Seek to EOF */
    if (fseek(fp, 0L, SEEK_END) == -1) {
        /* fseek() error ... */
        fglog("$ERROR: fseek EOF OUT file %s failed", out);
        if (bsy)
            bink_bsy_delete(node);
        fclose(fp);
        return NULL;
    }
    if ((pos = ftell(fp)) == -1L) {
        /* ftell() error ... */
        fglog("$ERROR: ftell OUT file %s failed", out);
        if (bsy)
            bink_bsy_delete(node);
        fclose(fp);
        return NULL;
    }

    if (pos == 0L) {
        Passwd *pwd;

        /*
         * This is a new packet file
         */
        debug(4, "%s is a new packet, writing header", out);

        cf_set_best(node->zone, node->net, node->node);

        pkt.from = cf_n_addr();
        pkt.to = *node;
        pkt.time = time(NULL);
        /* Password */
        pwd = passwd_lookup("packet", node);
        BUF_COPY(pkt.passwd, pwd ? pwd->passwd : "");

        /* Rest is filled in by pkt_put_hdr() */
        if (pkt_put_hdr(fp, &pkt) == ERROR) {
            fglog("$ERROR: can't write to packet file %s", out);
            if (bsy)
                bink_bsy_delete(node);
            fclose(fp);
            return NULL;
        }
    } else {
        /*
         * This is an already existing packet file: seek to end of file
         * - 2, there must be a terminating 0-word. Start writing new data
         * at the EOF - 2 position.
         */
        debug(4, "%s already exists, seek to EOF-2", out);

        if (fseek(fp, -2L, SEEK_END) == -1) {
            /* fseek() error ... */
            fglog("$ERROR: fseek EOF-2 OUT file %s failed", out);
            if (bsy)
                bink_bsy_delete(node);
            fclose(fp);
            return NULL;
        }
        if (pkt_get_int16(fp) != 0L) {
            fglog("$ERROR: malformed packet %s, no terminating 0-word", out);
            if (bsy)
                bink_bsy_delete(node);
            fclose(fp);
            return NULL;
        }
        if (fseek(fp, -2L, SEEK_END) == -1) {
            /* fseek() error ... */
            fglog("$ERROR: fseek EOF-2 OUT file %s failed", out);
            if (bsy)
                bink_bsy_delete(node);
            fclose(fp);
            return NULL;
        }
    }

    packet_file = fp;
    packet_node = *node;

    return fp;
}

/*
 * Create new packet
 */
static FILE *pkt_create(Node * to)
{
    Packet pkt;
    Passwd *pwd;

    if ((packet_file = fopen(packet_tmp, W_MODE)) == NULL) {
        fglog("$ERROR: pkt_open(): can't create packet %s", packet_tmp);
        return NULL;
    }

    /*
     * Change mode to PACKET_MODE
     */
    chmod(packet_tmp, PACKET_MODE);

    debug(4, "New packet file %s (tmp %s)", packet_name, packet_tmp);

    cf_set_best(to->zone, to->net, to->node);

    /*
     * Write packet header
     */
    pkt.from = cf_n_addr();
    pkt.to = *to;
    pkt.time = time(NULL);
    /* Password */
    pwd = passwd_lookup("packet", to);
    BUF_COPY(pkt.passwd, pwd ? pwd->passwd : "");

    /* Rest is filled in by pkt_put_hdr() */
    if (pkt_put_hdr(packet_file, &pkt) == ERROR) {
        fglog("$ERROR: can't write to packet file %s", packet_tmp);
        return NULL;
    }

    return packet_file;
}

/*
 * Create new packet file
 */
FILE *pkt_open(char *name, Node * node, char *flav, int bsy)
{
    if (name && !name[0])
        name = NULL;

    if (node && !name)
        /*
         * Open packet in Binkley outbound
         */
        return pkt_open_node(node, flav, bsy);
    else {
        /*
         * Open packet in FIDOGATE outbound directory or named packet
         */
        pkt_newname(name);
        return pkt_create(node ? node : cf_uplink());
    }

    /**NOT REACHED**/
    return NULL;
}

/*
 * Write end of packet to packet file, close and rename it.
 */
int pkt_close(void)
{
    int ret = OK;

    if (packet_file) {
        /* End of packet */
        pkt_put_int16(packet_file, 0);
        ret = fclose(packet_file);

        packet_file = NULL;

        if (packet_node.zone != -1) {
            if (packet_bsy)
                bink_bsy_delete(&packet_node);
            packet_bsy = FALSE;
            packet_node.zone = -1;
        }

        /* Rename .tmp -> .pkt, if not the same name */
        if (strcmp(packet_tmp, packet_name))
            if (rename(packet_tmp, packet_name) == ERROR) {
                fglog("$ERROR: can't rename %s to %s", packet_tmp, packet_name);
                ret = ERROR;
            }
    }

    return ret;
}

/*
 * Read NUL-terminated string from packet file.
 * If the buffer not big enough put nbytes - 1 to the buffer, make it
 * NUL-terminated and read the string (move the file pointer) up to
 * its NUL-terminator.
 *
 * Returns the number of read bytes (can be more than @nbytes).
 */
size_t pkt_get_string(FILE * fp, char *buf, size_t nbytes)
{
    int c, i;

    for (i = 0; TRUE; i++) {
        c = getc(fp);

        if (c == 0 || c == EOF)
            break;

        if (i < nbytes - 1) {
            buf[i] = c;
            continue;
        }

        if (i == nbytes - 1)
            buf[i] = '\0';
    }

    if (i < nbytes - 1)
        buf[i] = '\0';

    i += (c != EOF);            /* EOF means the byte was not actually read */
    return i;
}

/*
 * Return date of message in UNIX format (secs after the epoche)
 *
 * Understood formats: see FTS-0001 (actually using getdate.y parser)
 */
time_t pkt_get_date(FILE * fp)
{
    char buf[MSG_MAXDATE + 1];
    size_t len;

    /*
       date has a fixed length
       constant to rename,
       before date was treated as NULL-terminated string
     */
    len = fread(buf, sizeof(char), MSG_MAXDATE, fp);
    if (len < MSG_MAXDATE) {
        fglog("ERROR: reading date return %zd, %d expected", len, MSG_MAXDATE);
        return ERROR;
    }
    buf[MSG_MAXDATE] = '\0';

    // first date format - FTS         `20 Dec 02  14:26:03'
    // second date dormat - nostandart `Thu 12 Dec 02 18:19'
    if (!((buf[2] == ' ' && buf[6] == ' ' && buf[9] == ' ' && buf[10] == ' ' &&
           buf[13] == ':' && buf[16] == ':') ||
          (buf[3] == ' ' && buf[6] == ' ' && buf[10] == ' ' && buf[13] == ' ' &&
           buf[16] == ':'))) {
        fglog("WARNING: wrong or corrupted format message date header \'%s\'",
              buf);
    }

    return parsedate(buf, NULL);
}

#define READ_MSG_FIELD(fp, s, f, strict) do {					\
    size_t rc;								\
    size_t limit = f ## _limit;						\
    rc = pkt_get_string(fp, s->f, sizeof(s->f));			\
    if (rc > limit) {							\
        fglog("ERROR: %s is longer than %lu", #f, limit);		\
	if (strict)							\
	    return ERROR;						\
    }									\
    } while (0);

/*
 * Read message header from packet file
 */
int pkt_get_msg_hdr(FILE * fp, Message * msg, bool strict)
{
    size_t name_to_limit = MSG_MAXNAME;
    size_t name_from_limit = MSG_MAXNAME;
    size_t subject_limit = MSG_MAXSUBJ;

    msg->node_from.node = pkt_get_int16(fp);
    msg->node_to.node = pkt_get_int16(fp);
    msg->node_from.net = pkt_get_int16(fp);
    msg->node_to.net = pkt_get_int16(fp);
    msg->node_orig = msg->node_from;
    msg->attr = pkt_get_int16(fp);
    msg->cost = pkt_get_int16(fp);
    msg->date = pkt_get_date(fp);

    READ_MSG_FIELD(fp, msg, name_to, strict);
    READ_MSG_FIELD(fp, msg, name_from, strict);
    READ_MSG_FIELD(fp, msg, subject, strict);

    msg->area = NULL;

    if (verbose >= 6)
        pkt_debug_msg_hdr(stderr, msg, "Reading ");

    return ferror(fp);
}

#undef READ_MSG_FIELD

/*
 * Debug output of message header
 */
void pkt_debug_msg_hdr(FILE * out, Message * msg, char *txt)
{
    fprintf(out, "%sFTN message header:\n", txt);
    fprintf(out, "    From: %-36s @ %s\n",
            msg->name_from, nf1(&msg->node_from));
    fprintf(out, "    To  : %-36s @ %s\n", msg->name_to, nf1(&msg->node_to));
    fprintf(out, "    Subj: %s\n", msg->subject);
    fprintf(out, "    Date: %s\n",
            msg->date != -1 ? date(NULL, &msg->date) : "LOCAL");
    fprintf(out, "    Attr: %04x\n", msg->attr);
}

/*
 * Write string to packet in null-terminated format.
 */
void pkt_put_string(FILE * fp, char *s)
{
    fputs(s, fp);
    putc(0, fp);

    return;
}

/*
 * Write line to packet, replacing \n with \r
 */
void pkt_put_line(FILE * fp, char *s)
{
    for (; *s; s++) {
        if (*s == '\n')
	    putc('\r', fp);
	else
	    putc(*s, fp);
    }

    return;
}

/*
 * Write 16-bit integer in 80x86 format, i.e. low byte first,
 * then high byte. Machine independent function.
 */
void pkt_put_int16(FILE * fp, int value)
{
    putc(value & 0xff, fp);
    putc((value >> 8) & 0xff, fp);

    return;
}

/*
 * Write date/time in FTS-0001 format
 */
void pkt_put_date(FILE * pkt, time_t t, char *tz)
{
    static time_t last = -1;

    if (t == -1) {
        /* No valid time, use local time */
        debug(7, "using local time");
        t = time(NULL);
        /* Kludge to avoid the same date/time */
        if (t == last)
            t += 2;
        last = t;
    }

    /* Date according to FTS-0001 */
    pkt_put_string(pkt, date_tz(DATE_FTS_0001, &t, tz));

    return;
}

/*
 * name_* / subj can come long from rfc
 * Make sure they are not breaking FTS
 */
static void pkt_msg_sanitize_strings(Message *msg)
{
    int i;
    size_t len;
    struct field {
	char *str;
	size_t limit;
	char *name;
    } fields[] = {
	{
	    .str = msg->name_to,
	    .limit = MSG_MAXNAME,
	    .name = "`To`",
	},
	{
	    .str = msg->name_from,
	    .limit = MSG_MAXNAME,
	    .name = "`From`",
	},
	{
	    .str = msg->subject,
	    .limit = MSG_MAXSUBJ,
	    .name = "`Subject`",
	}
    };

    for (i = 0; i < sizeof(fields)/sizeof(fields[0]); i++) {
	len = strlen(fields[i].str);
	if (len < fields[i].limit)
	    continue;

	fglog("WARNING: truncating %s from %zu to %zu",
	      fields[i].name, len, fields[i].limit - 1);

	fields[i].str[fields[i].limit - 1] = '\0';
    }
}

/*
 * Write message header to packet.
 */
int pkt_put_msg_hdr(FILE * pkt, Message * msg, int kludge_flag)
    /* kludge_flag --- TRUE: write AREA/^AINTL,^AFMPT,^ATOPT */
{
    if (verbose >= 6)
        pkt_debug_msg_hdr(stderr, msg, "Writing ");

    /*
     * Write message header
     */
    pkt_put_int16(pkt, MSG_TYPE);
    pkt_put_int16(pkt, msg->node_from.node);
    pkt_put_int16(pkt, msg->node_to.node);
    pkt_put_int16(pkt, msg->node_from.net);
    pkt_put_int16(pkt, msg->node_to.net);
    pkt_put_int16(pkt, msg->attr);
    pkt_put_int16(pkt, msg->cost);

    pkt_put_date(pkt, msg->date, msg->tz);

    pkt_msg_sanitize_strings(msg);

    pkt_put_string(pkt, msg->name_to);
    pkt_put_string(pkt, msg->name_from);
    pkt_put_string(pkt, msg->subject);

    if (!kludge_flag)
        goto pkt_put_msg_hdr_ret;

    /*
     * Write area tag / zone, point adressing kludges
     */
    if (msg->area)
        fprintf(pkt, "AREA:%s\r", msg->area);
    else {
        Node tmpf, tmpt;

        tmpf = msg->node_from;
        tmpf.point = 0;
        tmpf.domain[0] = 0;
        tmpt = msg->node_to;
        tmpt.point = 0;
        tmpt.domain[0] = 0;
        fprintf(pkt, "\001INTL %s %s\r", znf1(&tmpt), znf2(&tmpf));

        if (msg->node_from.point > 0)
            fprintf(pkt, "\001FMPT %d\r", msg->node_from.point);
        if (msg->node_to.point > 0)
            fprintf(pkt, "\001TOPT %d\r", msg->node_to.point);
    }

 pkt_put_msg_hdr_ret:

    return ferror(pkt);
}

/*
 * Read 16-bit integer in 80x86 format, i.e. low byte first,
 * then high byte. Machine independent function.
 */
long pkt_get_int16(FILE * fp)
{
    int c;
    unsigned val;

    if ((c = getc(fp)) == EOF)
        return ERROR;
    val = c;
    if ((c = getc(fp)) == EOF)
        return ERROR;
    val |= c << 8;

    return val;
}

/*
 * Read n bytes from file stream
 */
int pkt_get_nbytes(FILE * fp, char *buf, int n)
{
    int c;

    while (n--) {
        if ((c = getc(fp)) == EOF)
            return ERROR;
        *buf++ = c;
    }

    return ferror(fp);
}

/*
 * Read packet header from file
 */
int pkt_get_hdr(FILE * fp, Packet * pkt)
{
    long val;
    struct tm t;
    int ozone, dzone;
    int cw, swap;
    int retVal;
    char xpkt[4];
    struct tm *tm;
    TIMEINFO ti;
    int auxnet;

    retVal = OK;
    GetTimeInfo(&ti);
    tm = localtime(&ti.time);
    node_clear(&pkt->from);
    node_clear(&pkt->to);
    pkt->time = -1;
    pkt->baud = 0;
    pkt->version = 0;
    pkt->product_l = 0;
    pkt->product_h = 0;
    pkt->rev_min = 0;
    pkt->rev_maj = 0;
    pkt->passwd[0] = 0;
    pkt->capword = 0;

    /* Set zone to default, i.e. use the zone from your FIRST aka
     * specified in fidogate.conf */
    pkt->from.zone = pkt->to.zone = cf_defzone();

    /* Orig node */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    pkt->from.node = val;
    /* Dest node */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    pkt->to.node = val;
    /* Year */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (val == 0 || val < 1900 || val > 2099)
#ifdef FIX_BAD_PKT_YEAR
        ;
#else
        retVal = ERROR;
#endif
    else
        t.tm_year = val - 1900;
    /* Month */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (val > 11)
        retVal = ERROR;
    t.tm_mon = val;
    /* Day */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (val > 31)
        retVal = ERROR;
    if (val == 0)
        val = tm->tm_mday;
    t.tm_mday = val;
    /* Hour */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (val > 23)
        retVal = ERROR;
    t.tm_hour = val;
    /* Minute */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (val > 59)
        retVal = ERROR;
    t.tm_min = val;
    /* Second */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (val > 59)
        retVal = ERROR;
    t.tm_sec = val;
    t.tm_wday = -1;
    t.tm_yday = -1;
    t.tm_isdst = -1;
    if (retVal == OK)
        pkt->time = mktime(&t);
    /* Baud */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    pkt->baud = val;
    /* Version --- MUST BE PKT_VERSION (2) */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (val != PKT_VERSION)
        retVal = ERROR;
    pkt->version = val;
    /* Orig net */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    pkt->from.net = val;
    /* Dest net */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    pkt->to.net = val;
    /* Prod code lo */
    if ((val = getc(fp)) == ERROR)
        return ERROR;
    pkt->product_l = val;
    /* Revision major */
    if ((val = getc(fp)) == ERROR)
        return ERROR;
    pkt->rev_maj = val;
    /* Password */
    if (pkt_get_nbytes(fp, pkt->passwd, PKT_MAXPASSWD) == ERROR)
        return ERROR;
    pkt->passwd[PKT_MAXPASSWD] = 0;
    /* Orig zone (FTS-0001 optional, QMail) */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    ozone = val;
    if (ozone)
        pkt->from.zone = ozone;
    /* Dest zone (FTS-0001 optional, QMail) */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    dzone = val;
    if (dzone)
        pkt->to.zone = dzone;
    /* Spare (auxNet in FSC-0048) */
    if ((auxnet = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    /* Cap word byte-swapped copy */
    if ((val = getc(fp)) == ERROR)
        return ERROR;
    swap = val << 8;
    if ((val = getc(fp)) == ERROR)
        return ERROR;
    swap |= val;
    /* Prod code hi */
    if ((val = getc(fp)) == ERROR)
        return ERROR;
    pkt->product_h = val;
    /* Revision minor */
    if ((val = getc(fp)) == ERROR)
        return ERROR;
    pkt->rev_min = val;
    /* Cap word */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    cw = val;
    if (cw && cw == swap)       /* 2+ packet according to FSC-0039 */
        debug(9, "Packet: type 2+");
    else
        cw = 0;
    pkt->capword = cw;
    /* Orig zone (FSC-0039) */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (cw && val) {
        pkt->from.zone = val;
        if (ozone != val)
            debug(9, "Packet: different zones %d (FTS-0001) / %ld (FSC-0039)",
                  ozone, val);
    }
    /* Dest zone (FSC-0039) */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (cw && val) {
        pkt->to.zone = val;
        if (dzone != val)
            debug(9, "Packet: different zones %d (FTS-0001) / %ld (FSC-0039)",
                  dzone, val);
    }
    /* Orig point (FSC-0039) */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (cw)
        pkt->from.point = val;
    /* Dest point (FSC-0039) */
    if ((val = pkt_get_int16(fp)) == ERROR)
        return ERROR;
    if (cw)
        pkt->to.point = val;
    /* Prod specific data */
    if (pkt_get_nbytes(fp, xpkt, 4) == ERROR)
        return ERROR;

    if (verbose >= 3)
        pkt_debug_hdr(stderr, pkt, "Reading ");

    /* FSC-0048 point packet check */
    if (pkt->from.net == 0xFFFF) {
        if (pkt->from.point != 0)
            pkt->from.net = auxnet;
        else
            /* hpt does it */
            pkt->from.net = pkt->to.net;
        debug(9, "Packet: FSC-0048 orig net: %d -> %d", 0xFFFF, pkt->from.net);
    }

    if (ferror(fp) == ERROR)
        return ERROR;
    return retVal;
}

/*
 * Debug output of packet header
 */
void pkt_debug_hdr(FILE * out, Packet * pkt, char *txt)
{
    fprintf(out, "%sFTN packet header:\n", txt);
    fprintf(out, "    From: %s\n", znfp1(&pkt->from));
    fprintf(out, "    To  : %s\n", znfp2(&pkt->to));
    fprintf(out, "    Date: %s\n", date(NULL, &pkt->time));
    fprintf(out, "    Baud: %d\n", pkt->baud);
    fprintf(out, "    Prod: %02x %02x\n", pkt->product_h, pkt->product_l);
    fprintf(out, "    Rev : %d.%d\n", pkt->rev_maj, pkt->rev_min);
    fprintf(out, "    Pass: \"%s\"\n", pkt->passwd);
    fprintf(out, "    Capw: %04x\n", pkt->capword & 0xffff);
}

/*
 * Write string to packet, padded with 0 bytes to length n
 */
int pkt_put_string_padded(FILE * fp, char *s, int n)
{
    int i;
    for (i = 0; *s && i < n; s++, i++)
        putc(*s, fp);
    for (; i < n; i++)
        putc(0, fp);

    return ferror(fp);
}

/*
 * Fills non-user information in pkt header
 */
void pkt_fill_hdr(Packet * pkt)
{
    /*
     * Fill rest of Packet structure
     */
    pkt->baud = 0;
    pkt->version = PKT_VERSION;
    pkt->product_l = PRODUCT_CODE;
    pkt->product_h = 0;
    pkt->rev_min = version_minor();
    pkt->rev_maj = version_major();
    pkt->capword = 0x0001;      /* Designates packet type 2+ */
    memcpy(pkt->psd, "XPKT", sizeof(pkt->psd)); /* Like SQUISH */
}

int pkt_put_hdr_raw(FILE * fp, Packet * pkt)
{
    struct tm *tm;
    int swap;                   /* Byte swapped capability word */
    int i;

    swap = (pkt->capword << 8) & 0xff00;
    swap |= (pkt->capword >> 8) & 0x00ff;
    tm = localtime(&pkt->time);

    pkt_put_int16(fp, pkt->from.node);
    pkt_put_int16(fp, pkt->to.node);
    pkt_put_int16(fp, tm->tm_year + 1900);
    pkt_put_int16(fp, tm->tm_mon);
    pkt_put_int16(fp, tm->tm_mday);
    pkt_put_int16(fp, tm->tm_hour);
    pkt_put_int16(fp, tm->tm_min);
    pkt_put_int16(fp, tm->tm_sec);
    pkt_put_int16(fp, pkt->baud);
    pkt_put_int16(fp, pkt->version);
    pkt_put_int16(fp, pkt->from.net);
    pkt_put_int16(fp, pkt->to.net);
    putc(pkt->product_l, fp);
    putc(pkt->rev_maj, fp);
    pkt_put_string_padded(fp, pkt->passwd, PKT_MAXPASSWD);
    pkt_put_int16(fp, pkt->from.zone);
    pkt_put_int16(fp, pkt->to.zone);
    pkt_put_int16(fp, 0 /* Spare */ );
    pkt_put_int16(fp, swap);
    putc(pkt->product_h, fp);
    putc(pkt->rev_min, fp);
    pkt_put_int16(fp, pkt->capword);
    pkt_put_int16(fp, pkt->from.zone);
    pkt_put_int16(fp, pkt->to.zone);
    pkt_put_int16(fp, pkt->from.point);
    pkt_put_int16(fp, pkt->to.point);

    for (i = 0; i < sizeof(pkt->psd); i++)
        putc(pkt->psd[i], fp);

    return ferror(fp);
}

/*
 * Write packet header to file. This function always writes a 2+
 * (FSC-0039 / FSC-0048) header.
 */
int pkt_put_hdr(FILE * fp, Packet * pkt)
{
    pkt_fill_hdr(pkt);

    if (verbose >= 3)
        pkt_debug_hdr(stderr, pkt, "Writing ");

    return pkt_put_hdr_raw(fp, pkt);
}
