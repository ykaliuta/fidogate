/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * Generate custom packet (for test purposes)
 * Main idea and implementation based on ftnoutpkt
 *
 * Copyright (c) 2019 Yauheni Kaliuta <y.kaliuta@gmail.com>
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

/*

  Generates ftn packets. Since there is no dependency of fidogate
  configuration, all the non-library information is provided with
  command line.

  Addresses: -t/--to, -f/--from to set address/name for both pkt and
             msg (and name for msg)
             --pkt-to, --pkt-from to set pkt address;
             --msg-to, --msg-from to set msg name/address.
  Argument order matters (--to after --pkt-to overrides former --pkt-to).

  Encoding: -r/--raw -- no recoding performed, user should supply
            charset kludges; Has priority if given;
            -c/--charset -- text and kludges recoded from locale
	    charset to the given charset. Only iconv charset accepted.

 */

#include "fidogate.h"
#include "getopt.h"
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>

#define PROGRAM		"ftngenpkt"

struct cfg {
    Node pkt_from;
    Node pkt_to;
    FTNAddr msg_from;
    FTNAddr msg_to;
    Textlist kludges;
    char *subject;
    char *area;
    char *origin;
    char *tearline;
    char *charset;
    char *output;
    char *input;
    char *password;
    time_t pkt_date;
    time_t msg_date;
    char *msgid;
    char *path;
    Textlist seenby;
    bool no_chrs;
    bool no_via;
    bool no_tearline;
    bool no_origin;
    bool raw;

    bool allocated;             /* subj, origin, tearline */
};

static void cfg_init(struct cfg *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    tl_init(&cfg->kludges);
    node_invalid(&cfg->pkt_from);
    node_invalid(&cfg->pkt_to);
    node_invalid(&cfg->msg_from.node);
    node_invalid(&cfg->msg_to.node);
    cfg->subject = "(no subject)";
    cfg->origin = "Default origin";
    cfg->tearline = PROGRAM;
    cfg->charset = "UTF-8";
    tl_init(&cfg->seenby);
}

static void cfg_free(struct cfg *cfg)
{
    tl_clear(&cfg->kludges);
    if (!cfg->allocated)
        return;
    free(cfg->subject);
    free(cfg->origin);
    free(cfg->tearline);
    tl_clear(&cfg->seenby);
}

static char *recode_string(char *s, char *charset)
{
    char *d;
    size_t s_len;
    size_t d_len;
    int rc;

    if (s == NULL)
        return NULL;

    /* most likely UTF-8 to 8 bit recoding */
    s_len = strlen(s) + 1;

    /* "" should mean locale charset */
    rc = charset_recode_buf(&d, &d_len, s, s_len, "", charset);
    if (rc != OK) {
        fprintf(stderr, "Could not recode string %s\n", s);
        abort();
    }
    return d;
}

static void tl_recode(Textlist * tl, char *charset)
{
    Textline *cur;
    char *p;

    for (cur = tl->first; cur != NULL; cur = cur->next) {
        p = recode_string(cur->line, charset);
        free(cur->line);
        cur->line = p;
    }
}

/* recodes buffer "in-place" (actually, copying back) */
static void buffer_recode(char *b, char *charset)
{
    char *p;

    p = recode_string(b, charset);
    strcpy(b, p);
    free(p);
}

static void cfg_recode(struct cfg *cfg)
{
    char *c = cfg->charset;

    cfg->subject = recode_string(cfg->subject, c);
    cfg->origin = recode_string(cfg->origin, c);
    cfg->tearline = recode_string(cfg->tearline, c);

    tl_recode(&cfg->kludges, c);
    buffer_recode(cfg->msg_from.name, c);
    buffer_recode(cfg->msg_to.name, c);

    cfg->allocated = true;
}

static void put_msgid(FILE * fp, struct cfg *cfg)
{
    int r;

    fprintf(fp, "\001MSGID: ");
    if (cfg->msgid) {
        fprintf(fp, "%s", cfg->msgid);
    } else {
        r = rand();
        fprintf(fp, "%s %08x", znf1(&cfg->msg_from.node), r);
    }
    fprintf(fp, "\r\n");
}

