/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Template for utility processing FTN packets
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

#include <signal.h>

#include "fidogate.h"
#include "getopt.h"

#define PROGRAM 	"pkttmpl"
#define CONFIG		DEFAULT_CONFIG_MAIN

/* Prototypes */
int do_netmail(Message *, MsgBody *);
int do_echomail(Message *, MsgBody *);
int do_packet(FILE *, Packet *);
int do_file(char *);
void prog_signal(int);
void short_usage(void);
void usage(void);

/* Global vars */
static char in_dir[MAXPATH];    /* Input directory */
static int must_exit = FALSE;   /* Flag for -x operation */
static int severe_error = OK;   /* ERROR: exit after error */
static int signal_exit = FALSE; /* Flag: TRUE if signal received */
static bool strict;

/*
 * Process one NetMail message
 */
int do_netmail(Message * msg, MsgBody * body)
{

    return OK;
}

/*
 * Process one EchoMail message
 */
int do_echomail(Message * msg, MsgBody * body)
{

    return OK;
}

/*
 * Read and process FTN packets
 */
int do_packet(FILE * pkt_file, Packet * pkt)
{
    Message msg = { 0 };        /* Message header */
    Textlist tl;                /* Textlist for message body */
    MsgBody body;               /* Message body of FTN message */
    int type, ret_n;

    /* Initialize */
    tl_init(&tl);
    msg_body_init(&body);

    /* Read packet */
    type = pkt_get_int16(pkt_file);
    if (type == ERROR) {
        if (feof(pkt_file)) {
            fglog("WARNING: premature EOF reading input packet");
            TMPS_RETURN(OK);
        }

        fglog("ERROR: reading input packet");
        TMPS_RETURN(ERROR);
    }

    while ((type == MSG_TYPE) && !xfeof(pkt_file)) {
        /* Read message header */
        msg.node_from = pkt->from;
        msg.node_to = pkt->to;
        if (pkt_get_msg_hdr(pkt_file, &msg, strict) == ERROR) {
            fglog("ERROR: reading input packet");
            TMPS_RETURN(ERROR);
        }

        /* Read & parse message body */
        if (pkt_get_body_parse(pkt_file, &body, &msg.node_from, &msg.node_to) !=
            OK)
            fglog("ERROR: parsing message body");
        /* Retrieve address information from kludges for NetMail */
        if (body.area == NULL) {
            /* Retrieve complete address from kludges */
            kludge_pt_intl(&body, &msg, TRUE);
            msg.node_orig = msg.node_from;

            debug(5, "NetMail: %s -> %s",
                  znfp1(&msg.node_from), znfp2(&msg.node_to));

            ret_n = do_netmail(&msg, &body);
            msg_body_clear(&body);
            if (ret_n == ERROR)
                TMPS_RETURN(ERROR);
        } else {
            /* Retrieve address information from * Origin line */
            if (msg_parse_origin(body.origin, &msg.node_orig) == ERROR)
                /* No * Origin line address, use header */
                msg.node_orig = msg.node_from;

            debug(5, "EchoMail: %s -> %s",
                  znfp1(&msg.node_from), znfp2(&msg.node_to));
            ret_n = do_echomail(&msg, &body);
            msg_body_clear(&body);

            if (ret_n == ERROR)
                TMPS_RETURN(ERROR);
        }

        /*
         * Exit if signal received
         */
        if (signal_exit) {
            TMPS_RETURN(severe_error = ERROR);
        }
    } /**while(type == MSG_TYPE)**/

    TMPS_RETURN(OK);
}

/*
 * Process one packet file
 */
