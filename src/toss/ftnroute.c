/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ftnroute.c,v 4.32 2004/08/22 20:19:14 n0ll Exp $
 *
 * Route FTN NetMail/EchoMail
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

#include <fcntl.h>
#include <utime.h>
#include <signal.h>



#define PROGRAM 	"ftnroute"
#define VERSION 	"$Revision: 4.32 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



/*
 * Prototypes
 */
int	do_routing		(char *, FILE *, Packet *);
int	do_move			(char *, FILE *, PktDesc *);
int	do_cmd			(PktDesc *, Routing *, Node *);
int	do_packet		(char *, FILE *, Packet *, PktDesc *);
void	add_via			(Textlist *, Node *);
int	do_file			(char *);
void	prog_signal		(int);
void	short_usage		(void);
void	usage			(void);



/*
 * Command line options
 */
int g_flag = 0;

static char in_dir [MAXPATH];

static int severe_error = OK;		/* ERROR: exit after error */

static int signal_exit = FALSE;		/* Flag: TRUE if signal received */



/*
 * Route packet
 */
int do_routing(char *name, FILE *fp, Packet *pkt)
{
    PktDesc *desc;
    Routing *r;
    LNode *p;
    Node match;
    
    desc = parse_pkt_name(name, &pkt->from, &pkt->to);
    if(desc == NULL)
	return ERROR;

    debug(2, "Source packet: from=%s to=%s grade=%c type=%c flav=%c",
	  znfp1(&desc->from), znfp2(&desc->to),
	  desc->grade, desc->type, desc->flav);

    /*
     * Search for matching routing commands
     */
    for(r=routing_first; r; r=r->next)
	if(desc->type == r->type)
	    for(p=r->nodes.first; p; p=p->next)
		if(node_match(&desc->to, &p->node))
		{
		    match = p->node;
		    debug(4, "routing: type=%c cmd=%c flav=%c flav_new=%c "
			  "match=%s",
			  r->type, r->cmd, r->flav, r->flav_new,
			  s_znfp_print(&match, TRUE, TRUE)                   );
		    
		    if(do_cmd(desc, r, &match))
			goto ready;
		    
		    break;			/* Inner for loop */
		}

 ready:
    /*
     * Write contents of this packet to output packet
     */
    debug(2, "Target packet: from=%s to=%s grade=%c type=%c flav=%c",
	  znfp1(&desc->from), znfp2(&desc->to), 
	  desc->grade, desc->type, desc->flav);


    if(node_eq(&desc->to, &pkt->to))
	logit("packet for %s (%s)", znfp1(&pkt->to),
	    flav_to_asc(desc->flav)                              );
    else
	logit("packet for %s via %s (%s)", znfp1(&pkt->to),
	    znfp2(&desc->to), flav_to_asc(desc->flav)     );

    return desc->move_only ? do_move(name, fp, desc)
	                   : do_packet(name, fp, pkt, desc);
}



/*
 * Move packet instead of copying (for SENDMOVE command)
 */
int do_move(char *name, FILE *fp, PktDesc *desc)
{
    long n;
    
    fclose(fp);
    
    n = outpkt_sequencer();
    outpkt_outputname(buffer, pkt_get_outdir(),
		      desc->grade, desc->type, desc->flav, n, "pkt");

    debug(5, "Rename %s -> %s", name, buffer);
    if(rename(name, buffer) == ERROR)
    {
	logit("$ERROR: can't rename %s -> %s", name, buffer);
	return ERROR;
    }

    /* Set a/mtime to current time after renaming */
    if(utime(buffer, NULL) == ERROR)
    {
#ifndef __CYGWIN32__		/* Some problems with utime() here */
	logit("$WARNING: can't set time of %s", buffer);
#endif
#if 0
	return ERROR;
#endif
    }
    
    return OK;
}



/*
 * Exec routing command
 */
