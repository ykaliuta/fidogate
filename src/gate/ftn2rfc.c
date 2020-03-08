/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Convert FTN mail packets to RFC mail and news batches
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

#include <pwd.h>
#include <sys/wait.h>

#define PROGRAM 	"ftn2rfc"
#define CONFIG		DEFAULT_CONFIG_GATE

/* Prototypes */
char *get_from(Textlist *, Textlist *);
char *get_reply_to(Textlist *);
char *get_to(Textlist *);
char *get_cc(Textlist *);
char *get_bcc(Textlist *);
char *get_subject(Textlist *);
Area *news_msg(char *, Node *);
int msg_format_buffer(char *, Textlist *);
static int gate_rfc_kludge = FALSE; /* GateRfcKludge          */
int unpack(FILE *, Packet *);
int unpack_file(char *);

void short_usage(void);
void usage(void);

/* Command line options */
int n_flag = FALSE;
int t_flag = FALSE;

char in_dir[MAXPATH];

/* X-FTN flags
 *    f    X-FTN-From
 *    t    X-FTN-To
 *    T    X-FTN-Tearline
 *    O    X-FTN-Origin
 *    V    X-FTN-Via, X-FTN-Recd
 *    D    X-FTN-Domain
 *    S    X-FTN-Seen-By
 *    P    X-FTN-Path
 *    K    X-FTN-Kludge
 *    F    X-FTN-Flag
 */
int x_ftn_f = FALSE;
int x_ftn_t = FALSE;
int x_ftn_T = FALSE;
int x_ftn_O = FALSE;
int x_ftn_V = FALSE;
int x_ftn_D = FALSE;
int x_ftn_S = FALSE;
int x_ftn_P = FALSE;
int x_ftn_K = FALSE;
int x_ftn_F = FALSE;

/* TrackerMail */
static char *tracker_mail_to = NULL;

/* MSGID handling */
static int no_unknown_msgid_zones = FALSE;
static int no_messages_without_msgid = FALSE;

/* Use * Origin line for Organization header */
static int use_origin_for_organization = FALSE;

/* Don't bounce message from FTN address not listed in HOSTS */
static int ignore_hosts = FALSE;

/* Newsgroup name for unknown FTN areas (NULL = don't gate) */
static char *ftn_junk_group = NULL;
static Area *ftn_junk_group_keys = NULL;

/* Address for Errors-To header (NULL = none) */
static char *errors_to = NULL;

/* Do not allow RFC addresses (chars !, &, @) in FTN to field */
static int no_address_in_to_field = FALSE;

/* Character conversion */
static int netmail_8bit = FALSE;
static int netmail_qp = FALSE;
static int netmail_b64 = FALSE;
static int netmail_headers_plain = FALSE;

/* Use FTN to address (cvt to Internet address) for mail_to */
static int use_ftn_to_address = FALSE;

/* Kill split messages with ^ASPLIT kludge */
static int kill_split = FALSE;

/* Write single news article files, not one big news batch */
static int single_articles = FALSE;

/* Charset stuff */
static char *default_charset_def = NULL;
static char *default_charset_out = NULL;
static char *netmail_charset_def = NULL;
static char *netmail_charset_out = NULL;

/* String to add to news Path header */
static char *news_path_tail = "fidogate!not-for-mail";
static short int ignore_chrs = FALSE;
static short int ignore_soft_cr = FALSE;
static short int ignore_mime_type = TRUE;
static bool strict;

/*
 * Get header for
 *   - From/Reply-To/UUCPFROM
 *   - To
 *   - Cc
 *   - Bcc
 */
char *get_from(Textlist * rfc, Textlist * kludge)
{
    char *p;
#ifndef IGNORE_FROM_IF_REPLY
    Node dummy;
    char *q;
#endif                          /* IGNORE_FROM_IF_REPLY */

#ifdef IGNORE_FROM_IF_REPLY
    if ((p = rfcheader_get(rfc, "REPLYTO"))) ;
    else if ((p = rfcheader_get(rfc, "REPLYADDR"))) ;
    else
#endif                          /* IGNORE_FROM_IF_REPLY */
        p = rfcheader_get(rfc, "From");
#ifndef IGNORE_FROM_IF_REPLY
    if (!p) {
        if ((p = kludge_get(kludge, "REPLYADDR", NULL)))
            if ((q = strchr(p, '%')) || (q = strchr(p, '@')))
                if (znfp_parse_partial(q + 1, &dummy) == OK)
                    return NULL;
    }
    if (!p)
        p = rfcheader_get(rfc, "UUCPFROM");
#endif                          /* IGNORE_FROM_IF_REPLY */
    return p;
}

char *get_reply_to(Textlist * tl)
{
    return rfcheader_get(tl, "Reply-To");
}

char *get_to(Textlist * tl)
{
    return rfcheader_get(tl, "To");
}

char *get_cc(Textlist * tl)
{
    return rfcheader_get(tl, "Cc");
}

char *get_bcc(Textlist * tl)
{
    return rfcheader_get(tl, "Bcc");
}

char *get_subject(Textlist * tl)
{
    return rfcheader_get(tl, "Subject");
}

/*
 * Test for EchoMail, return Area entry.
 */
Area *news_msg(char *line, Node * to)
{
    char *p;
    Area *pa;
#ifdef ACTIVE_LOOKUP
    Active *pg;
#endif                          /* ACTIVE_LOOKUP */
    static Area area;

#ifdef ACTIVE_LOOKUP
    char exec_line[MAXPATH];
#endif                          /* ACTIVE_LOOKUP */
    if (line) {
        /* Message is FIDO EchoMail */
        strip_crlf(line);

        for (p = line + strlen("AREA:"); *p && is_space(*p); p++) ;
        debug(7, "FIDO Area: %s", p);
        pa = areas_lookup(p, NULL, to);
        if (pa) {
            debug(7, "Found: %s %s Z%d", pa->area, pa->group, pa->zone);

#ifdef ACTIVE_LOOKUP
#ifndef SN
            active_init();
#endif
            pg = active_lookup(pa->group);
            if (pg) {
                if (!strcmp(pg->flag, "y")) {
                    debug(7, "Found: %s stat = y", pg->group);
                    return pa;
                } else {
                    debug(7,
                          "Article can't be posted in this newsgroup stat= %s",
                          pg->flag);
                }
            } else {
                if (cf_get_string("AutoCreateNG", TRUE)) {
                    fglog("create newsgroup %s", pa->group);
                    BUF_COPY2(buffer, "%N/ngoper create ", pa->group);
                    sprintf(exec_line, "%s/ngoper create %s", cf_p_bindir(),
                            pa->group);
                    debug(8, "run: %s", exec_line);
                    if (0 != run_system(exec_line))
                        fglog("can't create newsgroup (rc != 0)");
                    return pa;
                } else
                    debug(8, "config: AutoCreateNG not defuned");
            }
#endif                          /* ACTIVE_LOOKUP */

            return pa;
        }

        /* Area not found */
        area.next = NULL;
        area.area = p;
        area.group = NULL;
        area.zone = 0;
        node_invalid(&area.addr);
        area.origin = NULL;
        area.distribution = NULL;
        area.flags = 0;
        area.rfc_lvl = -1;
        area.maxsize = -1;
        tl_init(&area.x_hdr);
        area.encoding = MIME_DEFAULT;

        return &area;
    }

    /* Message is FIDO NetMail */
    return NULL;
}

/*
 * Check for 8bit characters in message body
 */
int check_8bit(Textlist * tl)
{
    Textline *pl;
    char *p;

    for (pl = tl->first; pl; pl = pl->next)
        for (p = pl->line; *p && *p != '\r'; p++)
            if (*p & 0x80)
                return TRUE;

    return FALSE;
}

/*
 * Check for 8bit characters in any string
 */
int check_8bit_s(char *s, int len)
{
    char *p;

    if (!s)
        return FALSE;

    for (p = s; (int)(p - s) < len && *p != '\0'; p++)
        if (*p & 0x80)
            return TRUE;

    return FALSE;
}

