/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: ftnafutil.c,v 1.16 2004/08/22 20:19:10 n0ll Exp $
 *
 * Utility program for Areafix.
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



#define PROGRAM		"ftnafutil"
#define VERSION		"$Revision: 1.16 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



/*
 * Prototypes
 */
int	areafix_init		(int);
void	areafix_auth_cmd	(void);
char   *areafix_areasbbs	(void);
void	areafix_set_areasbbs	(char *name);
char   *areafix_name		(void);
Node   *areafix_auth_node	(void);
int	areafix_do		(Node *node, char *subj, Textlist*, Textlist*);
int	rewrite_areas_bbs	(void);
int	areafix_do_cmd		(Node *, char *, Textlist *);
void	areafix_set_changed	(void);

int	do_mail			(Node *, char *, char *, Passwd *);
int	do_areasbbs		(int);
int	do_cmd			(char **);
void	short_usage		(void);
void	usage			(void);



/*
 * Global vars
 */
static int n_flag = FALSE;



/*
 * Subscribe to an area
 */
int do_mail(Node *node, char *area, char *s, Passwd *pwd)
{
    Textlist outbody;
    Message outmsg;
    char *to;

    if(!pwd || !pwd->passwd)
    {
	logit("ERROR: no uplink password for %s, can't send request", 
	    znfp1(node));
	return ERROR;
    }
    to = pwd->args && *pwd->args ? pwd->args : areafix_name();

    /* Send Areafix message */
    tl_init(&outbody);
    cf_set_zone(node->zone);

    outmsg.node_from = cf_n_addr();
    outmsg.node_to   = *node;
    outmsg.attr      = 0;
    outmsg.cost      = 0;
    outmsg.date      = time(NULL);
    BUF_COPY  (outmsg.name_from, areafix_name());
    BUF_APPEND(outmsg.name_from, " Daemon"     );
    BUF_COPY  (outmsg.name_to  , to            );
    BUF_COPY  (outmsg.subject  , pwd->passwd   );
    outmsg.area      = NULL;

    tl_appendf(&outbody, "%s%s", s, area);
    
    return outpkt_netmail(&outmsg, &outbody, PROGRAM);
}



/*
 * Process areas.bbs
 */
#define DO_DELETE	 0
#define DO_SUBSCRIBE	 1
#define DO_UNSUBSCRIBE	 2
#define DO_LISTGWLINKS 3

int do_areasbbs(int cmd)
{
    AreasBBS *p, *pl;
    LON *lon;
    Node *uplink;
    int n;
    char *state;
    Passwd *pwd;
    
    pl = NULL;
    p  = areasbbs_first();
    while(p)
    {
	lon    = &p->nodes;
	uplink = lon->first ? &lon->first->node : NULL;
	n      = lon->size - 1;
	state  = p->state ? p->state : "";
	
	debug(3, "processing area %s: state=%s uplink=%s #dl=%d",
	      p->area, state, uplink ? znfp1(uplink) : "(none)", n   );
	
	switch(cmd)
	{
	case DO_DELETE:
	    if(!uplink)
	    {
		logit("area %s: no uplink, deleting", p->area);
		if(!n_flag) 
		{
		    areasbbs_remove(p, pl);
		    areafix_set_changed();
		    p = p->next;
		    continue;
		}
	    }
	    break;
	    
	case DO_SUBSCRIBE:
	    if(uplink && n>0 && strchr(state, 'U'))
	    {
		if(! (pwd = passwd_lookup("uplink", uplink)) )
		    break;
		logit("area %s: #dl=%d state=%s, subscribing at uplink %s",
		      p->area, n, state, znfp1(uplink));
		if(!n_flag && do_mail(uplink, p->area, "+", pwd) != ERROR)
		{
		    /**FIXME: remove "U", add "S"**/
		    p->state = strsave("S");
		    areafix_set_changed();
		}
	    }
	    break;
	    
	case DO_UNSUBSCRIBE:
	    if(uplink && n<=0 && strchr(state, 'S'))
	    {
		if(! (pwd = passwd_lookup("uplink", uplink)) )
		    break;
		logit("area %s: #dl=%d state=%s, unsubscribing at uplink %s",
		      p->area, n, state, znfp1(uplink));
		if(!n_flag && do_mail(uplink, p->area, "-", pwd) != ERROR)
		{
		    /**FIXME: add "U", remove "S"**/
		    p->state = strsave("U");
		    areafix_set_changed();
		}
	    }
	    break;

	case DO_LISTGWLINKS:
	    n = lon->size;
	    cf_set_zone(p->zone);
    	    debug(5, "area %s, LON size %d, zone %d", p->area, n, p->zone);
	    if(uplink && node_eq(uplink, cf_addr())) {
		/* 1st downlink is gateway, don't include in # of downlinks */
		n--;
		debug(5, "     # downlinks is %d", n);
	    }
	    printf("%s %s %d\n",
		   p->area, uplink ? znfp1(uplink) : "-", n);
	    break;
	    
	default:
	    return ERROR;
	    /**NOT REACHED**/
	    break;
	}
	
	/* Next */
	pl = p;
	p  = p->next;

	tmps_freeall();
    }

    return OK;
}



