/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * setuid frontend for rfc2ftn, limiting options for security reasons
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

#define PROGRAM		"ftnmail"
#define CONFIG		DEFAULT_CONFIG_GATE

#define RFC2FTN		"rfc2ftn"

#define MAXARGS		64

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] user@domain ...\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
    exit(EX_USAGE);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] user@domain ...\n\n", PROGRAM);
    fprintf(stderr, "\
options: -a --addr                    (passed on as -a AND -u)\n\
         -i --ignore-hosts            (passed on as -i)\n\
         -O --out-dir  DIR            (passed on as -O %%S/DIR)\n\
\n\
	 -h --help                    this help\n");

    exit(0);
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, n;
    char *a_flag = NULL;
    int i_flag = FALSE;
    char *O_flag = NULL;
    char *s;
    char cmd[MAXPATH];
    char O_opt[MAXPATH];
    char *args[MAXARGS];

    int option_index;
    static struct option long_options[] = {
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"ignore-hosts", 0, 0, 'i'},    /* Do not bounce unknown hosts */
        {"out-dir", 1, 0, 'O'}, /* Set packet directory */

        {"help", 0, 0, 'h'},    /* Help */
        {0, 0, 0, 0}
    };

#ifdef SIGPIPE
    /* ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
#endif

    log_program(PROGRAM);

    /* init configuration */
    cf_initialize();
    cf_read_config_file(CONFIG);

    while ((c = getopt_long(argc, argv, "a:iO:hv",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftnmail options *****/
        case 'a':
            a_flag = optarg;
            break;
        case 'i':
            i_flag = TRUE;
            break;
        case 'O':
            O_flag = optarg;
            break;

    /***** common options *****/
        case 'h':
            usage();
            exit(0);
            break;
        case 'v':
            verbose++;
            break;
        default:
            short_usage();
            exit(EX_USAGE);
            break;
        }

    /* complete path of rfc2ftn */
    BUF_COPY3(cmd, cf_p_libexecdir(), "/", RFC2FTN);

    /* check -O option */
    if (O_flag) {
        s = O_flag;
        if (*s == '/' || *s == '%') {
            fprintf(stderr, "%s:-O %s: must not start with `%c'\n",
                    PROGRAM, O_flag, s[0]);
            exit_free();
            exit(EX_USAGE);
        }
        while (*s) {
            if (s[0] == '.' && s[1] == '.') {
                fprintf(stderr, "%s:-O %s: must not contain `..'\n",
                        PROGRAM, O_flag);
                exit_free();
                exit(EX_USAGE);
            }
            if (s[0] == '/' && s[1] == '/') {
                fprintf(stderr, "%s:-O %s: must not contain `//'\n",
                        PROGRAM, O_flag);
                exit_free();
                exit(EX_USAGE);
            }
            s++;
        }
    }

    /* build args[] */
    n = 0;
    args[n++] = RFC2FTN;
    if (a_flag) {
        args[n++] = "-a";
        args[n++] = a_flag;
        args[n++] = "-u";
        args[n++] = a_flag;
    }
    if (i_flag) {
        args[n++] = "-i";
    }
    if (O_flag) {
        BUF_COPY2(O_opt, "%S/", O_flag);
        args[n++] = "-O";
        args[n++] = O_opt;
    }
    args[n++] = "--";

    while (n < MAXARGS - 1 && optind < argc)
        args[n++] = argv[optind++];

    args[n++] = NULL;

#if 0
    /* debug */
    printf("cmd=%s\n", cmd);
    for (n = 0; args[n]; n++)
        printf("args[%d]=%s\n", n, args[n]);
    exit_free();
    exit(0);
#endif

    /* exec */
    if (execv(cmd, args) == ERROR)
        fglog("$can't exec %s", cmd);

    /* Only reached if error */
    exit_free();
    exit(1);
}
