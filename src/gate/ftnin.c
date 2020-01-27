/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Search for mail packets destined to gateway's FTN addresses and feed
 * them to ftn2rfc.
 *
 * With full supporting cast of busy files and locking. ;-)
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

#define PROGRAM		"ftnin"
#define CONFIG		DEFAULT_CONFIG_GATE

#define FTN2RFC		"ftn2rfc"

/*
 * Prototypes
 */
void args_add(char *);
int do_packets(void);
int exec_ftn2rfc(char *);

void short_usage(void);
void usage(void);

/* Filenames read from FLO file */
static char line[MAXPATH];

/* Command and args ftn2rfc */
static char cmd[MAXPATH];
static char args[MAXPATH];

/* Command for script */
static char script[MAXPATH];

/*
 * Command line options
 */
int n_flag = FALSE;
int x_flag = FALSE;

/*
 * Add string to args[]
 */
void args_add(char *s)
{
    BUF_APPEND(args, s);
}

/*
 * Process line from FLO file
 */
int do_flo_line(char *s)
{
    if (*s == '^' || *s == '#')
        s++;                    /* mode is unused */

    if (!wildmatch(s, "*.pkt", TRUE)) { /* only *.pkt */
        debug(5, "ignoring FLO entry: %s", s);
        return OK;
    }

    debug(5, "processing FLO entry: %s", s);
    if (cf_dos()) {
        s = cf_unix_xlate(s);
        debug(5, "converted to UNIX path: %s", s);
        if (!s)
            return ERROR;
    }

    return exec_ftn2rfc(s);
    /* the packet files will be removed by ftn2rfc */
}

/*
 * Process packets for all adresses
 */
int do_packets(void)
{
    char *name;
    Node *node;
    char *p;

    /*
     * If -n option not given, call ftn2rfc for each packet
     */
    if (!n_flag)
        /* Traverse all gate addresses */
        for (node = cf_addr_trav(TRUE); node; node = cf_addr_trav(FALSE)) {
            debug(5, "node=%s", znfp1(node));
            if (bink_bsy_create(node, NOWAIT) == ERROR) {
                fglog("%s busy, skipping", znfp1(node));
                continue;
            }

            /* Try *.?UT packets */
            if ((name = bink_find_out(node, NULL))) {
                debug(5, "OUT file=%s", name);
                exec_ftn2rfc(name);
            }

            /* Try *.?LO with *.pkt */
            if ((name = bink_find_flo(node, NULL))) {
                debug(5, "FLO file=%s", name);
                if (flo_open(node, FALSE) == ERROR) {
                    continue;
                }

                while ((p = flo_gets(line, sizeof(line)))) {
                    if (*p == ';' || *p == '~')
                        continue;
                    if (do_flo_line(p) == ERROR)
                        fglog("ERROR: processing line %s", p);
                    flo_mark();
                }

                flo_close(node, FALSE, TRUE);
            }

            bink_bsy_delete(node);

            tmps_freeall();
        }

    /*
     * If -x option given, call command in script[]
     */
    if (x_flag) {
        int ret;

        debug(2, "Command: %s", script);
        ret = run_system(script);
        debug(2, "Exit code=%d", ret);
        if (ret) {
            fglog("ERROR: can't exec command %s", script);
            return ERROR;
        }
        tmps_freeall();
    }

    return OK;
}

/*
 * Call ftn2rfc with name of packet file
 */
int exec_ftn2rfc(char *name)
{
    int ret;

    debug(2, "Packet: %s", name);

    BUF_COPY4(buffer, cmd, args, " ", name);
    debug(2, "Command: %s", buffer);

    ret = run_system(buffer);
    debug(2, "Exit code=%d", ret);
    if (ret) {
        fglog("ERROR: can't exec command %s", buffer);
        return ERROR;
    }

    return OK;
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
    exit(EX_USAGE);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options]\n\n", PROGRAM);
    fprintf(stderr, "\
options:  -n --no-toss                 don't call ftn2rfc for tossing\n\
          -x --exec-program SCRIPT     call SCRIPT after tossing\n\
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
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    char *exec = NULL;

    int option_index;
    static struct option long_options[] = {
        {"no-toss", 0, 0, 'n'}, /* Don't call ftn2rfc */
        {"exec-program", 1, 0, 'x'},    /* Exec script after tossing */

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"uplink-addr", 1, 0, 'u'}, /* Set FIDO uplink address */
        {0, 0, 0, 0}
    };

    log_program(PROGRAM);
    log_file("stderr");

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "nx:vhc:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** Local options *****/
        case 'n':
            n_flag = TRUE;
            break;
        case 'x':
            exec = optarg;
            x_flag = TRUE;
            break;

    /***** Common options *****/
        case 'v':
            args_add(" -v");
            verbose++;
            break;
        case 'h':
            usage();
            break;
        case 'c':
            args_add(" -c ");
            args_add(optarg);
            c_flag = optarg;
            break;
        case 'a':
            args_add(" -a ");
            args_add(optarg);
            a_flag = optarg;
            break;
        case 'u':
            args_add(" -u ");
            args_add(optarg);
            u_flag = optarg;
            break;
        default:
            short_usage();
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

    cf_i_am_a_gateway_prog();
    cf_debug();

    BUF_COPY3(cmd, cf_p_libexecdir(), "/", FTN2RFC);
    if (exec)
        BUF_EXPAND(script, exec);

    do_packets();

    exit_free();
    exit(0);
}
