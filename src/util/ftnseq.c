/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: ftnseq.c,v 4.10 2004/08/22 20:19:14 n0ll Exp $
 *
 * Command line interface to sequencer files
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


#define PROGRAM 	"ftnseq"
#define VERSION 	"$Revision: 4.10 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



/*
 * Prototypes
 */
void	short_usage		(void);
void	usage			(void);



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] SEQ\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] SEQ\n\n", PROGRAM);
    fprintf(stderr, "\
options: -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n");
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    char *c_flag=NULL;
    long n;
    
    int option_index;
    static struct option long_options[] =
    {
	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ "config",       1, 0, 'c'},	/* Config file */
	{ 0,              0, 0, 0  }
    };

    log_program(PROGRAM);
    
    /* Init configuration */
    cf_initialize();


    while ((c = getopt_long(argc, argv, "vhc:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
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

    cf_read_config_file(c_flag ? c_flag : CONFIG);
    cf_debug();
    
    /*
     * Process command line file args
     */
    if( (argc - optind) != 1 )
    {
	short_usage();
	exit(EX_USAGE);
    }

    n = sequencer_nx(argv[optind], FALSE);
    
    n == -1L ? printf("ERROR\n") : printf("%ld\n", n) ;
    
    exit(EX_OK);
}
