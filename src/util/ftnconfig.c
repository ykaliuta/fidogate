/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ftnconfig.c,v 4.14 2004/08/22 20:19:14 n0ll Exp $
 *
 * Fetch FIDOGATE config.* parameters
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



#define PROGRAM 	"ftnconfig"
#define VERSION 	"$Revision: 4.14 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



/*
 * Prototypes
 */
int	do_para			(char *);

void	short_usage		(void);
void	usage			(void);



/*
 * Command line options
 */
static int n_flag = FALSE;		/* -n --no-output */
static int l_flag = FALSE;		/* -l --no-newlines */
static int t_flag = FALSE;		/* -t --test-only */



/*
 * Process parameter name on command line
 */
int do_para(char *name)
{
    int ret = FALSE;
    
    /* List of =name for fixed cf_* functions */
    static struct st_parafunc
    {
	char *name;
	char *(*func)(void);
    }
    fixed[] =
    {
        { "=fqdn",        cf_fqdn        },
        { "=hostname",    cf_hostname    },
        { "=domainname",  cf_domainname  },
        { "=hostsdomain", cf_hostsdomain },
        { "=libdir",      cf_p_libdir    },
        { "=spooldir",    cf_p_spooldir  },
        { "=logdir",      cf_p_logdir    },
        { "=inbound",     cf_p_inbound   },
        { "=pinbound",    cf_p_pinbound  },
        { "=uuinbound",   cf_p_uuinbound },
        { "=outbound",    cf_p_btbasedir },
	{ NULL   , NULL     }
    };

    
    /* Fixed parameter */
    if(*name == '=')
    {
	struct st_parafunc *p;

	for(p=fixed; p->name; p++)
	    if(strieq(p->name, name))
	    {
		if(!n_flag)
		    printf("%s%s", p->func(), l_flag ? "" : "\n");
		ret = TRUE;
		break;
	    }
    }
    /* Arbitrary parameter */
    else 
    {
	char *p;

	if( (p = cf_get_string(name, TRUE)) )
	{
	    BUF_EXPAND(buffer, p);
	    if(!n_flag)
		printf("%s%s", buffer, l_flag ? "" : "\n");
	    ret = TRUE;
	}
    }

    if(t_flag)
	printf("%s%s", ret ? "1" : "0", l_flag ? "" : "\n");
    return ret;
}



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] parameter\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] parameter\n\n", PROGRAM);
    fprintf(stderr, "\
options: -l --no-newline              no newline after parameter value\n\
         -n --no-output               no output, exit code only\n\
         -t --test-only               output '1' if present, '0' ifnot\n\
\n\
	 -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config NAME             read config file (\"\" = none)\n");
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    char *c_flag=NULL;
    
    int option_index;
    static struct option long_options[] =
    {
	{ "no-newline",   0, 0, 'l'},	/* No newline */
	{ "no-output",    0, 0, 'n'},	/* No output */
	{ "test-only",    0, 0, 't'},	/* No output */

	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ "config",       1, 0, 'c'},	/* Config file */
	{ 0,              0, 0, 0  }
    };

    /* Init configuration */
    cf_initialize();


    while ((c = getopt_long(argc, argv, "lntvhc:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftnconfig options *****/
        case 'l':
	    l_flag = TRUE;
            break;
        case 'n':
	    n_flag = TRUE;
            break;
        case 't':
	    t_flag = TRUE;
	    n_flag = TRUE;
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

    if(optind != argc-1) 
    {
	    short_usage();
	    exit(EX_USAGE);
    }
    
    exit( do_para(argv[optind]) ? 0 : 1 );
    }
