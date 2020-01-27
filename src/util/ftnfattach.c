/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Attach file to FLO entry in outbound
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

#define PROGRAM		"ftnfattach"
#define CONFIG		DEFAULT_CONFIG_MAIN

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] Z:N/F.P file ...\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] Z:N/F.P file ...\n\n", PROGRAM);
    fprintf(stderr, "\
options:  -B --binkley NAME            set Binkley-style outbound directory\n\
          -F --flavor NAME             Hold | Normal | Direct | Crash\n\
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
    int c;
    char *name;
    int mode;
    char *B_flag = NULL;
    char *F_flag = NULL;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;

    int option_index;
    static struct option long_options[] = {
        {"binkley", 1, 0, 'B'}, /* Binkley outbound base dir */
        {"flavor", 1, 0, 'F'},  /* Outbound flavor */

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"uplink-addr", 1, 0, 'u'}, /* Set FIDO uplink address */
        {0, 0, 0, 0}
    };

    Node node;

    log_program(PROGRAM);
    log_file("stderr");

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "B:F:hvc:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
        case 'B':
            B_flag = optarg;
            break;
        case 'F':
            F_flag = optarg;
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
     * Additional config options
     */
    if (!F_flag)
        F_flag = cf_get_string("FAttachFlav", TRUE);
    if (!F_flag)
        F_flag = "Normal";

    /*
     * Process following command line arguments
     */
    /* FTN address */
    if (argc - optind < 2) {
        short_usage();
        return EX_USAGE;
    }

    if (asc_to_node(argv[optind], &node, FALSE) == ERROR) {
        short_usage();
        return EX_USAGE;
    }
    optind++;

    /* Files */
    for (; optind < argc; optind++) {
        name = argv[optind];
        if (*name == '^' || *name == '#') {
            mode = *name;
            name++;
        } else
            mode = 0;

        if (bink_attach(&node, mode, name, F_flag, TRUE) == ERROR) {
            exit_free();
            return 1;
        }
    }

    exit_free();
    return 0;
}