int do_cmd(PktDesc *desc, Routing *r, Node *match)
{
    int ret = FALSE;
    
    switch(r->cmd)
    {
    case CMD_SEND:
	if(desc->flav == FLAV_NORMAL)
	{
	    debug(4, "send %c %s", r->flav, znfp1(&desc->to));
	    desc->flav = r->flav;
	    desc->move_only = FALSE;
	    /*
	     * Special SEND syntax:
	     *   send 1:2/3  ==  route 1:2/3.0 1:2/3.*
	     */
	    if(match->point == EMPTY)
		desc->to.point = 0;
	    ret = TRUE;
	}
	break;
	
    case CMD_SENDMOVE:
	if(desc->flav == FLAV_NORMAL)
	{
	    debug(4, "sendmove %c %s", r->flav,
		  znfp1(&desc->to));
	    desc->flav = r->flav;
	    desc->move_only = TRUE;
	    ret = TRUE;
	}
	break;
	
    case CMD_ROUTE:
	if(desc->flav == FLAV_NORMAL)
	{
	    debug(4, "route %c %s -> %s", r->flav,
		  znfp1(&desc->to),
		  znfp2(&r->nodes.first->node) );
	    desc->flav = r->flav;
	    desc->to = r->nodes.first->node;
	    desc->move_only = FALSE;
	    ret = TRUE;
	}
	break;

    case CMD_CHANGE:
	if(desc->flav == r->flav)
	{
	    debug(4, "change %c -> %c %s", r->flav, r->flav_new,
		  znfp1(&desc->to)                );
	    desc->flav = r->flav_new;
	    desc->move_only = FALSE;
	    ret = TRUE;
	}
	break;
	
    case CMD_HOSTROUTE:
	if(desc->flav == FLAV_NORMAL)
	{
	    debug(4, "hostroute %c %s", r->flav,
		  znfp1(&desc->to) );
	    desc->flav = r->flav;
	    desc->to.node  = 0;
	    desc->to.point = 0;
	    desc->move_only = FALSE;
	    ret = TRUE;
	}
	break;
	
    case CMD_HUBROUTE:
	debug(2, "hubroute not yet implemented");
	break;
	
    case CMD_XROUTE:
	debug(2, "xroute not yet implemented");
	break;
	
    case CMD_BOSSROUTE:
	if(desc->flav == r->flav)
	{
	    debug(4, "route %c %s -> boss", r->flav,
		  znfp1(&desc->to)      );
	    desc->to.point = 0;
	    desc->move_only = FALSE;
	    ret = TRUE;
	}
	break;
	
    default:
	debug(2, "unknown routing command, strange");
	break;
    }

    /*
     * Set all -1 values to 0
     */
    if(desc->to.zone ==EMPTY || desc->to.zone ==WILDCARD)
	desc->to.zone  = 0;
    if(desc->to.net  ==EMPTY || desc->to.net  ==WILDCARD)
	desc->to.net   = 0;
    if(desc->to.node ==EMPTY || desc->to.node ==WILDCARD)
	desc->to.node  = 0;
    if(desc->to.point==EMPTY || desc->to.point==WILDCARD)
	desc->to.point = 0;

    return ret;
}



/*
 * Add our ^AVia line
 */
void add_via(Textlist *list, Node *gate)
{
    tl_appendf(list, "\001Via FIDOGATE/%s %s, %s\r\n",
		     PROGRAM, znfp1(gate),
		     date(DATE_VIA, NULL)  );
}



/*
 * Read and process packets, writing messages to output packet
 */