static void put_chrs(FILE * fp, struct cfg *cfg)
{
    char *c = cfg->charset;

    if (cfg->no_chrs)
        return;

    fprintf(fp, "\001CHRS: %s 2\r\n", c);
}

static void put_kludges(FILE * fp, struct cfg *cfg)
{
    tl_print_xx(&cfg->kludges, fp, "\001", "\r\n");
}

static void put_tearline(FILE * fp, struct cfg *cfg)
{
    if (cfg->no_tearline)
        return;

    fprintf(fp, "\r\n--- %s\r\n", cfg->tearline);
}

static void put_via(FILE * fp, struct cfg *cfg)
{
    if (cfg->no_via)
        return;

    fprintf(fp, "\001Via %s @%s FIDOGATE/%s\r\n",
            znf1(&cfg->msg_from.node), date(DATE_VIA, NULL), PROGRAM);
/*
  fprintf(fp, "\001Via FIDOGATE/%s %s, %s\r",
  PROGRAM, znf1(&cfg->msg_from.node), date(DATE_VIA, NULL));
*/
}

static void put_origin(FILE * fp, struct cfg *cfg)
{
    if (cfg->no_origin)
        return;

    fprintf(fp, " * Origin: %s (%s)\r\n", cfg->origin,
            znfp1(&cfg->msg_from.node));
}

static void put_path(FILE * fp, struct cfg *cfg)
{
    if (cfg->path)
        fprintf(fp, "\001PATH: %s\r\n", cfg->path);
}

static void put_seenby(FILE * fp, struct cfg *cfg)
{
    tl_print_xx(&cfg->seenby, fp, "SEEN-BY: ", "\r\n");
}

static bool is_echomail(struct cfg *cfg)
{
    return cfg->area != NULL;
}

static int pkt_put_body(FILE * fp, struct cfg *cfg, Textlist * body)
{
    bool raw = cfg->raw;

    /* Some copy'n'paste from outpkt_netmail */
    if (!raw) {
        put_msgid(fp, cfg);
        put_chrs(fp, cfg);
        put_kludges(fp, cfg);
    }

    tl_print_x(body, fp, "\r\n");

    if (!raw) {
        put_tearline(fp, cfg);
        if (is_echomail(cfg)) {
            put_origin(fp, cfg);
            put_seenby(fp, cfg);
            put_path(fp, cfg);
        } else {
            put_via(fp, cfg);
        }
    }

    putc(0, fp);

    return 0;
}

static void body_recode(Textlist * body, char *charset)
{
    tl_recode(body, charset);
}

static int _packet_write(struct cfg *cfg, Textlist * body, FILE * fp)
{
    Packet pkt;
    Message msg = { 0 };
    int write_kludges = !cfg->raw;
    int rc;

    /*
     * baud
     * version
     * product_l
     * product_h
     * rev_min
     * rev_maj
     * capword
     * psd
     */

    pkt_fill_hdr(&pkt);

    pkt.from = cfg->pkt_from;
    pkt.to = cfg->pkt_to;
    pkt.time = cfg->pkt_date;
    if (pkt.time == 0)
        pkt.time = time(NULL);

    if (cfg->password)
        BUF_COPY(pkt.passwd, cfg->password);
    else
        pkt.passwd[0] = '\0';

    msg.node_from = cfg->msg_from.node;
    msg.node_to = cfg->msg_to.node;
    node_invalid(&msg.node_orig);
    msg.attr = 0;
    msg.cost = 0;
    msg.date = cfg->msg_date;
    if (msg.date == 0)
        msg.date = time(NULL);
    BUF_COPY(msg.name_from, cfg->msg_from.name);
    BUF_COPY(msg.name_to, cfg->msg_to.name);
    BUF_COPY(msg.subject, cfg->subject);
    msg.area = cfg->area;

    rc = pkt_put_hdr_raw(fp, &pkt);
    if (rc != 0)
        return rc;

    rc = pkt_put_msg_hdr(fp, &msg, write_kludges);
    if (rc != 0)
        return rc;

    rc = pkt_put_body(fp, cfg, body);
    if (rc != 0)
        return rc;

    pkt_put_int16(fp, 0);

    return 0;
}

