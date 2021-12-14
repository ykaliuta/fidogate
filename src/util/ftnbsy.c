/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ftnbsy.c,v 4.10 2004/08/22 20:19:14 n0ll Exp $
 *
 * Command line interface to BinkleyTerm bsy files
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



#define PROGRAM 	"ftnbsy"
#define VERSION 	"$Revision: 4.10 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



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
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] Z:N/F.P ...\n\n", PROGRAM);
    fprintf(stderr, "\
options: -l --lock                    create bsy file\n\
         -t --test                    test bsy file\n\
         -u --unlock                  remove bsy file\n\
         -w --wait                    wait for removal of exisiting bsy file\n\
\n\
	 -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config NAME             read config file (\"\" = none)\n");
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, ret;
    int l_flag=TRUE, t_flag=FALSE, u_flag=FALSE, w_flag=NOWAIT;
    char *c_flag=NULL;
    Node nodes[MAXADDRESS];
    Node node;
    int i, n_nodes;
    char *s;

    int option_index;
    static struct option long_options[] =
    {
	{ "lock",         0, 0, 'l'},	/* Create bsy file */
	{ "test",         0, 0, 't'},	/* Test bsy file */
	{ "unlock",       0, 0, 'u'},	/* Remove bsy file */
	{ "wait",         0, 0, 'w'},	/* Wait */

	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ "config",       1, 0, 'c'},	/* Config file */
	{ 0,              0, 0, 0  }
    };

    log_program(PROGRAM);
    log_file("stderr");
    
    /* Init configuration */
    cf_initialize();


    while ((c = getopt_long(argc, argv, "ltuwvhc:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftnpack options *****/
        case 'l':
            l_flag = TRUE;
	    t_flag = FALSE;
            u_flag = FALSE;
            break;
        case 't':
            l_flag = FALSE;
	    t_flag = TRUE;
            u_flag = FALSE;
            break;
        case 'u':
            l_flag = FALSE;
	    t_flag = FALSE;
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

    if(optind >= argc)
    {
	short_usage();
	exit(EX_USAGE);
    }	

    /* Parse command line */
    n_nodes = 0;
    for(; optind<argc; optind++)
    {
	s = argv[optind];
	if(asc_to_node(s, &node, FALSE) == ERROR)
	{
	    fprintf(stderr, "%s: illegal FTN address %s", PROGRAM, s);
	    exit(EX_USAGE);
	}
	if(n_nodes >= MAXADDRESS)
	{
	    fprintf(stderr, "%s: too many FTN addresses", PROGRAM);
	    exit(EX_USAGE);
	}
	nodes[n_nodes++] = node;
    }


    ret = EXIT_OK;

    /* Test bsy files */
    if(t_flag)
    {
	for(i=0; i<n_nodes; i++)
	    if(nodes[i].zone != -1)
		if(bink_bsy_test(&nodes[i]))
		    ret = EXIT_BUSY;
	exit(ret);
    }
    
    /* Create bsy files */
    if(l_flag)
	for(i=0; i<n_nodes; i++)
	    if(nodes[i].zone != -1)
		if(bink_bsy_create(&nodes[i], w_flag) == ERROR)
		{
		    ret = w_flag==WAIT ? EXIT_ERROR : EXIT_BUSY;
		    node_invalid(&nodes[i]);
		}

    /* Delete bsy files */
    if(u_flag || ret!=EXIT_OK)
	for(i=0; i<n_nodes; i++)
	    if(nodes[i].zone != -1)
		if(bink_bsy_delete(&nodes[i]) == ERROR)
		    ret = EXIT_ERROR;

    
    exit(ret);
}
