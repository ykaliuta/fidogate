/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ftnoutpkt.c,v 4.9 2004/08/22 20:19:14 n0ll Exp $
 *
 * Create packet in OUTPKT with message from stdin.
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



#define PROGRAM		"ftnoutpkt"
#define VERSION		"$Revision: 4.9 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



/*
 * Send one message
 */
int do_addr(FTNAddr *from, FTNAddr *to, char *subj, Textlist *tl)
{
    Message msg;
    
    msg.node_from = from->node;
    msg.node_to   = to->node;
    node_invalid(&msg.node_orig);
    msg.attr      = 0;
    msg.cost      = 0;
    msg.date      = time(NULL);
    BUF_COPY(msg.name_from, from->name);
    BUF_COPY(msg.name_to, to->name);
    BUF_COPY(msg.subject, subj);
    msg.area      = NULL;
    
    TMPS_RETURN(outpkt_netmail(&msg, tl, PROGRAM));
}



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] 'User Name @ Z:N/F.P' ...\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
    exit(EX_USAGE);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] 'User Name Z:N/F.P' ...\n\n", PROGRAM);
    fprintf(stderr, "\
options:  -f --from NAME@Z:N/F.P       set from FTN address\n\
          -s --subject SUBJECT         set subject of message\n\
          -O --out-dir DIR             set output directory\n\
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
    char *f_flag=NULL, *s_flag=NULL;
    char *O_flag=NULL;
    char *c_flag=NULL;
    char *a_flag=NULL, *u_flag=NULL;
    FTNAddr to, from;
    Textlist tl;
    
    int option_index;
    static struct option long_options[] =
    {
	{ "from",         1, 0, 'f'},	/* From */
	{ "subject",      1, 0, 's'},	/* Subject */
	{ "out-dir",      1, 0, 'O'},	/* Output dir */

	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ "config",       1, 0, 'c'},	/* Config file */
	{ "addr",         1, 0, 'a'},	/* Set FIDO address */
	{ "uplink-addr",  1, 0, 'u'},	/* Set FIDO uplink address */
	{ 0,              0, 0, 0  }
    };

    log_program(PROGRAM);
    
    /* Init configuration */
    cf_initialize();


    while ((c = getopt_long(argc, argv, "f:s:O:vhc:a:u:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	case 'f':
	    f_flag = optarg;
	    break;
	case 's':
	    s_flag = optarg;
	    break;
	case 'O':
	    O_flag = optarg;
	    break;

	/***** Common options *****/
	case 'v':
	    verbose++;
	    break;
	case 'h':
	    usage();
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
	    break;
	}


    if(optind >= argc) 
	short_usage();
    
    /*
     * Read config file
     */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /*
     * Process config options
     */
    if(a_flag)
	cf_set_addr(a_flag);
    if(u_flag)
	cf_set_uplink(u_flag);
    if(f_flag)
    {
	from = ftnaddr_parse(f_flag);
	if(from.node.zone == INVALID) 
	{
	    fprintf(stderr, "%s: illegal address -f %s\n", PROGRAM, f_flag);
	    exit(EX_USAGE);
	}
    }
    else
    {
	BUF_COPY(from.name, "nobody");
	node_clear(&from.node);
    }
    if(!s_flag)
	s_flag = "(no subject)";
    if(O_flag)
	pkt_outdir(O_flag, NULL);
    else
	pkt_outdir(DEFAULT_OUTPKT, NULL);

    cf_debug();

    /* Read stdin, put into textlist */
    tl_init(&tl);
    while(fgets(buffer, BUFFERSIZE, stdin)) {
	strip_crlf(buffer);
	tl_append(&tl, buffer);
    }
    
    /* FTN to addresses */
    for(; optind<argc; optind++)
    {
	to = ftnaddr_parse(argv[optind]);
	if(to.node.zone == INVALID) 
	{
	    fprintf(stderr, "%s: illegal address %s\n", PROGRAM, argv[optind]);
	    exit(EX_USAGE);
	}

	do_addr(&from, &to, s_flag, &tl);
    }
    
    exit(0);
}