/*
 * Check for valid domain name string
 */
int check_valid_domain(char *s)
{
    if (!*s)
        return FALSE;
    while (*s) {
        if (!isalnum(*s) && *s != '-' && *s != '.')
            return FALSE;
        s++;
    }
    return TRUE;
}

struct encoding_state {
    int cvt8;
    char *cs_in;
    char *cs_out;
    bool plain_headers;
};

static int recode_header(Textline * tl, struct encoding_state *state)
{
    size_t len;
    char *tmpbuf;
    int rc;
    size_t size;
    size_t new_size;

    len = strlen(tl->line);
    size = len + 1;

    /* recode with the header's name, hopefully no charset amend ascii */
    rc = charset_recode_buf(&tmpbuf, &new_size, tl->line, size,
                            state->cs_in, state->cs_out);

    if (rc != OK) {
        fglog("ERROR: could not recode header from %s to %s\n",
              state->cs_in, state->cs_out);
        return ERROR;
    }

    xfree(tl->line);
    tl->line = tmpbuf;

    return OK;
}

/*
 * If body is not encoded, provide default for headers, if they are
 * not plain
 */
static int sanitize_encoding(int enc)
{
    switch (enc) {
    case MIME_B64:
        /* fallthrough */
    case MIME_QP:
        return enc;
    }

    return MIME_DEFAULT;
}

static char *encode_skip[] = {
    "Path:",
    "Message-ID:",
    "References:",
    "From ",
};

static int encode_header(Textline * tl, void *arg)
{
    struct encoding_state *state = arg;
    char *tmpbuf = NULL;
    int rc;
    size_t len;
    int i;
    int enc = state->cvt8;

    for (i = 0; i < sizeof(encode_skip) / sizeof(encode_skip[0]); i++) {
        size_t len;

        len = strlen(encode_skip[i]);
        if (strncasecmp(tl->line, encode_skip[i], len) == 0)
            return OK;
    }

    rc = recode_header(tl, state);
    if (rc != OK)
        return rc;

    if (state->plain_headers)
        return OK;

    len = strlen(tl->line);
    if (charset_is_7bit(tl->line, len) && len <= MIME_STRING_LIMIT)
        return OK;

    enc = sanitize_encoding(enc);
    rc = mime_header_enc(&tmpbuf, tl->line, state->cs_out, enc);
    if (rc != OK)
        return OK;              /* skip misformatted */

    xfree(tl->line);
    tl->line = tmpbuf;

    return OK;
}

/*
 * temporal copy'n'paste. Must traverse all the characters across the
 * lines
 */
static int recode_body(Textline * tl, void *arg)
{
    struct encoding_state *state = arg;
    char *line = tl->line;
    char *tmpbuf = NULL;
    size_t len;
    size_t new_size;
    size_t size;
    int rc;

    len = strlen(line);
    size = len + 1;

    rc = charset_recode_buf(&tmpbuf, &new_size, line, size,
                            state->cs_in, state->cs_out);

    if (rc != OK) {
        fglog("ERROR: could not recode body line from %s to %s\n",
              state->cs_in, state->cs_out);
        return ERROR;
    }

    xfree(tl->line);
    tl->line = tmpbuf;

    return OK;
}

static char *cvt8_to_str(int cvt8)
{
    switch (cvt8) {
    case MIME_7BIT:
        return "7bit";
    case MIME_8BIT:
        return "8bit";
    case MIME_B64:
        return "base64";
    case MIME_QP:
        return "quoted-printable";
    default:
        return cvt8_to_str(MIME_DEFAULT);
    }
}

static void ftn2rfc_finish_mime(Textlist * hdr, Textlist * body,
                                MIMEInfo * mime,
                                char *cs_in, char *cs_out,
                                int cvt8, bool plain_headers)
{
    Textlist body_encoded;
    struct encoding_state en_state;
    char *mime_type = mime->type;
    char *charset = "";
    char *encoding;
    int body_encoding = cvt8;

    /*
     * Check for 8bit characters in message body. If none are
     * found, don't use the quoted-printable encoding or 8bit.
     * Headers handled separately.
     */
    if (!check_8bit(body)) {
        body_encoding = MIME_7BIT;
        debug(5, "Body encoding switched to 7bit\n");
    }

    if (mime_type == NULL) {
        mime_type = "text/plain; charset=";
        if (body_encoding == MIME_7BIT)
            charset = CHARSET_STD7BIT;
        else
            charset = cs_out;
    }

    encoding = cvt8_to_str(body_encoding);

    /* MIME header */
    tl_appendf(hdr, "MIME-Version: 1.0\n");
    tl_appendf(hdr, "Content-Type: %s%s\n", mime_type, charset);
    tl_appendf(hdr, "Content-Transfer-Encoding: %s\n", encoding);

    en_state.cvt8 = cvt8;
    en_state.cs_in = cs_in;
    en_state.cs_out = cs_out;
    en_state.plain_headers = plain_headers;

    /* does recoding as well */
    tl_for_each(hdr, encode_header, &en_state);

    en_state.cvt8 = body_encoding;
    /* should not harm for 7 bit */
    tl_for_each(body, recode_body, &en_state);

    if (body_encoding == MIME_B64)
        mime_b64_encode_tl(body, &body_encoded);
    else if (body_encoding == MIME_QP)
        mime_qp_encode_tl(body, &body_encoded);
    else
        goto out;

    tl_clear(body);
    *body = body_encoded;
 out:
    return;
}

static char *charset_from_kludge(MsgBody * body)
{
    Textlist *kludge = &body->kludge;
    char codepage[8] = "CP";    /* rest is \0, including final */
    char *p;
    char *names[] = { "CHRS", "CHARSET" };
    int i;

    /* CHRS: IBMPC CODEPAGE: CP... is priority */
    p = kludge_get(kludge, "CODEPAGE", NULL);
    if (p != NULL) {
        strncpy(codepage + 2, p, sizeof(codepage) - 1 - 2);
        return charset_chrs_name(codepage);
    }

    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        p = kludge_get(kludge, names[i], NULL);
        if (p != NULL)
            return charset_chrs_name(p);
    }

    return NULL;
}

/*
 * Read and convert FTN mail packet
 */