int do_packet(char *pkt_name, FILE *pkt_file, Packet *pkt, PktDesc *desc)
{
    Message msg;			/* Message header */
    Textlist tl;			/* Textlist for message body */
    MsgBody body;			/* Message body of FTN message */
    int type, ret;
    FILE *fp;
    
    /*
     * Initialize
     */
    tl_init(&tl);
    msg_body_init(&body);

    /*
     * Open output packet
     */
    cf_set_zone(desc->to.zone);
    fp = outpkt_open(&desc->from, &desc->to,
		      desc->grade, desc->type, desc->flav, FALSE);
    if(fp == NULL)
    {
	fclose(pkt_file);
	TMPS_RETURN(ERROR);
    }
    
    /*
     * Read message from input packet and write to output packet
     */
    type = pkt_get_int16(pkt_file);
    ret  = OK;
    
    while(type == MSG_TYPE)
    {
	/*
	 * Read message header
	 */
	msg.node_from = pkt->from;
	msg.node_to   = pkt->to;
	if(pkt_get_msg_hdr(pkt_file, &msg) == ERROR)
	{
	    logit("ERROR: reading input packet");
	    ret = ERROR;
	    break;
	}
	
	/*
	 * Read message body
	 */
	type = pkt_get_body(pkt_file, &tl);
	if(type == ERROR)
	{
	    logit("ERROR: reading input packet");
	    ret = ERROR;
	    break;
	}

	/*
	 * Parse message body
	 */
	if( msg_body_parse(&tl, &body) == -2 )
	    logit("ERROR: parsing message body");

	if(body.area == NULL)
	{
	    /*
	     * NetMail
	     */
	    /* Retrieve address from kludges */
	    kludge_pt_intl(&body, &msg, TRUE);
	    /* Write message header and body */
	    if( pkt_put_msg_hdr(fp, &msg, TRUE) != OK )
	    {
		ret = severe_error=ERROR;
		break;
	    }
	    if( msg_put_msgbody(fp, &body, TRUE) != OK )
	    {
		ret = severe_error=ERROR;
		break;
	    }
	}
	else
	{
	    /*
	     * EchoMail
	     */
	    /* Write message header and body */
	    if( pkt_put_msg_hdr(fp, &msg, FALSE) != OK )
	    {
		ret = severe_error=ERROR;
		break;
	    }
	    if( msg_put_msgbody(fp, &body, TRUE) != OK )
	    {
		ret = severe_error=ERROR;
		break;
	    }
	}

	/*
	 * Exit if signal received
	 */
	if(signal_exit)
	{
	    ret = severe_error=ERROR;
	    break;
	}

	tmps_freeall();
    } /**while(type == MSG_TYPE)**/


    if(fclose(pkt_file) == ERROR)
    {
	logit("$ERROR: can't close packet %s", pkt_name);
	TMPS_RETURN(severe_error=ERROR);
    }

    if(ret == OK)
	if (unlink(pkt_name)) {
	    logit("$ERROR: can't unlink packet %s", pkt_name);
	    TMPS_RETURN(ERROR);
	}

    TMPS_RETURN(ret);
}



/*
 * Process one packet file
 */
int do_file(char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;

    /*
     * Open packet and read header
     */
    pkt_file = fopen(pkt_name, R_MODE);
    if(!pkt_file) {
	logit("$ERROR: can't open packet %s", pkt_name);
	TMPS_RETURN(ERROR);
    }
    if(pkt_get_hdr(pkt_file, &pkt) == ERROR)
    {
	logit("ERROR: reading header from %s", pkt_name);
	TMPS_RETURN(ERROR);
    }

    /*
     * Route it
     */
    if(do_routing(pkt_name, pkt_file, &pkt) == ERROR) 
    {
	logit("ERROR: in processing %s", pkt_name);
	TMPS_RETURN(ERROR);
    }

    TMPS_RETURN(OK);
}



/*
 * Function called on SIGINT
 */
