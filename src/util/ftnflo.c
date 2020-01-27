/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Run script for every entry in FLO file for node
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

#define PROGRAM		"ftnflo"
#define CONFIG		DEFAULT_CONFIG_MAIN

/*
 * Command for script
 */
static char script[MAXPATH];

/*
 * Command line options
 */
int l_flag = FALSE;
int n_flag = FALSE;
int x_flag = FALSE;

/*
 * Process FLO file of one node
 */
int do_flo(Node * node)
{
    int mode, ret, del_ok, fd;
    char *line;
    char buf[MAXPATH];

    del_ok = TRUE;

    /* Open FLO file */
    if (flo_open(node, TRUE) == ERROR) {
/*	fglog("nothing on hold for %s", znfp1(node)); */
        return OK;
    }

    /* Read FLO entries */
    while ((line = flo_gets(buffer, sizeof(buffer)))) {
        if (*line == '~')
            continue;
        mode = ' ';
        if (*line == '^' || *line == '#')
            mode = *line++;
        if (cf_dos())
            line = cf_unix_xlate(line);

        debug(2, "FLO entry: %c %s", mode, line);

        if (l_flag) {
            printf("%10ld    %c %s\n", check_size(line), mode, line);
        }
        if (x_flag) {
            /* Command */
            str_printf(buf, sizeof(buf), script, line);
            debug(2, "Command: %s", buf);

            if (!n_flag) {
                ret = run_system(buf);
                debug(2, "Exit code=%d", ret);
                if (ret) {
                    fglog("ERROR: running command %s", buf);
                    flo_close(node, TRUE, FALSE);
                    return ERROR;
                }

                /* According to mode ... */
                switch (mode) {
                case '^':
                    /* ... delete */
                    debug(2, "Removing %s", line);
                    if (unlink(line) == ERROR)
                        fglog("ERROR: can't remove %s", line);
                    break;

                case '#':
                    /* ... truncate */
                    debug(2, "Truncating %s", line);
#if 0                           /* truncate() is not a POSIX function */
                    if (truncate(line, 0) == ERROR)
                        fglog("ERROR: can't truncate %s", line);
#endif
                    if ((fd = open(line, O_WRONLY | O_TRUNC)) == ERROR)
                        fglog("ERROR: can't truncate %s", line);
                    close(fd);
                    break;
                }

                /* Mark as sent */
                flo_mark();
            }
        } else
            del_ok = FALSE;
    }

    /* Close and delete if completed FLO file */
    flo_close(node, TRUE, del_ok);

    return OK;
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] Z:N/F.P ...\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] Z:N/F.P ...\n\n", PROGRAM);
    fprintf(stderr, "\
options:  -B --binkley NAME            set Binkley-style outbound directory\n\
          -l --list                    list FLO entries\n\
          -n --no-delete               don't delete/truncate FLO entry\n\
          -x --exec SCRIPT             execute SCRIPT for every FLO entry\n\
\n\
          -v --verbose                 more verbose\n\
	  -h --help                    this help\n\
          -c --config name             read config file (\"\" = none)\n");
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    char *B_flag = NULL;
    char *c_flag = NULL;
    Node node;

    int option_index;
    static struct option long_options[] = {
        {"binkley", 1, 0, 'B'}, /* Binkley outbound base dir */
        {"exec", 1, 0, 'x'},    /* Execute script */
        {"list", 1, 0, 'l'},    /* List entries */
        {"no-delete", 0, 0, 'n'},   /* Don't delete/truncate entries */

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {0, 0, 0, 0}
    };

    log_program(PROGRAM);
    log_file("stderr");

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "B:x:lnvhc:",
                            long_options, &option_index)) != EOF)
        switch (c) {
        case 'B':
            B_flag = optarg;
            break;
        case 'l':
            l_flag = TRUE;
            break;
        case 'n':
            n_flag = TRUE;
            break;
        case 'x':
            x_flag = TRUE;
            BUF_COPY(script, optarg);
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
        default:
            short_usage();
            return EX_USAGE;
            break;
        }

    /* Default: -l */
    if (!l_flag && !x_flag)
        l_flag = TRUE;

    /*
     * Read config file
     */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /*
     * Process config options
     */
    if (B_flag)
        cf_s_btbasedir(B_flag);

    cf_debug();

    /*
     * Process following command line arguments
     */
    if (optind >= argc) {
        short_usage();
        return EX_USAGE;
    }

    /* Nodes */
    for (; optind < argc; optind++) {
        if (asc_to_node(argv[optind], &node, FALSE) == ERROR) {
            fprintf(stderr, "%s: not an FTN address: %s\n",
                    PROGRAM, argv[optind]);
            continue;
        }
        do_flo(&node);
    }

    exit_free();
    return 0;
}