/*
 * Do command
 */
int do_cmd(char *cmd[])
{
    if(cmd[0])
    {
	if     (strieq(cmd[0], "delete"))
	    do_areasbbs(DO_DELETE);
	else if(strieq(cmd[0], "subscribe"))
	    do_areasbbs(DO_SUBSCRIBE);
	else if(strieq(cmd[0], "unsubscribe"))
	    do_areasbbs(DO_UNSUBSCRIBE);
	else if(strieq(cmd[0], "listgwlinks"))
	{
	    cf_i_am_a_gateway_prog();
	    cf_debug();
	    do_areasbbs(DO_LISTGWLINKS);
	}
	else
	{
	    fprintf(stderr, "%s: illegal command %s\n", PROGRAM, cmd[0]);
	    return ERROR;
	}
    }
    
    return OK;
}



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] command ...\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
    exit(EX_USAGE);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] command ...\n\n", PROGRAM);
    fprintf(stderr, "\
options: -n --noaction                don't really do anything ;-)\n\
         -b --areas-bbs NAME          use alternate AREAS.BBS\n\
         -F --filefix                 run as Filefix program (FAREAS.BBS)\n\
         -O --out-dir DIR             set output packet directory\n\
\n\
         -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n\
	 -a --addr Z:N/F.P            set FTN address\n\
	 -u --uplink-addr Z:N/F.P     set FTN uplink address\n\
\n\
command: delete         delete dead areas (no uplink or downlink)\n\
         subscribe      subscribe to areas with new downlinks\n\
         unsubscribe    unsubscribe from areas with no more downlinks\n\
         listgwlinks    list areas with number of ext. links (excl. gateway)\n\
");

    exit(0);
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    char *c_flag=NULL;
    char *a_flag=NULL, *u_flag=NULL;
    char *O_flag=NULL;
    int areafix=TRUE;
    int ret;
    
    int option_index;
    static struct option long_options[] =
    {
	{ "no-rewrite",   0, 0, 'n'},
        { "areas-bbs",	  1, 0, 'b'},
	{ "filefix",      0, 0, 'F'},
	{ "out-dir",      1, 0, 'O'},	/* Set packet directory */

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


    while ((c = getopt_long(argc, argv, "nb:FO:vhc:a:u:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftnaf options *****/
	case 'n':
	    n_flag = TRUE;
	    break;
	case 'b':
	    areafix_set_areasbbs(optarg);
	    break;
	case 'F':
	    areafix = FALSE;
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

    /* Read config file */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /* Process config options */
    if(a_flag)
	cf_set_addr(a_flag);
    if(u_flag)
	cf_set_uplink(u_flag);
    cf_debug();

    /* Process local options */
    pkt_outdir(O_flag ? O_flag : DEFAULT_OUTPKT, NULL);

    /* Common init */
    areafix_init(areafix);
    /* Read PASSWD */
    passwd_init();
    
    ret = 0;
    
    /*
     * Process command on command line
     */
    if(optind >= argc)
    {
	fprintf(stderr, "%s: expecting command\n", PROGRAM);
	short_usage();
    }

    if(areasbbs_init(areafix_areasbbs()) == ERROR)
	exit(EX_OSFILE);

    if(do_cmd(argv+optind) == ERROR)
	ret = EX_DATAERR;
    if(ret==0 && !n_flag)
	if( rewrite_areas_bbs() == ERROR )
	    ret = EX_CANTCREAT;

    exit(ret);
}