int unpack(FILE * pkt_file, Packet * pkt)
{
    Message msg = { 0 };        /* Message header */
    RFCAddr addr_from, addr_to;
    Textlist tl;                /* Textlist for message body */
    MsgBody body;               /* Message body of FTN message */
    int lines;                  /* Lines in message body */
    Area *area;                 /* Area entry for EchoMail */
    int type;
    Textline *pl;
    char *p, *s, *pt;
    char *msgbody_rfc_from;     /* RFC header From */
    char *msgbody_rfc_reply_to; /* RFC header Reply-To */
    char *msgbody_rfc_to;       /* RFC header To */
    char *msgbody_rfc_cc;       /* RFC header Cc */
    char *msgbody_rfc_bcc;      /* RFC header Bcc */
    char *msgbody_rfc_subject;  /* RFC header Subject */
    char mail_to[MAXINETADDR];  /* Addressee of mail */
    char x_orig_to[MAXINETADDR];
    char *from_line, *to_line;  /* From:, To: */
    char *reply_to_line;        /* Reply-To: */
    char *id_line, *ref_line;   /* Message-ID:, References: */
    char *cc_line, *bcc_line;   /* Cc:, Bcc: */
    char *thisdomain, *uplinkdomain;    /* FQDN addr this node, uplink,  */
    char *origindomain;         /*           node in Origin line */
    char *gateway;              /* ^AGATEWAY */
    Textlist theader;           /* RFC headers */
    Textlist tbody;             /* RFC message body */
    int uucp_flag;              /* To == UUCP or GATEWAY */
    int ret;
    char *split_line;
    int cvt8 = MIME_DEFAULT;    /* MIME_* */
    char *cs_def, *cs_in, *cs_out;  /* Charset def, in(=FTN), out(=RFC) */
    char *cs_save;
    MIMEInfo *mime;
    char *mime_ver, *mime_type, *mime_enc;
    char *carbon_group = NULL;
    int addr_is_restricted = FALSE;
    bool plain_headers = false;

    /*
     * Initialize
     */
    tl_init(&tl);
    tl_init(&theader);
    tl_init(&tbody);
    msg_body_init(&body);
    ret = OK;

    /*
     * Read packet
     */
    type = pkt_get_int16(pkt_file);
    if (type == ERROR) {
        if (feof(pkt_file)) {
            fglog("WARNING: premature EOF reading input packet");
            return OK;
        }

        fglog("ERROR: reading input packet");
        return ERROR;
    }

    while ((type == MSG_TYPE) && !xfeof(pkt_file)) {
        x_orig_to[0] = 0;
        tl_clear(&theader);
        tl_clear(&tbody);

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

        /* Strip spaces at end of line */
        strip_space(msg.name_from);
        strip_space(msg.name_to);
        strip_space(msg.subject);

        /* Replace empty subject */
        if (!*msg.subject)
            BUF_COPY(msg.subject, "(no subject)");

        /*
         * Read & parse message body
         */

        if (pkt_get_body_parse(pkt_file, &body, &msg.node_from, &msg.node_to) !=
            OK)
            fglog("ERROR: parsing message body");
        /* Retrieve address information from kludges for NetMail */
        if (body.area == NULL) {

            if (ftnacl_lookup(&msg.node_from, &msg.node_to, NULL)) {
                debug(7, "ftnacl_lookup(): From=%s, To=%s",
                      znfp1(&msg.node_orig), znfp2(&msg.node_to));
                fglog
                    ("BOUNCE: Postings from address `%s' to  `%s' not allowed - skipped",
                     znfp1(&msg.node_orig), znfp2(&msg.node_to));
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);

                continue;
            }

            /* Don't use point address from packet for Netmail */
            msg.node_from.point = 0;
            msg.node_to.point = 0;
            /* Retrieve complete address from kludges */
            kludge_pt_intl(&body, &msg, FALSE);
            msg.node_orig = msg.node_from;
        } else {
            if (ftnacl_lookup(&msg.node_orig, NULL, body.area)) {
                debug(7, "ftnacl_lookup(): From=%s, echo=%s",
                      znfp1(&msg.node_orig), body.area);
                fglog
                    ("BOUNCE: Postings from address `%s' to area `%s' not allowed - skipped",
                     znfp1(&msg.node_orig), body.area);
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);

                continue;
            }

            /* Specially for echomail */
            msg.node_from = pkt->from;
            msg.node_to = pkt->to;

            /* Try to get address from Origin or MSGID */
            if (OK != msg_parse_origin(body.origin, &msg.node_orig) &&
                OK != msg_parse_msgid(kludge_get(&body.kludge, "MSGID", NULL),
                                      &msg.node_orig)) {
                node_invalid(&msg.node_orig);
            }
        }
        debug(7, "FIDO sender (from/origin): %s", znfp1(&msg.node_orig));

        /*
         * strip_crlf() all kludge and RFC lines
         */
        for (pl = body.kludge.first; pl; pl = pl->next)
            strip_crlf(pl->line);
        for (pl = body.rfc.first; pl; pl = pl->next)
            strip_crlf(pl->line);

        /*
         * X-Split header line
         */
        split_line = NULL;
        if ((p = kludge_get(&body.kludge, "SPLIT", NULL))) {
            split_line = p;
        } else if ((p = kludge_get(&body.body, "SPLIT", NULL))) {
            strip_crlf(p);
            split_line = p;
        }

        /*
         * Remove empty first line after RFC headers and empty last line
         */
        if (body.rfc.first && (pl = body.body.first)) {
            if (pl->line[0] == '\r')
                tl_delete(&body.body, pl);
        }
        if ((pl = body.body.last)) {
            if (pl->line[0] == '\r')
                tl_delete(&body.body, pl);
        }

        /*
         * Check for mail or news.
         *
         * area == NULL  -  NetMail
         * area != NULL  -  EchoMail
         */
        if ((area = news_msg(body.area, &msg.node_to))) {
            cvt8 = area->encoding;
            plain_headers = area->flags & AREA_HEADERS_PLAIN;

            /* Set AKA according to area's zone */
            cf_set_zone(area->zone);

            /* Skip, if unknown and FTNJunkGroup not set */
            if (!area->group && !ftn_junk_group) {
                fglog("unknown area %s", area->area);
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);
                continue;
            }
            if (!area->group) {
                if (ftn_junk_group_keys) {
                    char *p;

                    p = area->area;
                    area = ftn_junk_group_keys;
                    area->area = p;
                }

                area->group = ftn_junk_group;
            }
        } else {
            cvt8 = 0;
            /* Order make priority for netmail for now */
            if (netmail_8bit)
                cvt8 = MIME_8BIT;
            if (netmail_qp)
                cvt8 = MIME_QP;
            if (netmail_b64)
                cvt8 = MIME_B64;

            plain_headers = netmail_headers_plain;

            /* Set AKA according to sender's zone */
            cf_set_zone(msg.node_orig.zone != -1
                        ? msg.node_orig.zone : msg.node_from.zone);
        }

        if (!(mime_ver = rfcheader_get(&body.rfc, "MIME-Version")))
            mime_ver = kludge_get(&body.kludge, "RFC-MIME-Version", NULL);
        if (!(mime_type = rfcheader_get(&body.rfc, "Content-Type")))
            mime_type = kludge_get(&body.kludge, "RFC-Content-Type", NULL);
        if (!(mime_enc = rfcheader_get(&body.rfc, "Content-Transfer-Encoding")))
            mime_enc = kludge_get(&body.kludge,
                                  "RFC-Content-Transfer-Encoding", NULL);
        strip_space(mime_enc);

        if (ignore_mime_type)
            mime_type = NULL;
        mime = get_mime(mime_ver, mime_type, mime_enc);

        /* YK: I still keep that, but it sounds broken for me */
        /*
         * Check for Content-Transfer-Encoding in RFC headers or ^ARFC kludges
         */
        if (mime->encoding) {
            if (strieq(mime->encoding, "7bit"))
                cvt8 = MIME_7BIT;
            if (strieq(mime->encoding, "8bit"))
                cvt8 = MIME_8BIT;
            if (strieq(mime->encoding, "quoted-printable"))
                cvt8 = MIME_QP;
            if (strieq(mime->encoding, "base64"))
                cvt8 = MIME_B64;
        }
        debug(5, "cvt8:%s", cvt8_to_str(cvt8));

        /*
         * Convert message body
         */

        /* charset input (=FTN message) and output (=RFC message) */
        cs_save = NULL;
        cs_def = NULL;
        cs_in = NULL;
        cs_out = NULL;

        if (area) {             /* EchoMail -> News */
            if (area->charset) {
                cs_save = strsave(area->charset);
                cs_def = strtok(cs_save, ":");
                s = strtok(NULL, ":");
                cs_out = strtok(NULL, ":");
            }
        } else {                /* NetMail -> Mail */
            cs_def = netmail_charset_def;
            cs_out = netmail_charset_out;
        }
        /* defaults */
        if (!cs_def)
            cs_def = default_charset_def;
        if (!cs_def)
            cs_def = CHARSET_STDFTN;
        if (!cs_out)
            cs_out = default_charset_out;

        if (!ignore_chrs)
            cs_in = charset_from_kludge(&body);

        lines = 0;
        for (pl = body.body.first; pl; pl = pl->next) {
            p = pl->line;

            if (!ignore_chrs && *p == '\001') { /* Kludge in message body */
                if (strnieq(p + 1, "CHRS: ", 6))
                    if ((s = charset_chrs_name(p + 6)))
                        cs_in = s;
                if (strnieq(p + 1, "CHARSET: ", 9))
                    if ((s = charset_chrs_name(p + 9)))
                        cs_in = s;
            } else {
                msg_xlate_line(buffer, sizeof(buffer), p, ignore_soft_cr);
                tl_append(&tbody, buffer);
                lines++;
            }
        }

        /*
         * Convert FTN from/to addresses to RFCAddr struct
         */
        if (!cs_in)
            cs_in = cs_def;
        if (!cs_out)
            cs_out = CHARSET_STDRFC;

        cs_in = charset_fsc_canonize(cs_in);
        charset_set_in_out(cs_in, cs_out);

        addr_from = rfcaddr_from_ftn(msg.name_from, &msg.node_orig);
        addr_to = rfcaddr_from_ftn(msg.name_to, &msg.node_to);

        uucp_flag = FALSE;
        if (!stricmp(addr_to.real, "UUCP") || !stricmp(addr_to.real, "GATEWAY")) {
            /* Don't output UUCP or GATEWAY as real name */
            uucp_flag = TRUE;
        }

        /*
         * RFC address headers from text body
         */
        msgbody_rfc_from = get_from(&body.rfc, &body.kludge);
        if ((pt = xlat_s(msgbody_rfc_from, NULL)))
            msgbody_rfc_from = pt;
        msgbody_rfc_to = get_to(&body.rfc);
        if ((pt = xlat_s(msgbody_rfc_to, NULL)))
            msgbody_rfc_to = pt;
        msgbody_rfc_reply_to = get_reply_to(&body.rfc);
        if ((pt = xlat_s(msgbody_rfc_reply_to, NULL)))
            msgbody_rfc_reply_to = pt;
        msgbody_rfc_cc = get_cc(&body.rfc);
        if ((pt = xlat_s(msgbody_rfc_cc, NULL)))
            msgbody_rfc_cc = pt;
        msgbody_rfc_bcc = get_bcc(&body.rfc);
        if ((pt = xlat_s(msgbody_rfc_bcc, NULL)))
            msgbody_rfc_bcc = pt;
        msgbody_rfc_subject = get_subject(&body.rfc);
        if ((pt = xlat_s(msgbody_rfc_subject, NULL)))
            msgbody_rfc_subject = pt;

        /*
         * If kill_split is set, skip messages with ^ASPLIT
         */
        if (kill_split) {
            /* ^ASPLIT */
            if ((p = kludge_get(&body.kludge, "SPLIT", NULL))) {
                fglog("skipping split message, origin=%s",
                      znfp1(&msg.node_orig));
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);
                continue;
            }
        }

        /*
         * If -g flag is set for area and message seems to come from
         * another gateway, skip it.
         */
        if (area && (area->flags & AREA_NOGATE)) {
            if (msgbody_rfc_from) {
                fglog("skipping message from gateway, area %s, origin=%s",
                      area->area, znfp1(&msg.node_orig));
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);
                continue;
            }

            /* GIGO */
            if ((p = kludge_get(&body.kludge, "PID", NULL)) &&
                !strnicmp(p, "GIGO", 4)) {
                fglog
                    ("skipping message from gateway (GIGO), area %s, origin=%s",
                     area->area, znfp1(&msg.node_orig));
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);
                continue;
            }

            /* Broken FidoZerb message splitting */
            if ((p = kludge_get(&body.kludge, "X-FZ-SPLIT", NULL))) {
                fglog
                    ("skipping message from gateway (X-FZ-SPLIT), area %s, origin=%s",
                     area->area, znfp1(&msg.node_orig));
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);
                continue;
            }
        }

        /*
         * Do alias checking on both from and to names
         */
        if (!msgbody_rfc_from) {
            Alias *a;

            debug(7, "Checking for alias: %s",
                  s_rfcaddr_to_asc(&addr_from, TRUE));
        /**FIXME: why _strict()?**/
            a = alias_lookup_strict(&msg.node_orig, addr_from.real);
            if (a) {
                if (a->userdom) {
                    debug(7, "Alias found: %s@%s %s %s", a->username,
                          a->userdom, znfp1(&a->node), a->fullname);
                    BUF_COPY(addr_from.addr, a->userdom);
                } else
                    debug(7, "Alias found: %s %s %s", a->username,
                          znfp1(&a->node), a->fullname);
                BUF_COPY(addr_from.user, a->username);
#ifdef ALIASES_ARE_LOCAL
                BUF_COPY(addr_from.addr, cf_fqdn());
#endif
            }
        }
        if (!msgbody_rfc_to) {
            Alias *a;
            char *t;

            debug(7, "Checking for alias: %s",
                  s_rfcaddr_to_asc(&addr_to, TRUE));
        /**FIXME: why _strict()?**/
            debug(7, "node_to = %s, addr_to.real = %s, name = %s, %s",
                  znfp1(&msg.node_to), addr_to.real, msg.name_to, addr_to.user);
            a = alias_lookup_strict(&msg.node_to, addr_to.real);
            if (a && !strchr(msg.name_to, '@')) {
                if (a->userdom) {
                    debug(7, "Alias (AI2) found: %s@%s %s \"%s\"",
                          a->username, a->userdom,
                          znfp1(&a->node), a->fullname);
                    /*BUF_COPY(addr_to.addr, a->userdom); */
                    t = a->username;
                    while (t[0]) {
                        if (*t == ' ')
                            *t = '_';
                        t++;
                    }
                    BUF_COPY5(mail_to, msg.name_to, " <", a->username, "@",
                              a->userdom);

                    BUF_APPEND(mail_to, ">");
                } else {
                    debug(7, "Alias (old) found: %s %s \"%s\"",
                          a->username, znfp1(&a->node), a->fullname);
                    BUF_COPY(mail_to, a->username);
                }
            } else
                BUF_COPY(mail_to, addr_to.user);
        }

        /* Special handling for -t flag (insecure): Messages with To
         * line will be bounced
         */
        if (area == NULL && t_flag && msgbody_rfc_to) {
            debug(1, "Insecure message with To line");
            fglog("BOUNCE: insecure mail from %s",
                  s_rfcaddr_to_asc(&addr_from, TRUE));
            bounce_mail("insecure", &addr_from, &msg, msgbody_rfc_to, &tbody);
            tl_clear(&theader);
            tl_clear(&tbody);
            tl_clear(&tl);
            continue;
        }

        /* There are message trackers out there in FIDONET. Most of
         * they can't handle addressing the gateway so we send mail
         * from "MsgTrack..." etc. to TrackerMail.  */
        if (tracker_mail_to)
            if (!strnicmp(addr_from.user, "MsgTrack", 8) ||
                !strnicmp(addr_from.user, "Reflex_Netmail_Policeman", 24) ||
                !strnicmp(addr_from.user, "TrackM", 6) ||
                !strnicmp(addr_from.user, "ITrack", 6) ||
                !strnicmp(addr_from.user, "FTrack", 6) ||
                !strnicmp(addr_from.user, "O/T-Track", 9)
                /* || whatever ... */ ) {
                debug(1, "Mail from FIDO message tracker");
                BUF_COPY(x_orig_to, mail_to);
                BUF_COPY(mail_to, tracker_mail_to);
                /* Reset uucp_flag to avoid bouncing of these messages */
                uucp_flag = FALSE;
            }

        thisdomain = s_ftn_to_inet(cf_addr());
        uplinkdomain = s_ftn_to_inet(&msg.node_from);
        origindomain = msg.node_orig.zone != -1
            ? s_ftn_to_inet(&msg.node_orig) : FTN_INVALID_DOMAIN;

        /* Bounce mail from nodes not registered in HOSTS, but allow
         * mail to local users.
         */
        if (cf_get_string("HostsRestricted", TRUE))
            addr_is_restricted = TRUE;

        if (addr_is_restricted && !ignore_hosts &&
            area == NULL && msgbody_rfc_to && !addr_is_domain(msgbody_rfc_to)) {
            Host *h;

            /* Lookup host */
            if ((h = hosts_lookup(&msg.node_orig, NULL)) == NULL) {
                /* Not registered in HOSTS */
                debug(1, "Not a registered node: %s", znfp1(&msg.node_orig));
                fglog("BOUNCE: mail from unregistered %s",
                      s_rfcaddr_to_asc(&addr_from, TRUE));
                bounce_mail("restricted", &addr_from, &msg,
                            msgbody_rfc_to, &tbody);
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);
                continue;
            }

            /* Bounce, if host is down */
            if (h->flags & HOST_DOWN) {
                debug(1, "Registered node is down: %s", znfp1(&msg.node_orig));
                fglog("BOUNCE: mail from down %s",
                      s_rfcaddr_to_asc(&addr_from, TRUE));
                bounce_mail("down", &addr_from, &msg, msgbody_rfc_to, &tbody);
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);
                continue;
            }

        }

        /*
         * Check for address in mail_to
         */

        if (area == NULL) {
            if (strchr(mail_to, '@') || strchr(mail_to, '%') ||
                strchr(mail_to, '!')) {
                if (no_address_in_to_field) {
                    debug(1, "Message with address in mail_to: %s", mail_to);
                    fglog("BOUNCE: mail from %s with address in to field: %s",
                          s_rfcaddr_to_asc(&addr_from, TRUE), mail_to);
                    bounce_mail("addrinto", &addr_from, &msg, mail_to, &tbody);
                    tl_clear(&theader);
                    tl_clear(&tbody);
                    tl_clear(&tl);
                    msg_body_clear(&body);
                    continue;
                } else
                    if ((p = strchr(msg.name_to, '@')) ||
                        (p = strchr(msg.name_to, '%')))
                    if (check_valid_domain(p + 1))
                        /* Copy again to avoid "..." */
                        BUF_COPY(mail_to, msg.name_to);
            }
        }

        /*
         * Check for UUCP / GATEWAY, add address to mail without To line
         */
        if (area == NULL && !msgbody_rfc_to &&
            !strchr(mail_to, '@') && !strchr(mail_to, '%') &&
            !strchr(mail_to, '!')) {
            if (uucp_flag) {
                /* Addressed to `UUCP' or `GATEWAY', but no To: line */
                debug(1, "Message to `UUCP' or `GATEWAY' without To line");
                fglog("BOUNCE: mail from %s without To line",
                      s_rfcaddr_to_asc(&addr_from, TRUE));
                bounce_mail("noto", &addr_from, &msg, msgbody_rfc_to, &tbody);
                tl_clear(&theader);
                tl_clear(&tbody);
                tl_clear(&tl);
                msg_body_clear(&body);
                continue;
            }

            BUF_APPEND(mail_to, "@");
            if (use_ftn_to_address)
                /* Add @ftn-to-host */
                BUF_APPEND(mail_to, addr_to.addr);
            else
                /* Add @host.domain to local address */
                BUF_APPEND(mail_to, cf_fqdn());
        }

        /* Construct string for From: header line */
        if (msgbody_rfc_from) {
            RFCAddr rfc;

            rfc = rfcaddr_from_rfc(msgbody_rfc_from);
            if (!rfc.real[0])
                BUF_COPY(rfc.real, addr_from.real);

            addr_from = rfc;
        }
        from_line = s_rfcaddr_to_asc(&addr_from, TRUE);

        /* Construct Reply-To line */
        reply_to_line = msgbody_rfc_reply_to;

        /* Construct string for To:/X-Comment-To: header line */
        if (msgbody_rfc_to) {
            if (strchr(msgbody_rfc_to, '(') || strchr(msgbody_rfc_to, '<') ||
                !*addr_to.real || uucp_flag)
                to_line = s_printf("%s", msgbody_rfc_to);
            else
                to_line = s_printf("%s (%s)", msgbody_rfc_to, addr_to.real);
        } else {
            if (area)
                to_line = s_printf("%s", addr_to.real);
            else
                to_line = s_printf("%s", mail_to);
        }

        /* Construct Cc/Bcc header lines */
        cc_line = msgbody_rfc_to ? msgbody_rfc_cc : NULL;
        bcc_line = msgbody_rfc_to ? msgbody_rfc_bcc : NULL;

        /* Construct Message-ID and References header lines */
        id_line = NULL;
        ref_line = NULL;

        if ((p = kludge_get(&body.kludge, "ORIGID", NULL)))
            id_line = s_msgid_convert_origid(p);
        else if ((p = kludge_get(&body.kludge, "Message-ID", NULL)))
            id_line = s_msgid_convert_origid(p);

        if (gate_rfc_kludge && !id_line) {
            if ((p = kludge_get(&body.kludge, "RFC-Message-ID", NULL)))
                id_line = s_msgid_convert_origid(p);
        }

        if (!id_line) {
            int id_zone;

            if ((p = kludge_get(&body.kludge, "MSGID", NULL))) {
                if (!strncmp(p, "<NOMSGID_", 9)) {
                    fglog("MSGID: %s, not gated", p);
                    tl_clear(&theader);
                    tl_clear(&tbody);
                    tl_clear(&tl);
                    msg_body_clear(&body);
                    continue;
                }
                id_line = s_msgid_fido_to_rfc(p, &id_zone, FALSE, NULL);
                if (no_unknown_msgid_zones)
                    if (id_zone >= -1 && !cf_zones_check(id_zone)) {
                        fglog("MSGID %s: malformed or unknown zone, not gated",
                              p);
                        tl_clear(&theader);
                        tl_clear(&tbody);
                        tl_clear(&tl);
                        msg_body_clear(&body);
                        continue;
                    }
            } else {
                if (no_messages_without_msgid) {
                    fglog("MSGID: none, not gated");
                    tl_clear(&theader);
                    tl_clear(&tbody);
                    tl_clear(&tl);
                    msg_body_clear(&body);
                    continue;
                }
                id_line = s_msgid_default(&msg);
            }
        }
        /* Can't happen, but who knows ... ;-) */
        if (!id_line) {
            fglog("ERROR: id_line==NULL, strange.");
            tl_clear(&theader);
            tl_clear(&tbody);
            tl_clear(&tl);
            msg_body_clear(&body);
            continue;
        }

        if ((p = kludge_get(&body.kludge, "ORIGREF", NULL)))
            ref_line = s_msgid_convert_origid(p);
        else {
            if (gate_rfc_kludge) {
                if ((p = kludge_get(&body.kludge, "RFC-References", NULL)))
                    ref_line = p;
            }
            if ((p = kludge_get(&body.kludge, "REPLY", NULL)))
                ref_line = s_msgid_fido_to_rfc(p, NULL, area != NULL, ref_line);
        }

        /* ^AGATEWAY */
        gateway = kludge_get(&body.kludge, "GATEWAY", NULL);

        /*
         * Output RFC mail/news header
         */

        /* Different header for mail and news */
        if (area == NULL) {     /* Mail */
            fglog("MAIL: %s -> %s", from_line, to_line);

            tl_appendf(&theader,
                       "From %s %s\n", s_rfcaddr_to_asc(&addr_from, FALSE),
                       date(DATE_FROM, NULL));
            tl_appendf(&theader,
                       "Received: by %s (FIDOGATE %s)\n",
                       thisdomain, version_global());
            tl_appendf(&theader,
                       "\tid AA%05d; %s\n", getpid(), date(NULL, NULL));
        } else {                /* News */
        if (!strcmp(thisdomain, uplinkdomain))  /* this == uplink */
            tl_appendf(&theader,
                       "Path: %s!%s!%s\n",
                       thisdomain, origindomain, news_path_tail);
        else
            tl_appendf(&theader,
                       "Path: %s!%s!%s!%s\n",
                       thisdomain, uplinkdomain, origindomain, news_path_tail);
        }

        /* not greater than time with possible timezone */
        if (msg.date > (time(NULL) + (24 * 60 * 60))) {
            tl_appendf(&theader, "X-Original-Date: %s\n",
                       date(NULL, &msg.date));
            msg.date = time(NULL);
        }

        tl_appendf(&theader, "Date: %s\n",
                   date_tz(NULL,
                           &msg.date, kludge_get(&body.kludge, "TZUTC", NULL)));
        tl_appendf(&theader, "From: %s\n", from_line);

        if (!reply_to_line && gate_rfc_kludge) {
            if ((p = kludge_get(&body.kludge, "RFC-Reply-To", NULL))) {
                msg_xlate_line(buffer, sizeof(buffer), p, ignore_soft_cr);
                reply_to_line = buffer;
            }
        }
        if (reply_to_line)
            tl_appendf(&theader, "Reply-To: %s\n", reply_to_line);

        if (gate_rfc_kludge) {
            if ((p = kludge_get(&body.kludge, "RFC-User-Agent", NULL))) {
                msg_xlate_line(buffer, sizeof(buffer), p, ignore_soft_cr);
                tl_appendf(&theader, "User-Agent: %s\n", buffer);
            }
            if ((p = kludge_get(&body.kludge, "RFC-X-NewsReader", NULL))) {
                msg_xlate_line(buffer, sizeof(buffer), p, ignore_soft_cr);
                tl_appendf(&theader, "X-NewsReader: %s\n", buffer);
            }
        }

        if (NULL == msgbody_rfc_subject) {
            msg_xlate_line(buffer, sizeof(buffer), msg.subject, ignore_soft_cr);
            tl_appendf(&theader, "Subject: %s\n", buffer);
        } else {
            tl_appendf(&theader, "Subject: %s\n", msgbody_rfc_subject);
        }
        msg_xlate_line(buffer, sizeof(buffer), id_line, ignore_soft_cr);
        tl_appendf(&theader, "Message-ID: %s\n", buffer);

        /* Different header for mail and news */
        if (area == NULL) {     /* Mail */
            if ((!ref_line || strlen(ref_line) != 8) &&
                (s = kludge_get(&body.kludge, "RFC-References", NULL))) {
                msg_xlate_line(buffer, sizeof(buffer), s, ignore_soft_cr);
                tl_appendf(&theader, "References: %s\n", buffer);
            } else if (ref_line) {
                msg_xlate_line(buffer, sizeof(buffer), ref_line,
                               ignore_soft_cr);
                tl_appendf(&theader, "References: %s\n", buffer);
            }

            tl_appendf(&theader, "To: %s\n", to_line);

            if (cc_line) {
                tl_appendf(&theader, "Cc: %s\n", cc_line);
            }

            if (bcc_line) {
                tl_appendf(&theader, "Bcc: %s\n", bcc_line);
            }
            if (*x_orig_to) {
                msg_xlate_line(buffer, sizeof(buffer), x_orig_to,
                               ignore_soft_cr);
                tl_appendf(&theader, "X-Orig-To: %s\n", x_orig_to);
            }
            if (errors_to) {
                msg_xlate_line(buffer, sizeof(buffer), errors_to,
                               ignore_soft_cr);
                tl_appendf(&theader, "Errors-To: %s\n", buffer);
            }
            /* FTN ReturnReceiptRequest -> Return-Receipt-To */
            if (msg.attr & MSG_RRREQ) {
                msg_xlate_line(buffer, sizeof(buffer),
                               s_rfcaddr_to_asc(&addr_from, FALSE),
                               ignore_soft_cr);
                tl_appendf(&theader, "Return-Receipt-To: %s\n", buffer);
            }
        } else {                /* News */
            char *tm_c, *tm_fc;
            int len_s;

            if (ref_line) {
                msg_xlate_line(buffer, sizeof(buffer), ref_line,
                               ignore_soft_cr);
                tl_appendf(&theader, "References: %s\n", buffer);
            }

            carbon_group = NULL;

            for (p = cf_get_string("CarbonNameGroup", TRUE);
                 p && *p; p = cf_get_string("CarbonNameGroup", FALSE)) {
                BUF_COPY(buffer, p);
                s = xstrtok(buffer, " \t");
                tm_c = xstrtok(NULL, " \t");
                tm_fc = xstrtok(NULL, " \t");
                if (!s)
                    continue;
                len_s = strlen_zero(s);
                if ((strnicmp(msg.name_to, s, len_s) == 0) ||
                    (strnicmp(msg.name_from, s, len_s) == 0))
                    goto carbon;
                pt = s;
                while (*pt) {
                    if (*pt == '_')
                        *pt = ' ';
                    pt++;
                }
                if ((strnicmp(msg.name_to, s, len_s) == 0) ||
                    (strnicmp(msg.name_from, s, len_s) == 0)) {
 carbon:
                    debug(5, "Carbon copy: %s - %s", msg.name_to, s);
                    if (tm_fc && (strnicmp(msg.name_from, s, len_s) == 0))
                        carbon_group = strsave(tm_fc);
                    else if (tm_c && (strnicmp(msg.name_to, s, len_s) == 0))
                        carbon_group = strsave(tm_c);
                    else
                        debug(5, "Carbon group for: %s not defined", s);
                    break;
                }
            }

            if (carbon_group != NULL) {
                tl_appendf(&theader, "Newsgroups: %s,%s\n", area->group,
                           carbon_group);
                carbon_group = NULL;
            } else
                tl_appendf(&theader, "Newsgroups: %s\n", area->group);

            if (!area->group)
                tl_appendf(&theader, "X-FTN-Area: %s\n", area->area);
            if (area->distribution)
                tl_appendf(&theader, "Distribution: %s\n", area->distribution);
            if (to_line)
                tl_appendf(&theader, "X-Comment-To: %s\n", to_line);
        }

        /* Common header */
        if (body.origin) {
            if (gate_rfc_kludge
                && (p = kludge_get(&body.kludge, "RFC-Organization", NULL))) {
                msg_xlate_line(buffer, sizeof(buffer), p, ignore_soft_cr);
                tl_appendf(&theader, "Organization: %s\n", buffer);
            } else {
                if (use_origin_for_organization) {
                    strip_crlf(body.origin);
                    msg_xlate_line(buffer, sizeof(buffer), body.origin,
                                   ignore_soft_cr);

                    if ((p = strrchr(buffer, '(')))
                        *p = 0;
                    strip_space(buffer);
                    p = buffer + strlen(" * Origin: ");
                    while (is_blank(*p))
                        p++;
                    if (strlen(p) == 0)
#ifdef NOINSERT_ORGANIZATION
                        tl_appendf(&theader, "Organization: %s\n", "(none)");
#else                           /* NOINSERT_ORGANIZATION */
                        tl_appendf(&theader, "Organization: %s\n",
                                   cf_p_organization());
#endif                          /* NOINSERT_ORGANIZATION */
                    else
                        tl_appendf(&theader, "Organization: %s\n", p);
                } else
                    tl_appendf(&theader, "Organization: %s\n",
                               cf_p_organization());
            }
        }
        tl_appendf(&theader, "Lines: %d\n", lines);
        if (gateway)
            tl_appendf(&theader, "X-Gateway: FIDO %s [FIDOGATE %s], %s\n",
                       cf_fqdn(), version_global(), gateway);
        else
            tl_appendf(&theader, "X-Gateway: FIDO %s [FIDOGATE %s]\n",
                       cf_fqdn(), version_global());

        if (area == NULL) {
            if (x_ftn_f)
                tl_appendf(&theader, "X-FTN-From: %s @ %s\n",
                           msg.name_from, znfp1(&msg.node_orig));
            if (x_ftn_t)
                tl_appendf(&theader, "X-FTN-To: %s @ %s\n",
                           msg.name_to, znfp1(&msg.node_to));
            if (x_ftn_F) {
                BUF_COPY(buffer, "");
                if (msg.attr & MSG_CRASH)
                    BUF_APPEND(buffer, "Cra ");
                if (msg.attr & MSG_PRIVATE)
                    BUF_APPEND(buffer, "Prv ");
                if (msg.attr & MSG_HOLD)
                    BUF_APPEND(buffer, "Hld ");
                if (msg.attr & MSG_FILE)
                    BUF_APPEND(buffer, "Att ");
                if (msg.attr & MSG_RRREQ)
                    BUF_APPEND(buffer, "Req ");
                if (msg.attr & MSG_AUDIT)
                    BUF_APPEND(buffer, "Aud ");

		/* do not put blank headers, breaks some newsreaders */
		if (buffer[0] != '\0')
		    tl_appendf(&theader, "X-FTN-Flags: %s\n", buffer);
            }
        }