int do_file(char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;
    long pkt_size;

    /* Open packet and read header */
    pkt_file = fopen(pkt_name, R_MODE);
    if (!pkt_file) {
        fglog("ERROR: can't open packet %s", pkt_name);
        rename_bad(pkt_name);
        TMPS_RETURN(OK);
    }
    if (pkt_get_hdr(pkt_file, &pkt) == ERROR) {
        fglog("ERROR: reading header from %s", pkt_name);
        fclose(pkt_file);
        rename_bad(pkt_name);
        TMPS_RETURN(OK);
    }

    /* Process it */
    pkt_size = check_size(pkt_name);
    fglog("packet %s (%ldb) from %s to %s", pkt_name, pkt_size,
          znfp1(&pkt.from), znfp2(&pkt.to));

    if (do_packet(pkt_file, &pkt) == ERROR) {
        fglog("ERROR: processing %s", pkt_name);
        fclose(pkt_file);
        rename_bad(pkt_name);
        TMPS_RETURN(severe_error);
    }

    fclose(pkt_file);

    if (unlink(pkt_name)) {
        fglog("ERROR: can't unlink %s", pkt_name);
        rename_bad(pkt_name);
        TMPS_RETURN(ERROR);
    }

    TMPS_RETURN(OK);
}

/*
 * Signal handler
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

    fglog("WARNING: KILLED%s: exit forced", name);
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
options: -I --in-dir DIR              set input packet directory\n\
         -O --out-dir DIR             set output packet directory\n\
         -l --lock-file               create lock file while processing\n\
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
    int l_flag = FALSE;
    char *I_flag = NULL, *O_flag = NULL;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    char *pkt_name;

    int option_index;
    static struct option long_options[] = {
        {"in-dir", 1, 0, 'I'},  /* Set inbound packets directory */
        {"lock-file", 0, 0, 'l'},   /* Create lock file while processing */
        {"out-dir", 1, 0, 'O'}, /* Set packet directory */

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"uplink-addr", 1, 0, 'u'}, /* Set FIDO uplink address */
        {0, 0, 0, 0}
    };

    /* Log name */
    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();

    /* Parse options */
    while ((c = getopt_long(argc, argv, "O:I:lvhc:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** program options *****/
        case 'I':
            I_flag = optarg;
            break;
        case 'l':
            l_flag = TRUE;
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
        default:
            short_usage();
            return EX_USAGE;
            break;
        }

    /* Read config file */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /* Process config option */
    if (a_flag)
        cf_set_addr(a_flag);
    if (u_flag)
        cf_set_uplink(u_flag);

    cf_debug();

    /* Process optional config statements */
    if (cf_get_string("XXX", TRUE)) {
        debug(8, "config: XXX");

    }
    strict = (cf_get_string("FTNStrictPktCheck", TRUE) != NULL);

    /* Process local options */
    BUF_EXPAND(in_dir, I_flag ? I_flag : cf_p_pinbound());
    pkt_outdir(O_flag ? O_flag : cf_p_outpkt(), NULL);

    /* Install signal/exit handlers */
    signal(SIGHUP, prog_signal);
    signal(SIGINT, prog_signal);
    signal(SIGQUIT, prog_signal);

    /***** Main processing loop *****/
    ret = EXIT_OK;

    if (optind >= argc) {
        /* process packet files in directory */
        dir_sortmode(DIR_SORTMTIME);
        if (dir_open(in_dir, "*.pkt", TRUE) == ERROR) {
            fglog("$ERROR: can't open directory %s", in_dir);
            exit_free();
            return EX_OSERR;
        }

        /* Lock file */
        if (l_flag)
            if (lock_program(PROGRAM, FALSE) == ERROR) {
                /* Already busy */
                exit_free();
                return EXIT_BUSY;
            }

        for (pkt_name = dir_get(TRUE); pkt_name; pkt_name = dir_get(FALSE)) {
            if (do_file(pkt_name) == ERROR) {
                ret = EXIT_ERROR;
                break;
            }
            if (must_exit) {
                ret = EXIT_CONTINUE;
                break;
            }
        }

        dir_close();

        /* Lock file */
        if (l_flag)
            unlock_program(PROGRAM);
    } else {
        /* Lock file */
        if (l_flag)
            if (lock_program(PROGRAM, FALSE) == ERROR) {
                /* Already busy */
                exit_free();
                return EXIT_BUSY;
            }

        /* Process packet files on command line */
        for (; optind < argc; optind++) {
            if (do_file(argv[optind]) == ERROR) {
                ret = EXIT_ERROR;
                break;
            }
            if (must_exit) {
                ret = EXIT_CONTINUE;
                break;
            }
        }

        /* Lock file */
        if (l_flag)
            unlock_program(PROGRAM);
    }

    exit_free();
    return ret;
}
