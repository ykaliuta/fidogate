/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ftnlock.c,v 4.11 2004/08/22 20:19:14 n0ll Exp $
 *
 * Command line interface to lock files in SPOOLDIR/locks
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



#define PROGRAM 	"ftnlock"
#define VERSION 	"$Revision: 4.11 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] [name] [id]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] [name] [id]\n\n", PROGRAM);
    fprintf(stderr, "\
options: -l --lock                    create lock file\n\
         -u --unlock                  remove lock file\n\
         -w --wait                    wait while creating lock file\n\
\n\
	 -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config NAME             read config file (\"\" = none)\n");
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, ret;
    int l_flag=TRUE, u_flag=FALSE, w_flag=NOWAIT;
    char *c_flag=NULL;
    char *name, *id;
    
    int option_index;
    static struct option long_options[] =
    {
	{ "lock",         0, 0, 'l'},	/* Create lock file */
	{ "unlock",       0, 0, 'u'},	/* Remove lock file */
	{ "wait",         0, 0, 'w'},	/* Wait while creating */

	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ "config",       1, 0, 'c'},	/* Config file */
	{ 0,              0, 0, 0  }
    };

    log_program(PROGRAM);
    log_file("stderr");
    
    /* Init configuration */
    cf_initialize();


    while ((c = getopt_long(argc, argv, "luwvhc:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftnpack options *****/
        case 'l':
            l_flag = TRUE;
            u_flag = FALSE;
            break;
        case 'u':
            l_flag = FALSE;
            u_flag = TRUE;
            break;
        case 'w':
            w_flag = WAIT;
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
	default:
	    short_usage();
	    exit(EX_USAGE);
	    break;
	}

    /*
     * Read config file
     */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    cf_debug();


    ret = EXIT_OK;

    name = optind>=argc ? PROGRAM  : argv[optind];
    optind++;
    id   = optind>=argc ? "-none-" : argv[optind];
    
    /* Lock file */
    if(l_flag)
	if(lock_program_id(name, w_flag, id) == ERROR)
	    /* Already busy */
	    ret = EXIT_BUSY;
	
    /* Unlock file */
    if(u_flag)
	if(unlock_program(name) == ERROR)
	    /* Error */
	    ret = EXIT_ERROR;
    
    exit(ret);
}
