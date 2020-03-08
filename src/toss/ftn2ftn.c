/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * FTN-FTN gateway for NetMail, using the %Z:N/F.P addressing in the
 * from/to fields.
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

#define PROGRAM 	"ftn2ftn"
#define CONFIG		DEFAULT_CONFIG_MAIN

/*
 * Prototypes
 */
void add_via(Textlist *, Node *);
int do_message(Node *, Message *, MsgBody *);
int unpack(Node *, FILE *, Packet *);
int unpack_file(Node *, char *);
int do_outbound(Node *, Node *);

void short_usage(void);
void usage(void);

static char *o_flag = NULL;     /* -o --out-packet-file option */

/*
 * Zone gate addresses
 */
Node gate_a;                    /* FTN address in network A */
Node gate_b;                    /* FTN address in network B */
static bool strict;

/*
 * Add our ^AVia line
 */
void add_via(Textlist * list, Node * gate)
{
#ifndef FTS_VIA
    tl_appendf(list, "\001Via FIDOGATE/%s %s, %s\r",
               PROGRAM, znf1(gate), date(DATE_VIA, NULL));
#else
    tl_appendf(list, "\001Via %s @%s FIDOGATE/%s\r",
               znf1(gate), date(DATE_VIA, NULL), PROGRAM);
#endif                          /* FTS_VIA */
}

/*
 * Process one message for zone gate
 */
int do_message(Node * gate, Message * msg, MsgBody * body)
{
    static int last_zone = -1;
    static FILE *fp;
    char *from, *to;
    char *p;
    Node node;

    from = msg->name_from;
    to = msg->name_to;

    debug(3, "Message to: %s", to);

    p = strrchr(to, '%');
    if (!p)
        p = strrchr(to, '@');

    /*
     * Check for EchoMail
     */
    if (body->area) {
        fglog("ftn2ftn: skipping EchoMail");
        return ERROR;
    }

    /*
     * Gateway addressing with "User Name%Z:N/F.P"
     * "User Name@Z:N/F.P" found. Get address and push
     * to the other side.
     */
    if (p && asc_to_node(p + 1, &node, FALSE) == OK) {
        cf_set_zone(gate->zone);

        /* Add gate addresses to ^Via lines */
        add_via(&body->via, &msg->node_to);
        add_via(&body->via, gate);

        /* Strip % addressing */
        *p = 0;
        /* Add sender address to from name */
        BUF_COPY2(buffer, "%", znf1(&msg->node_from));
        if (strlen(from) + strlen(buffer) > MSG_MAXNAME - 1)
            from[MSG_MAXNAME - 1 - strlen(buffer)] = 0;
        str_append(from, MSG_MAXNAME, buffer);
        /* Readdress */
        msg->node_from = *gate;
        msg->node_to = node;
    }
    /*
     * Local message, simply readdress to our main AKA
     */
    else {
        cf_set_zone(msg->node_to.zone);

        /* Add gate addresses to ^Via lines */
        add_via(&body->via, &msg->node_to);

        /* Readdress */
        msg->node_to = cf_n_addr();
    }

    /*
     * Write this message to the output packet
     */
    if (!o_flag && cf_zone() != last_zone)
        pkt_close();
    if (!pkt_isopen())
        if ((fp = pkt_open(o_flag, NULL, NULL, FALSE)) == NULL)
            return ERROR;
    last_zone = cf_zone();

    msg->area = NULL;
    if (pkt_put_msg_hdr(fp, msg, TRUE) != OK)
        return ERROR;
    if (msg_put_msgbody(fp, body) != OK)
        return ERROR;

    return OK;
}

/*
 * Unpack and process messages
 */
int unpack(Node * gate, FILE * pkt_file, Packet * pkt)
{
    Message msg = { 0 };
    Textlist tl;
    MsgBody body;
    int type;

    tl_init(&tl);
    msg_body_init(&body);

    type = pkt_get_int16(pkt_file);
    while ((type == MSG_TYPE) && !xfeof(pkt_file)) {
        msg.node_from = pkt->from;
        msg.node_to = pkt->to;
        if (pkt_get_msg_hdr(pkt_file, &msg, strict) == ERROR)
            TMPS_RETURN(ERROR);

        if (pkt_get_body_parse(pkt_file, &body, &msg.node_from, &msg.node_to) !=
            OK) {
            fglog("ftn2ftn: error parsing message body");
            TMPS_RETURN(ERROR);
        }
        /* Retrieve address information from body */
        kludge_pt_intl(&body, &msg, TRUE);

        if (do_message(gate, &msg, &body) != OK)
            TMPS_RETURN(ERROR);
    }

    /* Close packet */
    pkt_close();

    TMPS_RETURN(OK);
}