#ifdef X_FTN_FROM_ECHOMAIL
        else {
            if (x_ftn_f)
                tl_appendf(&theader, "X-FTN-From: %s @ %s\n",
                           msg.name_from, znfp1(&msg.node_orig));
            if (x_ftn_F) {
                BUF_COPY(buffer, "");
                if (msg.attr & MSG_CRASH)
                    BUF_APPEND(buffer, "Cra ");
                if (msg.attr & MSG_PRIVATE)
                    BUF_APPEND(buffer, "Prv ");
                if (msg.attr & MSG_HOLD)
                    BUF_APPEND(buffer, "Hld ");
                if (msg.attr & MSG_FILE)
                    BUF_APPEND(buffer, "Att ");
                if (msg.attr & MSG_RRREQ)
                    BUF_APPEND(buffer, "Req ");
                if (msg.attr & MSG_AUDIT)
                    BUF_APPEND(buffer, "Aud ");

		if (buffer[0] != '\0')
		    tl_appendf(&theader, "X-FTN-Flags: %s\n", buffer);
            }
        }
#endif                          /* X_FTN_FROM_ECHOMAIL */

        if (x_ftn_T && body.tear) {
            if (strncmp(body.tear, "--- ", 4))
                tl_appendf(&theader, "X-FTN-Tearline: %s\n", "(none)");
            else {
                strip_crlf(body.tear);
                msg_xlate_line(buffer, sizeof(buffer), body.tear,
                               ignore_soft_cr);
                tl_appendf(&theader, "X-FTN-Tearline: %s\n", buffer + 4);
            }
        }
        if (x_ftn_O && body.origin) {
            strip_crlf(body.origin);
            msg_xlate_line(buffer, sizeof(buffer), body.origin, ignore_soft_cr);

            p = buffer + strlen(" * Origin: ");
            while (is_blank(*p))
                p++;
            tl_appendf(&theader, "X-FTN-Origin: %s\n", p);
        }
        if (x_ftn_V)
            for (pl = body.via.first; pl; pl = pl->next) {
                p = pl->line;
                strip_crlf(p);
                msg_xlate_line(buffer, sizeof(buffer), p + 1, ignore_soft_cr);

                if (!strncmp(buffer, "Via ", 4))
                    tl_appendf(&theader, "X-FTN-Via: %s\n", buffer + 4);
                else if (!strncmp(buffer, "Recd ", 5))
                    tl_appendf(&theader, "X-FTN-Recd: %s\n", buffer + 5);
                else            /* Something wrong if we here */
                    tl_appendf(&theader, "X-FTN-Kludge: %s\n", buffer + 1);
            }
        if (x_ftn_D)
            tl_appendf(&theader, "X-FTN-Domain: Z%d@%s\n",
                       cf_zone(), cf_zones_ftn_domain(cf_zone()));
        if (x_ftn_S)
            for (pl = body.seenby.first; pl; pl = pl->next) {
                p = pl->line;
                strip_crlf(p);
                tl_appendf(&theader, "X-FTN-Seen-By: %s\n", p + 9);
            }
        if (x_ftn_P)
            for (pl = body.path.first; pl; pl = pl->next) {
                p = pl->line;
                strip_crlf(p);
                tl_appendf(&theader, "X-FTN-Path: %s\n", p + 7);
            }
        if (x_ftn_K) {
            for (lines = 1; buffer[lines]; lines++) {
                if (buffer[lines] == ':')
                    buffer[lines] = '%';
            }

            for (pl = body.kludge.first; pl; pl = pl->next) {
                if (!strncmp(pl->line, "RFC", 3))
                    tl_appendf(&theader, "%s\n", pl->line + 1);
                else
                    tl_appendf(&theader, "X-FTN-Kludge: %s\n", pl->line + 1);
            }
        }

        if (split_line)
            tl_appendf(&theader, "X-SPLIT: %s\n", split_line);

        /* Add extra headers */
        if (area)
            for (pl = area->x_hdr.first; pl; pl = pl->next)
                tl_appendf(&theader, "%s\n", pl->line);

        /* adds mime headers, recodes and encodes message */
        ftn2rfc_finish_mime(&theader, &tbody, mime,
                            cs_in, cs_out, cvt8, plain_headers);

        tl_appendf(&theader, "\n");

        /* Write header and message body to output file */
        if (area) {
            if (!mail_file('n'))
                if (mail_open('n') == ERROR) {
                    ret = ERROR;
                    break;
                }

            /* News batch */
            if (!single_articles) {
                fprintf(mail_file('n'), "#! rnews %ld\n",
                        tl_size(&theader) + tl_size(&tbody));
            }
            tl_print(&theader, mail_file('n'));
            tl_print(&tbody, mail_file('n'));

            if (single_articles)
                mail_close('n');
        } else {
            if (mail_open('m') == ERROR) {
                ret = ERROR;
                break;
            }

            /* Mail message */
            tl_print(&theader, mail_file('m'));
            tl_print(&tbody, mail_file('m'));
            /* Close mail */
            mail_close('m');
        }

        tl_clear(&theader);
        tl_clear(&tbody);
        tl_clear(&tl);
        msg_body_clear(&body);
        tmps_freeall();

        if (cs_save)
            xfree(cs_save);
    } /**while(type == MSG_TYPE)**/

    if (mail_file('n'))
        mail_close('n');

    TMPS_RETURN(ret);
}