static int packet_write(struct cfg *cfg, Textlist * body)
{
    FILE *fp;
    int rc;

    if ((cfg->output == NULL) || streq(cfg->output, "-")) {
        fp = stdout;
    } else {
        fp = fopen(cfg->output, "w");
        if (fp == NULL) {
            fprintf(stderr, "Could not open output file %s: %m\n", cfg->output);
            return -EIO;
        }
    }

    rc = _packet_write(cfg, body, fp);

    if (fp != stdout)
        fclose(fp);

    return rc;
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] [message]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] [message] \n\n", PROGRAM);
    fprintf(stderr, "\
options:  -f --from NAME@Z:N/F.P       set from FTN addresses\n\
          -t --to NAME@Z:N/F.P         set to FTN addresses\n\
             --pkt-from Z:N/F.P        set pkt from FTN address\n\
             --pkt-to Z:N/F.P          set pkt to FTN address\n\
             --msg-from NAME@Z:N/F.P   set msg from FTN name and address\n\
             --msg-to NAME@Z:N/F.P     set msg to FTN name and address\n\
          -k --kludge KLUDGE           add kludge\n\
          -r --raw                     do not convert charset or add kludges\n\
          -c --charset                 convert local message to the charset\n\
             --no-chrs                 do not add CHRS kludge\n\
          -S --subject SUBJECT         set subject of message\n\
          -A --area AREA               set area name\n\
          -T --tearline TEARLINE       set tearline\n\
	  -O --origin ORIGIN           set origin\n\
             --pkt-date DATE           set pkt date, RFC822\n\
             --msg-date DATE           set msg date, RFC822\n\
          -P --password PASS           set pkt password\n\
          -M --msgid 'NODE SERNO'      set MSGID instead of generated\n\
             --path 'PATH NODES'       set PATH for echomail\n\
             --seen-by 'SEEN-BY LIST'  add SEEN-BY\n\
             --no-via                  do not generate VIA for netmail\n \
             --no-tearline             do not generate Tearline for netmail\n\
             --no-origin               do not generate Origin for echomail\n\
          -i --input  FILE             body text file\n\
          -o --output FILE             set output file\n\
\n\
          -v --verbose                 more verbose\n\
	  -h --help                    this help\n");
}

enum opts {
    OPT_PKTFROM = 1,
    OPT_PKTTO,
    OPT_MSGFROM,
    OPT_MSGTO,
    OPT_PKTDATE,
    OPT_MSGDATE,
    OPT_NOCHRS,
    OPT_PATH,
    OPT_SEENBY,
    OPT_NOVIA,
    OPT_NOTEARLINE,
    OPT_NOORIGIN,
};

