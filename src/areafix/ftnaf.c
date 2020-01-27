/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Areafix-like AREAS.BBS EchoMail distribution manager. Commands somewhat
 * conforming to FSC-0057.
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

#define PROGRAM		"ftnaf"
#define CONFIG		DEFAULT_CONFIG_MAIN

static int r_flag = FALSE;
static int m_flag = FALSE;

extern char *areas_bbs;

/*
 * Prototypes
 */
void areafix_init(int);
int areafix_auth_check(Node *, char *, int);
void areafix_auth_cmd(void);
char *areafix_areasbbs(void);
void areafix_set_areasbbs(char *name);
char *areafix_name(void);
Node *areafix_auth_node(void);
void areafix_do(Node * node, char *subj, Textlist *, Textlist *);
int areafix_do_cmd(Node *, char *, Textlist *, Textlist *);
void areafix_free(void);

FILE *mailer_open(char *, int, char *, char *);
int mailer_close(FILE *);
int do_mail(void);
void short_usage(void);
void usage(void);

void send_request(Textlist *);

/*
 * Output FTNAddr as User_Name@p.f.n.z.domain
 */
char *s_ftnaddr_print_pfnz(FTNAddr * ftn)
{
    TmpS *s;
    char *p;

    s = tmps_alloc(MAXINETADDR);
    str_copy(s->s, s->len, ftn->name);
    for (p = s->s; *p; p++)
        if (*p == ' ')
            *p = '_';
    str_append(s->s, s->len, "@");
    str_append(s->s, s->len, node_to_pfnz(&ftn->node));
    str_append(s->s, s->len, cf_zones_inet_domain(ftn->node.zone));
    tmps_stripsize(s);

    return s->s;
}

/*
 * Process request message from stdin
 */
int do_mail(void)
{
    RFCAddr from;
    FTNAddr xfrom;
    char *pfrom, *subj, *x_ftn_from;
    Node *n;
    FILE *output;
    Textlist tl, out;

    ftnaddr_invalid(&xfrom);
    tl_init(&tl);
    tl_init(&out);

    /* Read message header and body from stdin */
    header_delete();
    header_read(stdin);
    while (fgets(buffer, BUFFERSIZE, stdin)) {
        strip_crlf(buffer);
        tl_append(&tl, buffer);
    }

    pfrom = header_get("From");
    if (!pfrom)
        TMPS_RETURN(EX_UNAVAILABLE);
    debug(3, "From: %s", pfrom);

    /* Check From / X-FTN-From for FTN address */
    n = NULL;
    if ((x_ftn_from = header_get("X-FTN-From"))) {
        debug(3, "X-FTN-From: %s", x_ftn_from);
        xfrom = ftnaddr_parse(x_ftn_from);
        n = &xfrom.node;
        if (n->zone <= 0)
            n = NULL;
    } else {
        from = rfcaddr_from_rfc(pfrom);
        n = inet_to_ftn(from.addr);
        if (n)
            xfrom.node = *n;
    }

    if (n) {
        debug(3, "FTN address: %s", znfp1(n));
        cf_set_zone(n->zone);
    }

    /* Run Areafix */
    subj = header_get("Subject");
    areafix_do(&xfrom.node, subj, &tl, (r_flag ? NULL : &out));

    /* Address may have been changed using the PASSWD command */
    xfrom.node = *areafix_auth_node();

    /* Reply to FTN address */
    if (x_ftn_from && n)
        pfrom = s_ftnaddr_print_pfnz(&xfrom);
    debug(3, "Sending reply to: %s", pfrom);

    /* Send output to mailer */
    if (!r_flag) {
        output = mailer_open(pfrom, FALSE, "", "");
        if (!output)
            TMPS_RETURN(EX_OSERR);
        tl_print_x(&out, output, "\n");
        TMPS_RETURN(mailer_close(output));
    }

    TMPS_RETURN(EX_OK);
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] [Z:N/F.P command]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] [Z:N/F.P command]\n\n", PROGRAM);
    fprintf(stderr, "\
options: -m --mail                    process Areafix mail on stdin\n\
         -r --no-reply                don't send reply via mail\n\
         -n --no-rewrite              don't rewrite AREAS.BBS\n\
         -b --areas-bbs NAME          use alternate AREAS.BBS\n\
         -F --filefix                 run as Filefix program (FAREAS.BBS)\n\
\n\
         -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n\
         -w --wait [TIME]             wait for areas.bbs lock to be released\n");
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    int n_flag = FALSE;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    int areafix = TRUE;
    int w_flag = FALSE;
    Node node;
    int ret;
    Textlist req;
    char bbslock[MAXPATH];

    int option_index;
    static struct option long_options[] = {
        {"mail", 0, 0, 'm'},
        {"no-reply", 0, 0, 'r'},
        {"no-rewrite", 0, 0, 'n'},
        {"areas-bbs", 1, 0, 'b'},
        {"filefix", 0, 0, 'F'},

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"uplink-addr", 1, 0, 'u'}, /* Set FIDO uplink address */
        {"wait", 1, 0, 'w'},
        {0, 0, 0, 0}
    };

