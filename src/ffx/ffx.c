/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ffx.c,v 4.20 2004/08/26 20:56:20 n0ll Exp $
 *
 * ffx FIDO-FIDO execution
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

#include <pwd.h>



#define PROGRAM		"ffx"
#define VERSION		"$Revision: 4.20 $"
#define CONFIG		DEFAULT_CONFIG_FFX



/*
 * Configuration
 */
static char *data_flav = "Normal";



/*
 * Prototypes
 */
char   *get_user		(void);
char   *new_job_id		(int);
int	ffx			(Node *, int, char **,
				 char *, int, char *);

void	short_usage		(void);
void	usage			(void);



/*
 * Get current user id
 */
char *get_user(void)
{
    struct passwd *pwd;

    if((pwd = getpwuid(getuid())))
	return pwd->pw_name;
    
    return "root";
}



/*
 * Get new job ID
 */
char *new_job_id(int f)
{
    static char buf[16];
    long seq;

    seq = sequencer(DEFAULT_SEQ_FF) % 1000000;		/* max. 6 digits */
    if(!f)
	f = 'f';
  
    str_printf(buf, sizeof(buf), "f%c%06ld", f, seq);
    return buf;
}



/*
 * Do the remote execution
 */
int ffx(Node *node, int cmdc, char **cmdv,
	char *flav, int grade, char *batch)
{
    int i, ret;
    char *seq;
    char *out;
    char ctrlname[MAXPATH], dataname[MAXPATH];
    FILE *ctrl, *data;
    Passwd *pwd;
    char *password;
    
    for(i=0; i<cmdc; i++)
	if(i == 0)
	    BUF_COPY(buffer, cmdv[i]);
	else
	{
	    BUF_APPEND(buffer, " ");
	    BUF_APPEND(buffer, cmdv[i]);
	}

    seq = new_job_id(grade);
    cf_set_zone(node->zone);
    
    out = cf_zones_out(node->zone);
    if(!out)
    {
	logit("ffx: no configured outbound for zone %d",
		node->zone);
	return EX_DATAERR;
    }

    if(batch)
    {
	str_printf(ctrlname, sizeof(ctrlname),
		"%s/%s/%s", cf_p_btbasedir(), out, batch);
	if( mkdir(ctrlname, DIR_MODE) == -1 )
	    if(errno != EEXIST)
	    {
		logit("$ffx: can't create directory %s", ctrlname);
		return EX_OSERR;
	    }
	
	str_printf(ctrlname, sizeof(ctrlname),
		   "%s/%s/%s/%s.ffx", cf_p_btbasedir(), out, batch, seq);
	str_printf(dataname, sizeof(dataname),
		   "%s/%s/%s/%s",
		   cf_p_btbasedir(), out, batch, seq);
    }
    else
    {
	str_printf(ctrlname, sizeof(ctrlname),
		   "%s/%s/%s.ffx", cf_p_btbasedir(), out, seq);
	str_printf(dataname, sizeof(dataname),
		   "%s/%s/%s" , cf_p_btbasedir(), out, seq);
    }
    
    debug(2, "ffx: job=%s", seq);
    debug(2, "ffx: ctrl=%s", ctrlname);
    debug(2, "ffx: data=%s", dataname);
    
    logit("job %s: to %s / %s", seq, znfp1(node), buffer);

    /* Get password for node */
    pwd = passwd_lookup("ffx", node);
    if(pwd)
    {
	password = pwd->passwd;
	if(password)
	    debug(3, "ffx: password=%s", password);
    }
    else
	password = NULL;
    
    /* Create control file */
    ctrl = fopen(ctrlname, W_MODE);
    if(!ctrl)
    {
	logit("$ffx: can't create %s", ctrlname);
	return EX_OSERR;
    }
    
    chmod(ctrlname, DATA_MODE);
    fprintf(ctrl, "# ffx %s\n", version_local(VERSION));
    fprintf(ctrl, "U %s %s %s %s\n",
	    get_user(), znfp1(cf_addr()), znfp2(node), cf_fqdn());
    fprintf(ctrl, "Z\n");
    fprintf(ctrl, "J %s\n", seq);
    fprintf(ctrl, "F %s\n", seq);
    fprintf(ctrl, "I %s\n", seq);
    fprintf(ctrl, "C %s\n", buffer);
    if(password)
	fprintf(ctrl, "P %s\n", password);
    fprintf(ctrl, "# EOF\n");
    
    fclose(ctrl);

    /* Copy stdin to data file */
    ret = OK;
    if( (data = fopen(dataname, W_MODE)) )
    {
	int nr, nw;

	chmod(dataname, DATA_MODE);

	do 
	{
	    nr = fread(buffer, sizeof(char), sizeof(buffer), stdin);
	    if(ferror(stdin))
	    {
		logit("$ERROR: can't read from stdin");
		ret = ERROR;
	    }
	    
	    nw = fwrite(buffer, sizeof(char), nr, data);
	    if(ferror(data))
	    {
		logit("$ERROR: can't write to %s", dataname);
		ret = ERROR;
	    }
	}
	while(!feof(stdin));
	
	fclose(data);
    }
    else
    {
	logit("$ffx: can't open %s", dataname);
	ret = ERROR;
    }
    
    if(ret)
    {
	logit("ffx: failed to spool job %s", seq);
	unlink(ctrlname);
	unlink(dataname);
	return EX_OSERR;
    }

    if(!batch)
    {
	/*
	 * Attach to FLO file
	 *
	 * batch==NULL: TRUE, if -b option is not given. FALSE, if -b
	 * option is given. In this case the BSY file will be created
	 * and deleted in main().
	 */
	if(bink_attach(node, '^', dataname, flav, batch==NULL) == ERROR)
	{
	    logit("ffx: failed to spool job %s", seq);
	    unlink(ctrlname);
	    unlink(dataname);
	    return EX_OSERR;
	}
	if(bink_attach(node, '^', ctrlname, flav, batch==NULL) == ERROR)
	{
	    logit("ffx: failed to spool job %s", seq);
	    unlink(ctrlname);
	    unlink(dataname);
	    return EX_OSERR;
	}
    }

    return OK;
}



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] node command\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
    exit(EX_USAGE);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] node command\n\n", PROGRAM);
    fprintf(stderr, "\
options:  -b --batch-dir DIR           operate in batch mode, using DIR\n\
          -B --binkley DIR             set Binkley-style outbound directory\n\
          -F --flavor FLAV             Hold | Normal | Direct | Crash\n\
          -g --grade G                 Grade [a-z]\n\
          -n                           ignored for compatibilty\n\
\n\
          -v --verbose                 more verbose\n\
	  -h --help                    this help\n\
          -c --config FILE             read config file (\"\" = none)\n\
	  -L --lib-dir DIR             set lib directory\n\
	  -S --spool-dir DIR           set spool directory\n\
	  -a --addr Z:N/F.P            set FTN address\n\
	  -u --uplink-addr Z:N/F.P     set FTN uplink address\n");

    exit(0);
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, ret;
    char *p;
    char *b_flag=NULL, *B_flag=NULL;
    char *F_flag=NULL;
    int   g_flag=0;
    char *c_flag=NULL;
    char *S_flag=NULL, *L_flag=NULL;
    char *a_flag=NULL, *u_flag=NULL;

    int option_index;
    static struct option long_options[] =
    {
	{ "batch-dir",    1, 0, 'b'},	/* Set batch mode and batch dir */
	{ "binkley",      1, 0, 'B'},	/* Binkley outbound base dir */
	{ "flavor",       1, 0, 'F'},	/* Outbound flavor */
	{ "grade",        1, 0, 'g'},	/* ffx grade (a-z) */

	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ "config",       1, 0, 'c'},	/* Config file */
	{ "spool-dir",    1, 0, 'S'},	/* Set FIDOGATE spool directory */
	{ "lib-dir",      1, 0, 'L'},	/* Set FIDOGATE lib directory */
	{ "addr",         1, 0, 'a'},	/* Set FIDO address */
	{ "uplink-addr",  1, 0, 'u'},	/* Set FIDO uplink address */
	{ 0,              0, 0, 0  }
    };

    Node node;
    char **cmdv;
    int cmdc;

    log_program(PROGRAM);
    
    /* Init configuration */
    cf_initialize();


    while ((c = getopt_long(argc, argv, "b:B:F:g:nvhc:S:L:a:u:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	case 'b':
	    b_flag = optarg;
	    break;
	case 'B':
	    B_flag = optarg;
	    break;
	case 'F':
	    F_flag = optarg;
	    break;
	case 'g':
	    g_flag = *optarg;
	    if(g_flag<'a' || g_flag>'z')
		g_flag = 0;
	    break;
	case 'n':
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
	case 'S':
	    S_flag = optarg;
	    break;
	case 'L':
	    L_flag = optarg;
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


    /* Don't allow options for setuid ffx */
    if(getuid() != geteuid())
    {
	L_flag = c_flag = B_flag = S_flag = a_flag = u_flag = NULL;
    }
    
    /*
     * Read config file
     */
    if(L_flag)				/* Must set libdir beforehand */
	cf_s_libdir(L_flag);
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /*
     * Process config options
     */
    if(B_flag)
	cf_s_btbasedir(B_flag);
    if(L_flag)
	cf_s_libdir(L_flag);
    if(S_flag)
	cf_s_spooldir(S_flag);
    if(a_flag)
	cf_set_addr(a_flag);
    if(u_flag)
	cf_set_uplink(u_flag);

    cf_debug();

    /*
     * Process optional config statements
     */
    if( (p = cf_get_string("FFXDataFlav", TRUE)) )
    {
	debug(8, "config: FFXDataFlav %s", p);
	data_flav = p;
    }


    /*
     * Node address from command line
     */
    if(optind >= argc)
    {
	logit("expecting FTN address");
	short_usage();
    }
    if(asc_to_node(argv[optind], &node, FALSE) == ERROR)
    {
	logit("invalid FTN address %s", argv[optind]);
	short_usage();
    }
    optind++;

    /*
     * Remote execution command
     */
    if(optind >= argc)
    {
	logit("expecting command");
	short_usage();
    }
    cmdc = argc - optind;
    cmdv = &argv[optind];

    passwd_init();
    
    if(b_flag  &&  bink_bsy_create(&node, WAIT) == ERROR)
	exit(1);
    
    ret = ffx(&node, cmdc, cmdv, 
	      F_flag ? F_flag : data_flav,
	      g_flag, b_flag              );
    tmps_freeall();

    if(b_flag)
	bink_bsy_delete(&node);
    
    exit(ret);
}