void prog_signal(int signum)
{
    char *name = "";

    signal_exit = TRUE;
    
    switch(signum)
    {
    case SIGHUP:
	name = " by SIGHUP";  break;
    case SIGINT:
	name = " by SIGINT";  break;
    case SIGQUIT:
	name = " by SIGQUIT"; break;
    default:
	name = "";            break;
    }

    logit("KILLED%s: exit forced", name);
}



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] [packet ...]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] [packet ...]\n\n", PROGRAM);
    fprintf(stderr, "\
options: -g --grade G                 processing grade\n\
         -I --in-dir NAME             set input packet directory\n\
         -O --out-dir NAME            set output packet directory\n\
         -l --lock-file               create lock file while processing\n\
         -r --routing-file NAME       read routing file\n\
         -M --maxopen N               set max # of open packet files\n\
\n\
	 -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config NAME             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n");

    exit(0);
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, ret;
    char *p;
    int l_flag = FALSE;
    char *I_flag=NULL, *O_flag=NULL, *r_flag=NULL, *M_flag=NULL;
    char *c_flag=NULL;
    char *a_flag=NULL, *u_flag=NULL;
    char *pkt_name;
    char pattern[16];
    
    int option_index;
    static struct option long_options[] =
    {
	{ "grade",        1, 0, 'g'},	/* grade */
	{ "in-dir",       1, 0, 'I'},	/* Set inbound packets directory */
	{ "lock-file",    0, 0, 'l'},	/* Create lock file while processing */
	{ "out-dir",      1, 0, 'O'},	/* Set packet directory */
	{ "routing-file", 1, 0, 'r'},	/* Set routing file */
	{ "maxopen",      1, 0, 'M'},	/* Set max # open packet files */

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


    while ((c = getopt_long(argc, argv, "g:O:I:lr:M:vhc:a:u:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftnroute options *****/
	case 'g':
	    g_flag = *optarg;
	    break;
	case 'I':
	    I_flag = optarg;
	    break;
        case 'l':
	    l_flag = TRUE;
            break;
	case 'O':
	    O_flag = optarg;
	    break;
	case 'r':
	    r_flag = optarg;
	    break;
	case 'M':
	    M_flag = optarg;
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
	case 'a':
	    a_flag = optarg;
	    break;
	case 'u':
	    u_flag = optarg;
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

    /*
     * Process config options
     */
    if(a_flag)
	cf_set_addr(a_flag);
    if(u_flag)
	cf_set_uplink(u_flag);

    cf_debug();
    
    /*
     * Process optional config statements
     */
    if(!M_flag && (p = cf_get_string("MaxOpenFiles", TRUE)))
    {
	debug(8, "config: MaxOpenFiles %s", p);
	M_flag = p;
    }
    if(M_flag)
	outpkt_set_maxopen(atoi(M_flag));

    /*
     * Process local options
     */
    BUF_EXPAND(in_dir, I_flag ? I_flag : DEFAULT_TOSS_TOSS);
    pkt_outdir(O_flag ? O_flag : DEFAULT_TOSS_ROUTE, NULL);

    routing_init(r_flag ? r_flag : cf_p_routing() );
    passwd_init();
    
    /* Install signal/exit handlers */
    signal(SIGHUP,  prog_signal);
    signal(SIGINT,  prog_signal);
    signal(SIGQUIT, prog_signal);


    ret = EXIT_OK;
    
    if(optind >= argc)
    {
	BUF_COPY(pattern, "????????.pkt");
	if(g_flag)
	    pattern[0] = g_flag;
	
	/* process packet files in directory */
	dir_sortmode(DIR_SORTMTIME);
	if(dir_open(in_dir, pattern, TRUE) == ERROR)
	{
	    logit("$ERROR: can't open directory %s", in_dir);
	    exit(EX_OSERR);
	}

	/* Lock file */
	if(l_flag)
	    if(lock_program(PROGRAM, FALSE) == ERROR)
		/* Already busy */
		exit(EXIT_BUSY);
	
	for(pkt_name=dir_get(TRUE); pkt_name; pkt_name=dir_get(FALSE))
	    if(do_file(pkt_name) == ERROR)
	    {
		ret = EXIT_ERROR;
		break;
	    }
	

	dir_close();

	/* Lock file */
	if(l_flag)
	    unlock_program(PROGRAM);
    }
    else
    {
	/* Lock file */
	if(l_flag)
	    if(lock_program(PROGRAM, FALSE) == ERROR)
		/* Already busy */
		exit(EXIT_BUSY);
	
	/*
	 * Process packet files on command line
	 */
	for(; optind<argc; optind++)
	    if(do_file(argv[optind]) == ERROR)
	    {
		ret = EXIT_ERROR;
		break;
	    }

	/* Lock file */
	if(l_flag)
	    unlock_program(PROGRAM);
    }
    
    outpkt_close();

    exit(ret);
}