static int options_parse(int argc, char **argv, struct cfg *cfg)
{
    int option_index;
    FTNAddr faddr;
    int c;

    static struct option long_options[] = {
        {"from", 1, 0, 'f'},
        {"to", 1, 0, 't'},
        {"pkt-from", 1, 0, OPT_PKTFROM},
        {"pkt-to", 1, 0, OPT_PKTTO},
        {"msg-from", 1, 0, OPT_MSGFROM},
        {"msg-to", 1, 0, OPT_MSGTO},
        {"kludge", 1, 0, 'k'},
        {"raw", 0, 0, 'r'},
        {"charset", 1, 0, 'c'},
        {"no-chrs", 0, 0, OPT_NOCHRS},
        {"subject", 1, 0, 'S'},
        {"area", 1, 0, 'A'},
        {"tearline", 1, 0, 'T'},
        {"origin", 1, 0, 'O'},
        {"pkt-date", 1, 0, OPT_PKTDATE},
        {"msg-date", 1, 0, OPT_MSGDATE},
        {"password", 1, 0, 'P'},
        {"msgid", 1, 0, 'M'},
        {"path", 1, 0, OPT_PATH},
        {"seen-by", 1, 0, OPT_SEENBY},
        {"no-via", 0, 0, OPT_NOVIA},
        {"no-via", 0, 0, OPT_NOTEARLINE},
        {"no-origin", 0, 0, OPT_NOORIGIN},
        {"input", 1, 0, 'i'},
        {"output", 1, 0, 'o'},

        {"verbose", 0, 0, 'v'},
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "f:t:k:rc:S:A:T:O:P:M:i:o:vh",
                            long_options, &option_index)) != -1) {
        switch (c) {
        case 'f':
            faddr = ftnaddr_parse(optarg);
            cfg->pkt_from = faddr.node;
            cfg->msg_from = faddr;
            break;
        case 't':
            faddr = ftnaddr_parse(optarg);
            cfg->pkt_to = faddr.node;
            cfg->msg_to = faddr;
            break;
        case OPT_PKTFROM:
            faddr = ftnaddr_parse(optarg);
            cfg->pkt_from = faddr.node;
            break;
        case OPT_PKTTO:
            faddr = ftnaddr_parse(optarg);
            cfg->pkt_to = faddr.node;
            break;
        case OPT_MSGFROM:
            faddr = ftnaddr_parse(optarg);
            cfg->msg_from = faddr;
            break;
        case OPT_MSGTO:
            faddr = ftnaddr_parse(optarg);
            cfg->msg_to = faddr;
            break;
        case 'k':
            tl_append(&cfg->kludges, optarg);
            break;
        case 'r':
            cfg->raw = true;
            break;
        case 'c':
            cfg->charset = optarg;
            break;
        case OPT_NOCHRS:
            cfg->no_chrs = true;
            break;
        case 'S':
            cfg->subject = optarg;
            break;
        case 'A':
            cfg->area = optarg;
            break;
        case 'T':
            cfg->tearline = optarg;
            break;
        case 'O':
            cfg->origin = optarg;
            break;
        case OPT_PKTDATE:
            cfg->pkt_date = parsedate(optarg, NULL);
            break;
        case OPT_MSGDATE:
            cfg->msg_date = parsedate(optarg, NULL);
            break;
        case 'P':
            cfg->password = optarg;
            break;
        case 'M':
            cfg->msgid = optarg;
            break;
        case OPT_PATH:
            cfg->path = optarg;
            break;
        case OPT_SEENBY:
            tl_append(&cfg->seenby, optarg);
            break;
        case OPT_NOVIA:
            cfg->no_via = true;
            break;
        case OPT_NOORIGIN:
            cfg->no_origin = true;
            break;
        case OPT_NOTEARLINE:
            cfg->no_tearline = true;
            break;
        case 'i':
            cfg->input = optarg;
            break;
        case 'o':
            cfg->output = optarg;
            break;
        case 'v':
            verbose++;
            break;
        case 'h':
            usage();
            return 1;
        default:
            short_usage();
            return -1;
        }
    }
    return 0;
}

static int body_read(Textlist * body, struct cfg *cfg,
                     int optind, int argc, char **argv)
{
    FILE *in;

    if (optind < argc) {
        debug(1, "Reading body from command line\n");

        for (; optind < argc; optind++)
            tl_append(body, argv[optind]);

        return 0;
    }

    if (cfg->input == NULL) {
        fprintf(stderr, "You must supply body via -i or arguments\n");
        short_usage();
        return -EINVAL;
    }

    if (streq(cfg->input, "-")) {
        in = stdin;
    } else {
        in = fopen(cfg->input, "r");
        if (in == NULL) {
            fprintf(stderr, "Could not open input file %s: %m\n", cfg->input);
            return -EIO;
        }
    }

    while (fgets(buffer, BUFFERSIZE, in)) {
        strip_crlf(buffer);
        tl_append(body, buffer);
    }

    if (in != stdin)
        fclose(in);

    return 0;
}

int main(int argc, char **argv)
{
    struct cfg cfg;
    int rc;
    Textlist body;

    setlocale(LC_ALL, "");
    srand(time(NULL));

    log_program(PROGRAM);

    tl_init(&body);
    cfg_init(&cfg);

    rc = options_parse(argc, argv, &cfg);
    if (rc != 0)
        return EX_USAGE;

    rc = body_read(&body, &cfg, optind, argc, argv);
    if (rc != 0)
        return EX_IOERR;

    if (!cfg.raw) {
        body_recode(&body, cfg.charset);
        cfg_recode(&cfg);
    }

    rc = packet_write(&cfg, &body);
    cfg_free(&cfg);
    tl_clear(&body);

    return rc == 0 ? 0 : EX_SOFTWARE;
}
