/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id$
 *
 * Active group
 *
 *****************************************************************************
 * Copyright (C) 2001-2002
 * 
 *    Dmitry Fedotov            FIDO:      2:5030/1229
 *				Internet:  dyff@users.sourceforge.net
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

#define PROGRAM 	"confval"
#define VERSION 	"$Revision$"
#define CONFIG		DEFAULT_CONFIG_MAIN


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] \n\n", PROGRAM);
    fprintf(stderr, "\
	  -p --param VARIABLE          value of read param\n\
          -v --verbose                 more verbose\n\
	  -h --help                    this help\n\
          -c --config name             read config file (\"\" = none)\n");
    exit(0);
}


int main(int argc, char **argv)
{
    int c;
    char *p;
    char *c_flag = NULL;
    char *p_flag = NULL;

    int option_index;
    static struct option long_options[] =
    {
	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ "config",       1, 0, 'c'},	/* Config file */
	{ "param",        1, 0, 'p'},
	{ 0,              0, 0, 0  }
    };
	
    /* Set log and debug output */
    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();
    
    while ((c = getopt_long(argc, argv, "vhc:p:", long_options, &option_index)) != EOF)
	switch (c) {
	case 'h':
	    usage();
	    break;
	case 'c':
	    c_flag = optarg;
	    break;
	case 'v':
	    verbose++;
	    break;
	case 'p':
	    p_flag = optarg;
	    break;
	default:
	    usage();
	    break;
	};

    if(argc == 1)
        usage();
	    
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    if ( p_flag )
    {
	int IsExist = 0;
	for(p = cf_get_string(p_flag, TRUE); p && *p; p = cf_get_string(p_flag, FALSE))
	{
	    IsExist = 1;
	    fprintf(stderr, "fidogate_%s=\"%s\"\n", p_flag, p);
	}
	if(!IsExist)
	    fprintf(stderr, "variable %s does not specifed\n", p_flag);
    }
    exit_free();
    exit(0);
}