/*
 * Unpack one packet file
 */
int unpack_file(Node * gate, char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;

    debug(1, "Unpacking FTN packet %s", pkt_name);

    /*
     * Open packet and read header
     */
    pkt_file = fopen(pkt_name, R_MODE);
    if (!pkt_file) {
        fglog("$Can't open packet %s", pkt_name);
        exit_free();
        exit(EX_OSERR);
    }
    if (pkt_get_hdr(pkt_file, &pkt) == ERROR) {
        fglog("Error reading header from %s", pkt_name);
        exit_free();
        exit(EX_OSERR);
    }

    /*
     * Unpack it
     */
    if (unpack(gate, pkt_file, &pkt) != OK) {
        fclose(pkt_file);
        fglog("Error in unpacking %s", pkt_name);
        exit_free();
        exit(EX_OSERR);
    }

    fclose(pkt_file);

    if (unlink(pkt_name)) {
        fglog("$Could not unlink packet %s", pkt_name);
        exit_free();
        exit(EX_OSERR);
    }

    return OK;
}

/*
 * Process packet for gateway address in Binkley outbound
 */
int do_outbound(Node * gate_outb, Node * gate_other)
{
    char *name;

    /* Create BSY file */
    if (bink_bsy_create(gate_outb, NOWAIT) == ERROR)
        return ERROR;

    /* Find and process OUT packet file */
    if ((name = bink_find_out(gate_outb, NULL)))
        unpack_file(gate_other, name);

    /* Delete BSY file */
    if (bink_bsy_delete(gate_outb) == ERROR)
        return ERROR;

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
options: -A --address-a Z:N/F.P       FTN address in network A\n\
         -B --address-b Z:N/F.P       FTN address in network B\n\
	 -o --out-packet-file NAME    set output packet file name\n\
	 -O --out-packet-dir NAME     set output packet directory\n\
\n\
         -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n");

    exit(0);
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    char *O_flag = NULL;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;

    int option_index;
    static struct option long_options[] = {
        {"address-a", 1, 0, 'A'},   /* Address A */
        {"address-b", 1, 0, 'B'},   /* Address B */
        {"out-packet-file", 1, 0, 'o'}, /* Set packet file name */
        {"out-dir", 1, 0, 'O'}, /* Set packet directory */

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

    while ((c = getopt_long(argc, argv, "A:B:o:O:vhc:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftn2ftn options *****/
        case 'A':
            if (asc_to_node(optarg, &gate_a, FALSE) != OK) {
                fprintf(stderr, "%s: option -A: illegal address %s\n",
                        PROGRAM, optarg);
                exit_free();
                exit(EX_USAGE);
            }
            break;
        case 'B':
            if (asc_to_node(optarg, &gate_b, FALSE) != OK) {
                fprintf(stderr, "%s: option -B: illegal address %s\n",
                        PROGRAM, optarg);
                exit_free();
                exit(EX_USAGE);
            }
            break;
        case 'o':
            /* Set packet file name */
            o_flag = optarg;
            break;
        case 'O':
            /* Set packet dir */
            O_flag = optarg;
            break;

    /***** Common options *****/
        case 'v':
            verbose++;
            break;
        case 'h':
            usage();
            exit(0);
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
            exit(EX_USAGE);
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
    debug(4, "gateway address A: %s", znfp1(&gate_a));
    debug(4, "gateway address B: %s", znfp1(&gate_b));

    strict = (cf_get_string("FTNStrictPktCheck", TRUE) != NULL);

    /*
     * Process local options
     */
    if (O_flag)
        pkt_outdir(O_flag, NULL);
    else
        pkt_outdir(cf_p_outpkt(), NULL);

    passwd_init();

    /*
     * Process command line file args
     */
    if (optind >= argc) {
        /* Process outbound packets for both addresses */
        do_outbound(&gate_a, &gate_b);
        tmps_freeall();
        do_outbound(&gate_b, &gate_a);
        tmps_freeall();
    } else
        for (; optind < argc; optind++) {
            unpack_file(&gate_a, argv[optind]);
            tmps_freeall();
        }

    exit_free();
    exit(EX_OK);
}
