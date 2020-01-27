/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * setuid frontend for rfc2ftn, limiting options for security reasons
 *
 *****************************************************************************
 * Copyright (C) 1990-1999
 *  _____ _____
 * |     |___  |   Martin Junius             FIDO:      2:2452/110
 * | | | |   | |   Radiumstr. 18             Internet:  mj@fido.de
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
#include <string.h>

#define PROGRAM		"ftnnews"
#define CONFIG		DEFAULT_CONFIG_GATE

#define RFC2FTN		"rfc2ftn"

#define MAXARGS		64

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
options: -b --news-batch              (passed on as -b)\n\
         -n --news-mode               (passed on as -n)\n\
         -f --batch-file FILE         (passed on as -f FILE)\n\
	 -m --maxmsg N                (passed on as -m N)\n\
\n\
	 -h --help                    this help\n");

    exit(0);
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, n;
    int b_flag = FALSE;
    int n_flag = FALSE;
    char *f_flag = NULL;
    char *m_flag = NULL;
    char cmd[MAXPATH];
    char *args[MAXARGS];

    int option_index;
    static struct option long_options[] = {
        {"news-batch", 0, 0, 'b'},  /* Process news batch */
        {"news-mode", 0, 0, 'n'},   /* Set news mode */
        {"batch-file", 1, 0, 'f'},  /* Read batch file for list of articles */
        {"maxmsg", 1, 0, 'm'},  /* New output packet after N msgs */

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

    while ((c = getopt_long(argc, argv, "bnf:m:h",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftnmail options *****/
        case 'b':
            b_flag = TRUE;
            break;
        case 'n':
            n_flag = TRUE;
            break;
        case 'f':
            f_flag = optarg;
            break;
        case 'm':
            m_flag = optarg;
            break;

    /***** common options *****/
        case 'h':
            usage();
            exit(0);
            break;
        default:
            short_usage();
            exit(EX_USAGE);
            break;
        }

    /* complete path of rfc2ftn */
    BUF_COPY3(cmd, cf_p_libexecdir(), "/", RFC2FTN);

    /* build args[] */
    n = 0;
    args[n++] = RFC2FTN;
    if (b_flag) {
        args[n++] = "-b";
    }
    if (n_flag) {
        args[n++] = "-n";
    }
    if (f_flag) {
        args[n++] = "-f";
        args[n++] = f_flag;
    }
    if (m_flag) {
        args[n++] = "-m";
        args[n++] = m_flag;
    }

    args[n++] = NULL;

#if 0
    /* debug */
    printf("cmd=%s\n", cmd);
    for (n = 0; args[n]; n++)
        printf("args[%d]=%s\n", n, args[n]);
    exit(0);
#endif

    /* exec */
    if (execv(cmd, args) == ERROR)
        fglog("$can't exec %s", cmd);

    /* Only reached if error */
    exit(1);

    /**NOT REACHED**/
    return 1;
}
