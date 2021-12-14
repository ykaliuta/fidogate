/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ftntick.c,v 4.29 2004/08/22 20:19:13 n0ll Exp $
 *
 * Process incoming TIC files
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
#include <utime.h>



#define PROGRAM		"ftntick"
#define VERSION		"$Revision: 4.29 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



#define MY_AREASBBS	"FAreasBBS"    
#define MY_CONTEXT	"ff"

#define MY_FILESBBS	"files.bbs"



static char *unknown_tick_area = NULL;	/* config.main: UnknownTickArea */

static char in_dir[MAXPATH];		/* Input directory */

static char *exec_script = NULL;	/* -x --exec option */



/*
 * Prototypes
 */
int	do_tic			(int);
int	process_tic		(Tick *);
int	move			(Tick *, char *, char *);
int	copy_file		(char *, char *);
int	add_files_bbs		(Tick *, char *);
int	do_seenby		(LON *, LON *, LON *);
int	check_file		(Tick *);

void	short_usage		(void);
void	usage			(void);



/*
 * Processs *.tic files in Binkley inbound directory
 */
int do_tic(int t_flag)
{
    Tick tic;
    char *name;
    Passwd *pwd;
    char *passwd;
    char buf[MAXPATH];
    char pattern[16];

    tick_init(&tic);
    
    BUF_COPY(pattern, "*.tic");

    dir_sortmode(DIR_SORTMTIME);
    if(dir_open(in_dir, pattern, TRUE) == ERROR)
    {
	logit("$ERROR: can't open directory %s", in_dir);
	return ERROR;
    }
    
    for(name=dir_get(TRUE); name; name=dir_get(FALSE))
    {
	debug(1, "ftntick: tick file %s", name);

	/* Read TIC file */
	if(tick_get(&tic, name) == ERROR)
	{
	    logit("$ERROR: reading %s", name);
	    goto rename_to_bad;
	}

	/* Check file against Tick data */
	if(check_file(&tic) == ERROR)
	    goto rename_to_bad;
	
	tick_debug(&tic, 3);

	logit("area %s file %s (%lub) from %s", tic.area, tic.file, tic.size,
	    znfp1(&tic.from));
	
	/*
	 * Get password for from node
	 */
	if( (pwd = passwd_lookup(MY_CONTEXT, &tic.from)) )
	    passwd = pwd->passwd;
	else
	    passwd = NULL;
	if(passwd)
	    debug(3, "ftntick: password %s", passwd);
	
	/*
	 * Require password unless -t option is given
	 */
	if(!t_flag && !passwd)
	{
	    logit("%s: no password for %s in PASSWD", name,
		znfp1(&tic.from)  );
	    goto rename_to_bad;
	}
	
	/*
	 * Check password
	 */
	if(passwd)
	{
	    if(tic.pw)
	    {
		if(stricmp(passwd, tic.pw))
		{
		    logit("%s: wrong password from %s: ours=%s his=%s",
			name, znfp1(&tic.from), passwd,
			tic.pw                                      );
		    goto rename_to_bad;
		}
	    }
	    else
	    {
		logit("%s: no password from %s: ours=%s", name,
		    znfp1(&tic.from), passwd );
		goto rename_to_bad;
	    }
	}
	
	if(process_tic(&tic) == ERROR)
	{
	    logit("%s: failed", name);
	rename_to_bad:
	    /*
	     * Error: rename .tic -> .bad
	     */
	    str_change_ext(buf, sizeof(buf), name, "bad");
	    rename(name, buf);
	    logit("%s: renamed to %s", name, buf);
	}
	else 
	{
	    /* Run -x script, if any */
	    if(exec_script)
	    {
		int ret;
		
		BUF_EXPAND(buffer, exec_script);
		BUF_APPEND(buffer, " ");
		BUF_APPEND(buffer, name);
		
		debug(4, "Command: %s", buffer);
		ret = run_system(buffer);
		debug(4, "Exit code=%d", ret);
	    }
	    
	    /* o.k., remove the TIC file */
	    if(unlink(name) == ERROR)
		logit("$ERROR: can't remove %s", name);
	}

	tmps_freeall();
    }
    
    dir_close();

    return OK;
}



/*
 * Process Tick
 */
