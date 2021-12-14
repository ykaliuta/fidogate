/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: ftnafmail.c,v 1.10 2004/08/22 20:19:10 n0ll Exp $
 *
 * setuid frontend for ftnaf, limiting options for security reason
 *
 *****************************************************************************
 * Copyright (C) 1990-2004
 *  _____ _____
 * |     |___  |   Martin Junius             <mj.at.n0ll.dot.net>
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



#define PROGRAM		"ftnafmail"
#define VERSION		"$Revision: 1.10 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



/*
 * ftnaf program
 */
char cmd[MAXPATH] = "";

/*
 * Args for running Areafix
 */
char *args_areafix[] = { "ftnaf", "-m", NULL };

/*
 * Args for running Filefix
 */
char *args_filefix[] = { "ftnaf", "-m", "-F", NULL };



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
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options]\n\n", PROGRAM);
    fprintf(stderr, "\
options: -F --filefix                 run as Filefix program (FAREAS.BBS)\n\
\n\
	 -h --help                    this help\n");

    exit(0);
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    int filefix = FALSE;
    
    int option_index;
    static struct option long_options[] =
    {
	{ "filefix",      0, 0, 'F'},

	{ "help",         0, 0, 'h'},	/* Help */
	{ 0,              0, 0, 0  }
    };

#ifdef SIGPIPE
    /* Ignore SIGPIPE */
    signal(SIGPIPE, SIG_IGN);
#endif

    log_file("stderr");
    log_program(PROGRAM);
    
    while ((c = getopt_long(argc, argv, "Fh",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftnafmail options *****/
	case 'F':
	    filefix = TRUE;
	    break;
	    
	/***** Common options *****/
	case 'h':
	    usage();
	    exit(0);
	    break;
	default:
	    short_usage();
	    exit(EX_USAGE);
	    break;
	}

    /* Run ftnaf */
    BUF_COPY(cmd, cf_p_libdir());
    BUF_APPEND(cmd, "/ftnaf");
    if( execv(cmd, filefix ? args_filefix : args_areafix) == ERROR )
	logit("$can't exec %s", cmd);
	
    /* Only reached if error */
    exit(1);
}