/*
 * Unpack one packet file
 */
int unpack_file(char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;

    /* Open packet and read header */
    pkt_file = fopen(pkt_name, R_MODE);
    if (!pkt_file) {
        fglog("$ERROR: can't open packet %s", pkt_name);
        if (n_flag)
            return ERROR;
        else {
            rename_bad(pkt_name);
            return OK;
        }
    }
    if (pkt_get_hdr(pkt_file, &pkt) == ERROR) {
        fglog("ERROR: reading header from %s", pkt_name);
        if (n_flag)
            return ERROR;
        else {
            rename_bad(pkt_name);
            return OK;
        }
    }

    /* * Unpack it */
    fglog("packet %s (%ldb) from %s to %s", pkt_name, check_size(pkt_name),
          znfp1(&pkt.from), znfp2(&pkt.to));

    if (unpack(pkt_file, &pkt) == ERROR) {
        fglog("ERROR: processing %s", pkt_name);
        if (n_flag)
            return ERROR;
        else {
            rename_bad(pkt_name);
            return OK;
        }
    }

    fclose(pkt_file);

    if (!n_flag && unlink(pkt_name) == ERROR) {
        fglog("$ERROR: can't unlink packet %s", pkt_name);
        rename_bad(pkt_name);
        return OK;
    }

    return OK;
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
options: -1 --single-articles         write single news articles, not batch\n\
         -I --in-dir name             set input packet directory\n\
         -i --ignore-hosts            do not bounce unknown host\n\
         -l --lock-file               create lock file while processing\n\
         -n --no-remove               don't remove/rename input packet file\n\
	 -t --insecure                process insecure packets\n\
         -x --exec-program name       exec program after processing\n\
\n\
	 -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n");
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, ret;
    char *execprog = NULL;
    int l_flag = FALSE;
    char *I_flag = NULL;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    char *pkt_name;
    char *p;

    int option_index;
    static struct option long_options[] = {
        {"single-articles", 0, 0, '1'}, /* Write single article files */
        {"in-dir", 1, 0, 'I'},  /* Set inbound packets directory */
        {"ignore-hosts", 0, 0, 'i'},    /* Do not bounce unknown hosts */
        {"lock-file", 0, 0, 'l'},   /* Create lock file while processing */
        {"no-remove", 0, 0, 'n'},   /* Don't remove/rename packet file */
        {"insecure", 0, 0, 't'},    /* Toss insecure packets */
        {"exec-program", 1, 0, 'x'},    /* Exec program after processing */

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

    while ((c = getopt_long(argc, argv, "1itI:lnx:vhc:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftn2rfc options *****/
        case '1':
            /* Write single article files */
            single_articles = TRUE;
            break;
        case 'I':
            /* Inbound packets directory */
            I_flag = optarg;
            break;
        case 'i':
            /* Don't bounce unknown hosts */
            ignore_hosts = TRUE;
            break;
        case 'l':
            /* Lock file */
            l_flag = TRUE;
            break;
        case 'n':
            /* Don't remove/rename input packet file */
            n_flag = TRUE;
            break;
        case 't':
            /* Insecure */
            t_flag = TRUE;
            break;
        case 'x':
            /* Exec program after unpack */
            execprog = optarg;
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

    /* Read config file */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /* Process config options */
    if (a_flag)
        cf_set_addr(a_flag);
    if (u_flag)
        cf_set_uplink(u_flag);

    cf_i_am_a_gateway_prog();
    cf_debug();

    /* Process local options */
    BUF_EXPAND(in_dir, I_flag ? I_flag : cf_p_pinbound());

    /* Initialize mail_dir[], news_dir[] output directories */
    BUF_EXPAND(mail_dir, cf_p_outrfc_mail());
    BUF_EXPAND(news_dir, cf_p_outrfc_news());

    /* Process optional config statements */
    if (cf_get_string("DotNames", TRUE)) {
        rfcaddr_dot_names();
    }
    if ((p = cf_get_string("FallbackUsername", TRUE)) != NULL) {
        rfcaddr_fallback_username(p);
    }
    if ((p = cf_get_string("X-FTN", TRUE))) {
        while (*p) {
            switch (*p) {
            case 'f':
                x_ftn_f = TRUE;
                break;
            case 't':
                x_ftn_t = TRUE;
                break;
            case 'T':
                x_ftn_T = TRUE;
                break;
            case 'O':
                x_ftn_O = TRUE;
                break;
            case 'V':
                x_ftn_V = TRUE;
                break;
            case 'D':
                x_ftn_D = TRUE;
                break;
            case 'S':
                x_ftn_S = TRUE;
                break;
            case 'P':
                x_ftn_P = TRUE;
                break;
            case 'K':
                x_ftn_K = TRUE;
                break;
            case 'F':
                x_ftn_F = TRUE;
                break;
            }
            p++;
        }
    }
    if ((p = cf_get_string("BounceCCMail", TRUE))) {
        bounce_set_cc(p);
    }
    if ((p = cf_get_string("TrackerMail", TRUE))) {
        tracker_mail_to = p;
    }
    if (cf_get_string("KillUnknownMSGIDZone", TRUE)) {
        no_unknown_msgid_zones = TRUE;
    }
    if (cf_get_string("KillNoMSGID", TRUE)) {
        no_messages_without_msgid = TRUE;
    }
    if (cf_get_string("UseOriginForOrganization", TRUE)) {
        use_origin_for_organization = TRUE;
    }
    if ((p = cf_get_string("FTNJunkGroup", TRUE))) {
        ftn_junk_group = p;
    }
    if ((p = cf_get_string("FTNJunkGroupKeys", TRUE))) {
        ftn_junk_group_keys = areas_parse_line(p);
    }
    if ((p = cf_get_string("ErrorsTo", TRUE))) {
        ftn_junk_group = p;
    }
    if (cf_get_string("NoAddressInToField", TRUE)) {
        no_address_in_to_field = TRUE;
    }
    if (cf_get_string("NetMail8bit", TRUE)) {
        netmail_8bit = TRUE;
    }
    if (cf_get_string("NetMailBase64", TRUE)) {
        netmail_b64 = TRUE;
    }
    if (cf_get_string("NetMailQuotedPrintable", TRUE) ||
        cf_get_string("NetMailQP", TRUE)) {
        netmail_qp = TRUE;
    }
    if (cf_get_string("NetMailHeadersPlain", TRUE)) {
        netmail_headers_plain = TRUE;
    }
    if (cf_get_string("UseFTNToAddress", TRUE)) {
        use_ftn_to_address = TRUE;
    }
    if (cf_get_string("KillSplit", TRUE)) {
        kill_split = TRUE;
    }
    if (cf_get_string("SingleArticles", TRUE)) {
        single_articles = TRUE;
    }
    if ((p = cf_get_string("RFCAddrMode", TRUE))) {
        int m = 0;

        switch (*p) {
        case '(':
        case 'p':
        case '0':
            m = 0;              /* user@do.main (Real Name) */
            break;
        case '<':
        case 'a':
        case '1':
            m = 1;              /* Real Name <user@do.main> */
            break;
        }
        rfcaddr_mode(m);
        debug(8, "actual RFCAddrMode %d", m);
    }
    if ((p = cf_get_string("DefaultCharset", TRUE))) {
        default_charset_def = strtok(p, ":");
        strtok(NULL, ":");
        default_charset_out = strtok(NULL, ":");
    }
    if ((p = cf_get_string("NetMailCharset", TRUE))) {
        netmail_charset_def = strtok(p, ":");
        strtok(NULL, ":");
        netmail_charset_out = strtok(NULL, ":");
    }
    if ((p = cf_get_string("NewsPathTail", TRUE))) {
        /* <FIDOGATE_CONFIG>
         * <CMD>     NewsPathTail
         * <PARA>    STRING
         * <DEFAULT> "fidogate!not-for-mail"
         * <DESC>    The STRING which FIDOGATE's ftn2rfc adds to the
         *           Path header.
         */
        news_path_tail = p;
    }
    if (cf_get_string("IgnoreCHRS", TRUE)) {
        ignore_chrs = TRUE;
    }
    if (cf_get_string("IgnoreSoftCR", TRUE)) {
        ignore_soft_cr = TRUE;
    }
    if (cf_get_string("DontIgnoreMimeType", TRUE)) {
        ignore_mime_type = FALSE;
    }
    if (cf_get_string("DontIgnore0x8d", TRUE)) {
        ignore_soft_cr = FALSE;
    }
    if (cf_get_string("GateRfcKludge", TRUE)) {
        gate_rfc_kludge = TRUE;
    }
    strict = (cf_get_string("FTNStrictPktCheck", TRUE) != NULL);

    /* Init various modules */
    areas_init();
    hosts_init();
    alias_init();
    charset_init();
    acl_init();

    /* If called with -l lock option, try to create lock FILE */
    if (l_flag)
        if (lock_program(PROGRAM, NOWAIT) == ERROR)
            return EXIT_BUSY;

    ret = EXIT_OK;

    if (optind >= argc) {
        /* process packet files in input directory */
        dir_sortmode(DIR_SORTMTIME);
        if (dir_open(in_dir, "*.pkt", TRUE) == ERROR) {
            fglog("$ERROR: can't open directory %s", in_dir);
            if (l_flag)
                unlock_program(PROGRAM);
            exit_free();
            return EX_OSERR;
        }

        for (pkt_name = dir_get(TRUE); pkt_name; pkt_name = dir_get(FALSE)) {
            if (unpack_file(pkt_name) != OK)
                ret = EXIT_ERROR;
            tmps_freeall();
        }

        dir_close();
    } else {
        /* Process packet files on command line */
        for (; optind < argc; optind++) {
            if (unpack_file(argv[optind]) != OK)
                ret = EXIT_ERROR;
            tmps_freeall();
        }
    }

    /* Execute given command, if option -x set.  */
    if (execprog) {
        int retx;

        BUF_EXPAND(buffer, execprog);
        debug(4, "Command: %s", buffer);
        retx = run_system(buffer);
        debug(4, "Exit code=%d", retx);
        if (retx != EXIT_OK)
            ret = EXIT_ERROR;
    }

    if (l_flag)
        unlock_program(PROGRAM);

    exit_free();
    return ret;
}