int process_tic(Tick *tic)
{
    AreasBBS *bbs;
    LON new;
    LNode *p;
    char old_name[MAXPATH];
    char new_name[MAXPATH];
    int is_unknown = FALSE;
    
    /*
     * Lookup file area
     */
    if(!tic->area)
    {
	logit("ERROR: missing area in %s", tic->file);
	return ERROR;
    }
    if( (bbs = areasbbs_lookup(tic->area)) == NULL )
    {
	if( unknown_tick_area &&
	    (bbs = areasbbs_lookup(unknown_tick_area)) )
	{
	    is_unknown = TRUE;
	    logit("unknown area %s, using %s instead",
		  tic->area, unknown_tick_area      );
	}
	else
	{
	    logit("unknown area %s from %s",
		tic->area, znfp1(&tic->from) );
	    return ERROR;
	}
    }
    cf_set_zone(bbs->zone);
    tic->to = cf_n_addr();

    if(!is_unknown)
    {
	/*
	 * Check that sender is listed in FAreas.BBS
	 */
	if(! lon_search(&bbs->nodes, &tic->from) )
	{
	    logit("insecure tic area %s from %s", tic->area,
		znfp1(&tic->from)             );
	    return ERROR;
	}
	
	/*
	 * Replaces: move or delete old file
	 */
	if(tic->replaces)
	{
	    char *rdir = cf_get_string("TickReplacedDir", TRUE);
	    
	    BUF_COPY3(old_name, bbs->dir, "/", tic->replaces);
	    if(check_access(old_name, CHECK_FILE) == TRUE)
	    {
		if(rdir)
		{
		    /* Copy to ReplacedFilesDir */
		    BUF_COPY3(new_name, rdir, "/", tic->replaces);
		    debug(1, "%s -> %s", old_name, new_name);
		    if(copy_file(old_name, new_name) == ERROR)
		    {
			logit("$ERROR: can't copy %s -> %s", old_name, new_name);
			return ERROR;
		    }
		    logit("area %s file %s replaces %s, moved to %s",
			tic->area, tic->file, tic->replaces, rdir);
		}
		else
		    logit("area %s file %s replaces %s, removed",
			tic->area, tic->file, tic->replaces);
		
		/* Remove old file, no error if this fails */
		unlink(old_name);
		
		/* Remove old file from FILES.BBS */
		/**FIXME**/
	    }
	}
    }
    
    /*
     * Move file from inbound to file area, add description to FILES.BBS
     */
    BUF_COPY3(old_name, in_dir, "/", tic->file);
    BUF_COPY3(new_name, bbs->dir    , "/", tic->file);
    debug(1, "%s -> %s", old_name, new_name);
    if(move(tic, old_name, new_name) == ERROR)
	return ERROR;
    add_files_bbs(tic, bbs->dir);

    if(!is_unknown)
    {
	/*
	 * Add us to Path list
	 */
	tick_add_path(tic);
	
	/*
	 * Add sender to SEEN-BY if not already there
	 */
	if(!lon_search(&tic->seenby, &tic->from))
	    lon_add(&tic->seenby, &tic->from);
	
	/*
	 * We're the sender
	 */
	tic->from = cf_n_addr();
	
	/*
	 * Add nodes not already in SEEN-BY to seenby and new.
	 */
	lon_init(&new);
	do_seenby(&tic->seenby, &bbs->nodes, &new);
	lon_debug(3, "Send to new nodes: ", &new, TRUE);
	
	/*
	 * Send file to all nodes in LON new
	 */
	BUF_COPY3(new_name, bbs->dir, "/", tic->file);
	for(p=new.first; p; p=p->next)
	{
	    if(tick_send(tic, &p->node, new_name) == ERROR)
		logit("ERROR: send area %s file %s to %s failed",
		    tic->area, tic->file, znfp1(&p->node));
	    tmps_freeall();
	}
    }
    
    return OK;
}



/*
 * Move file (copy then unlink)
 */
int move(Tick *tic, char *old, char *new)
{
    unsigned long crc;
    struct utimbuf ut;

    /* Copy */
    if(copy_file(old, new) == ERROR)
    {
	logit("$ERROR: can't copy %s -> %s", old, new);
	return ERROR;
    }
    
    /* Compute CRC again to be sure */
    crc = crc32_file(new);
    if(crc != tic->crc)
    {
	logit("ERROR: error while copying to %s, wrong CRC", new);
	unlink(new);
	return ERROR;
    }
    
    /* o.k., now unlink file in inbound */
    if(unlink(old) == ERROR)
    {
	logit("$ERROR: can't remove %s", old);
	return ERROR;
    }

    /* Set a/mtime to time from TIC */
    if(tic->date != -1)
    {
	ut.actime = ut.modtime = tic->date;
	if(utime(new, &ut) == ERROR)
	{
#ifndef __CYGWIN32__		/* Some problems with utime() here */
	    logit("$WARNING: can't set time of %s", new);
#endif
#if 0
	    return ERROR;
#endif
	}
    }

    return OK;
}



/*
 * Copy file
 */
int copy_file(char *old, char *new)
{
    FILE *fold, *fnew;
    int nr, nw;
    
    /* Open */
    if( (fold = fopen(old, R_MODE)) == NULL)
    {
	return ERROR;
    }
    if( (fnew = fopen(new, W_MODE)) == NULL)
    {
	fclose(fold);
	return ERROR;
    }

    /* Copy */
    do 
    {
	nr = fread(buffer, sizeof(char), sizeof(buffer), fold);
	if(ferror(fold))
	{
	    logit("$ERROR: can't read from %s", old);
	    fclose(fold);
	    fclose(fnew);
	    unlink(new);
	    return ERROR;
	}
	
	nw = fwrite(buffer, sizeof(char), nr, fnew);
	if(ferror(fnew))
	{
	    logit("$ERROR: can't write to %s", new);
	    fclose(fold);
	    fclose(fnew);
	    unlink(new);
	    return ERROR;
	}
    }
    while(!feof(fold));
    
    /* Close */
    fclose(fold);
    fclose(fnew);

    return OK;
}