#ifdef SIGPIPE
    /* Ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
#endif

    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "mrnb:Fvhc:a:w:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftnaf options *****/
        case 'm':
            m_flag = TRUE;
            break;
        case 'r':
            r_flag = TRUE;
            break;
        case 'n':
            n_flag = TRUE;
            break;
        case 'b':
            areafix_set_areasbbs(optarg);
            break;
        case 'F':
            areafix = FALSE;
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

    /* Common init */
    areafix_init(areafix);
    /* Read PASSWD */
    passwd_init();
    /* Read HOSTS */
    hosts_init();
    /* Read UPLINKS */
    uplinks_init();
    tl_init(&req);

    ret = 0;

    if (m_flag) {
        /*
         * Process stdin as mail request for Areafix
         */
        if (lock_program(PROGRAM, WAIT) == ERROR)
            ret = EX_OSERR;
        else {
            BUF_COPY2(bbslock, areas_bbs, ".lock");
            if (ERROR == lock_path(bbslock, w_flag ? w_flag : WAIT))
                ret = EX_OSERR;
            if (areasbbs_init(areafix_areasbbs()) == ERROR)
                ret = EX_OSFILE;
            else
                ret = do_mail();
            if (ret == 0 && !n_flag)
                if (areasbbs_rewrite() == ERROR)
                    ret = EX_CANTCREAT;
            unlock_path(bbslock);
        }
        unlock_program(PROGRAM);
    } else {
        /*
         * Process command on command line
         */
        /* Node */
        if (optind >= argc) {
            fprintf(stderr, "%s: expecting Z:N/F.P node\n", PROGRAM);
            short_usage();
            return EX_USAGE;
        }
        if (asc_to_node(argv[optind], &node, FALSE) == ERROR) {
            fprintf(stderr, "%s: invalid node %s\n", PROGRAM, argv[optind]);
            short_usage();
            return EX_DATAERR;
        }
        optind++;

        /*
         * Execute command, always authorized if command line
         */
        areafix_auth_cmd();

        areafix_auth_check(&node, NULL, FALSE);

        if (areasbbs_init(areafix_areasbbs()) == ERROR) {
            areafix_free();
            exit_free();
            return EX_OSFILE;
        }

        /* Command is rest of args on command line */
        buffer[0] = 0;
        for (; optind < argc; optind++) {
            BUF_APPEND(buffer, argv[optind]);
            if (optind < argc - 1)
                BUF_APPEND(buffer, " ");
        }

        if (areafix_do_cmd(&node, buffer, NULL, &req) == ERROR)
            ret = EX_DATAERR;
        else
            send_request(&req);
        if (ret == 0 && !n_flag)
            if (areasbbs_rewrite() == ERROR)
                ret = EX_CANTCREAT;
    }

    areafix_free();
    exit_free();
    return ret;
}
