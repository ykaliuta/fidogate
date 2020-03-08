/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Debug contents of FTN packet
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

#define PROGRAM		"pktdebug"

static void debug_line(FILE *, char *, int);
void msg_body_debug(FILE *, MsgBody *, char);


/*
 * Debug output of line
 */
static void debug_line(FILE * out, char *line, int crlf)
{
    int c;

    while ((c = *line++))
        if (!(c & 0xe0)) {
            if (crlf || (c != '\r' && c != '\n'))
                fprintf(out, "^%c", '@' + c);
        } else
            putc(c, out);
    putc('\n', out);
}

/*
 * Debug output of message body
 */
void msg_body_debug(FILE * out, MsgBody * body, char crlf)
{
    Textline *p;

    fprintf(out, "----------------------------------------"
            "--------------------------------------\n");
    if (body->area)
        debug_line(out, body->area, crlf);
    for (p = body->kludge.first; p; p = p->next)
        debug_line(out, p->line, crlf);
    fprintf(out, "----------------------------------------"
            "--------------------------------------\n");
    if (body->rfc.first) {
        for (p = body->rfc.first; p; p = p->next)
            debug_line(out, p->line, crlf);
        fprintf(out, "----------------------------------------"
                "--------------------------------------\n");
    }
    for (p = body->body.first; p; p = p->next)
        debug_line(out, p->line, crlf);
    fprintf(out, "----------------------------------------"
            "--------------------------------------\n");
    if (body->tear)
        debug_line(out, body->tear, crlf);
    if (body->origin)
        debug_line(out, body->origin, crlf);
    for (p = body->seenby.first; p; p = p->next)
        debug_line(out, p->line, crlf);
    for (p = body->path.first; p; p = p->next)
        debug_line(out, p->line, crlf);
    for (p = body->via.first; p; p = p->next)
        debug_line(out, p->line, crlf);
    fprintf(out, "========================================"
            "======================================\n");
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] file ...\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
    exit(EX_USAGE);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] file ...\n\n", PROGRAM);
    fprintf(stderr, "\
options:  -m --msg-header              print message header\n\
          -t --msg-text                print message text\n\
          -s --short                   short format\n\
\n\
          -v --verbose                 more verbose\n\
	  -h --help                    this help\n");

    exit(0);
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    FILE *fp;
    Packet pkt;
    Message msg = { 0 };
    Textlist tl;
    int c, type;
    char *name;
    int m_flag = FALSE, t_flag = 0, s_flag = FALSE;
    MsgBody body;
    long n_mail, n_echo;

    int option_index;
    static struct option long_options[] = {
        {"msg-header", 0, 0, 'm'},
        {"msg-text", 0, 0, 't'},
        {"short", 0, 0, 's'},
        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {0, 0, 0, 0}
    };

    log_file("stdout");
    log_program(PROGRAM);

    tl_init(&tl);
    msg_body_init(&body);

    while ((c = getopt_long(argc, argv, "mtsvh",
                            long_options, &option_index)) != EOF)
        switch (c) {
        case 'm':
            m_flag = TRUE;
            break;
        case 't':
            m_flag = TRUE;
            t_flag++;
            break;
        case 's':
            s_flag = TRUE;
            break;

    /***** Common options *****/
        case 'v':
            verbose++;
            break;
        case 'h':
            usage();
            break;
        default:
            short_usage();
            break;
        }

    /*
     * Process following command line arguments
     */
    if (optind == argc)
        short_usage();

    /* Files */
    for (; optind < argc; optind++) {
        name = argv[optind];

        if (!strcmp(name, "-")) /* "-" = stdin */
            fp = stdin;
        else
            fp = xfopen(name, R_MODE);

        n_mail = n_echo = 0;

        do {
            if (pkt_get_hdr(fp, &pkt) == ERROR) {
                printf("ERROR: %s: reading packet header\n", name);
                if (!s_flag) {
                    printf("Partially read ");
                    pkt_debug_hdr(stdout, &pkt, "");
                }
                break;
            }

            if (!s_flag) {
                if (t_flag)
                    fprintf(stdout,
                            "========================================"
                            "======================================\n");
                pkt_debug_hdr(stdout, &pkt, "");
                if (t_flag)
                    fprintf(stdout,
                            "========================================"
                            "======================================\n");
            }

            type = pkt_get_int16(fp);
            if (type == ERROR) {
                if (feof(fp)) {
                    printf("WARNING: %s: premature EOF reading input packet\n",
                           name);
                    break;
                }
                printf("ERROR: %s: reading input packet\n", name);
                break;
            }
            while ((type == MSG_TYPE) && !xfeof(fp)) {
                msg.node_from = pkt.from;
                msg.node_to = pkt.to;
                if (pkt_get_msg_hdr(fp, &msg, false) == ERROR) {
                    printf("ERROR: %s: reading message header\n", name);
                    break;
                }
#ifdef OLD_TOSS
                type = pkt_get_body(fp, &tl);
                if (type == ERROR) {
                    if (feof(fp)) {
                        printf("WARNING: %s: premature EOF reading "
                               "input packet\n", name);
                    } else {
                        printf("ERROR: %s: reading input packet\n", name);
                        break;
                    }
                }
                if ((c = msg_body_parse(&tl, &body)) != OK)
#else
                if ((c = pkt_get_body_parse(fp, &body, &msg.node_from,
                                            &msg.node_to)) != OK)
#endif
                    fprintf(stdout, "ERROR: %s: parsing message "
                            "body failed (%d)\n", name, c);
                if (body.area == NULL) {
                    /* NetMail */
                    n_mail++;
                    /* Retrieve complete address from kludges */
                    kludge_pt_intl(&body, &msg, FALSE);
                } else {
                    /* EchoMail */
                    n_echo++;
                }

                if (m_flag)
                    pkt_debug_msg_hdr(stdout, &msg, "");
                if (t_flag)
                    msg_body_debug(stdout, &body, t_flag > 1 ? TRUE : FALSE);

                tmps_freeall();
            }
        }
        while (0);

        if (fp != stdin)
            fclose(fp);

        /* Short format output */
        if (s_flag)
            printf("%s: %s -> %s, %ld mail, %ld echo\n",
                   name, znfp1(&pkt.from), znfp2(&pkt.to), n_mail, n_echo);

        tmps_freeall();
    }

    exit_free();
    exit(0);
}