/*
 * Add description to FILES.BBS
 */
int add_files_bbs(Tick *tic, char *dir)
{
    char files_bbs[MAXPATH];
    FILE *fp;

    BUF_COPY3(files_bbs, dir, "/", MY_FILESBBS);
    if( (fp = fopen(files_bbs, A_MODE)) == NULL )
    {
	logit("$ERROR: can't append to %s", files_bbs);
	return ERROR;
    }
    
    fprintf(fp, "%-12s  %s\r\n", tic->file, 
	    tic->desc.first ? tic->desc.first->line : "--no description--");

    fclose(fp);
    
    return OK;
}



/*
 * Add nodes to SEEN-BY (4D)
 */
int do_seenby(LON *seenby, LON *nodes, LON *new)
{
    LNode *p;
    
    for(p=nodes->first; p; p=p->next)
	if(! lon_search(seenby, &p->node) )
	{
	    lon_add(seenby, &p->node);
	    if(new)
		lon_add(new, &p->node);
	}

    return OK;
}



/*
 * Check file
 */
int check_file(Tick *tic)
{
    struct stat st;
    unsigned long crc;
    char name[MAXPATH];
    
    if(!tic->file)
    {
	logit("ERROR: no file name");
	return ERROR;
    }

    /* Search file */
    if(dir_search(in_dir, tic->file) == NULL)
    {
	logit("ERROR: can't find file %s", tic->file);
	return ERROR;
    }

    /* Full path name */
    BUF_COPY3(name, in_dir, "/", tic->file);
    if(stat(name, &st) == ERROR)
    {
	logit("$ERROR: can't stat() file %s", name);
	return ERROR;
    }

    /*
     * File size
     */
    if(tic->size)
    {
	if(tic->size != st.st_size)
	{
	    logit("ERROR: wrong size for file %s: got %lu, expected %lu",
		name, st.st_size, tic->size                           );
	    return ERROR;
	}
    }
    else
	tic->size = st.st_size;

    /*
     * File date
     */
    if(tic->date == -1)
	tic->date = st.st_mtime;
    
    /*
     * File CRC
     */
    crc  = crc32_file(name);
    if(tic->crc == 0 && crc != 0)
	tic->crc = crc;
    else
    {
	if(tic->crc != crc)
	{
	    logit("ERROR: wrong CRC for file %s: got %08lx, expected %08lx",
		name, crc, tic->crc                                       );
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
options:  -b --fareas-bbs NAME         use alternate FAREAS.BBS\n\
          -I --inbound DIR             set inbound dir (default: PINBOUND)\n\
          -t --insecure                process TIC files without password\n\
          -x --exec SCRIPT             exec script for incoming TICs,\n\
                                       called as SCRIPT FILE.TIC\n\
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
    char *areas_bbs = NULL;
    char *p;
    int c;
    char *I_flag=NULL;
    int   t_flag=FALSE;
    char *c_flag=NULL;
    char *a_flag=NULL, *u_flag=NULL;
    
    int option_index;
    static struct option long_options[] =
    {
        { "fareas-bbs",	  1, 0, 'b'},
	{ "insecure",     0, 0, 't'},	/* Insecure */
	{ "inbound",      1, 0, 'I'},	/* Set tick inbound */
	{ "exec",         1, 0, 'x'},	/* Run script */

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


    while ((c = getopt_long(argc, argv, "b:tI:x:vhc:a:u:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftntick options *****/
	case 'b':
	    areas_bbs = optarg;
	    break;
	case 't':
	    t_flag = TRUE;
	    break;
	case 'I':
	    I_flag = optarg;
	    break;
	case 'x':
	    exec_script = optarg;
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

    BUF_EXPAND(in_dir, I_flag ? I_flag : cf_p_pinbound());

    /*
     * Process optional config statements
     */
    if( (p = cf_get_string("UnknownTickArea", TRUE)) )
    {
	debug(8, "config: UnknownTickArea %s", p);
	unknown_tick_area = p;
    }

    /*
     * Get name of fareas.bbs file from config file
     */
    if( !areas_bbs && (areas_bbs = cf_get_string(MY_AREASBBS, TRUE)) )
    {
	debug(8, "config: %s %s", MY_AREASBBS, areas_bbs);
    }
    if(!areas_bbs)
    {
	fprintf(stderr, "%s: no areas.bbs specified\n", PROGRAM);
	exit(EX_USAGE);
    }

    /* Read PASSWD */
    passwd_init();
    /* Read FAreas.BBS */
    if(areasbbs_init(areas_bbs) == ERROR)
    {
	logit("$ERROR: can't open %s", areas_bbs);
	return EXIT_ERROR;
    }

    
    if(lock_program(PROGRAM, NOWAIT) == ERROR)
	exit(EXIT_BUSY);
    
    do_tic(t_flag);

    unlock_program(PROGRAM);


    exit(0);
}
