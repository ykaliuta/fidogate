/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id$
 *
 * Processing inbound packets
 *
 *****************************************************************************
 * Copyright (C) 2001
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

#include <sys/wait.h>

#if defined(HAVE_SYS_MOUNT_H) && defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#include <sys/mount.h>
#else
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#define VFS
#else

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#define VFS
#else

#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>


#endif /* HAVE_STATFS_H */
#endif /* HAVE_SYS_STATVFS_H */
#endif /* HAVE_SYSVFS_H */
#endif /* HAVE_SYS_MOUNT_H && HAVE_SYS_PARAM_H */

#if !defined(VFS) || !defined(HAVE_SYS_STATFS_H) || !(defined(HAVE_SYS_MOUNT_H) && defined(HAVE_SYS_PARAM_H))
#define NO_FS
#endif

#define PROGRAM		"runinc"
#define VERSION		"$Revision$"
#define CONFIG		DEFAULT_CONFIG_MAIN

void* subs(char *str,char *macro,char *expand);
void		move			(char *, char *, char *, char *);
void		unpack			(char *);
Unpacking       *arch_type		(char *);
int		run_unpack		(char *, char *, char *, char *);
void		short_usage		(void);
void		do_dir			(char *);
void		run_toss		(Runtoss *);
int		run_tick		(char *);
#if defined(USE_RUNINC_SFGT)
void		send_fidogate		(void);
#endif /* USE_RUNINC_SFGT */
char		*parse_arc		(char *);
Unpacking	*unpacking_parse_line	(char *);
void		unpacking_init		(char *);
int		system_run		(char *cmd);


static  char		pkt_flag	 = FALSE; /* Packet found flag */
static  char		tic_flag	 = FALSE; /* Tick found flag */
static  Unpacking	*unpacking_first = NULL; /* unpack struct first key */
static	Unpacking	*unpacking_last  = NULL; /* unpack struct last key */
static	char		unpack_init      = FALSE; /* Init unpack flag */
static  char 		*libexecdir		 = NULL; /* Fidogate library directory */
static  char 		*bindir	 = NULL; /* Fidogate binary directory */
static  char 		verbose_flag[16];        /* Verbose flag */
static  char		*a_flag		 = NULL; /* Exec script after tosting */
static  char		*b_flag		 = NULL; /* Exec script before tosting */
static	Runtoss		toss[7];
static	char		*site		 = "fidogate";

/*
 * Search processed files and unpack archives
 */
void unpack(char *inb)
{

    DIR *dp;
    Unpacking *a;
    struct dirent *dir;
    char archive[MAXPATH], type[4];
    short unpack_flag = FALSE;


    /* Make sure temporary unpacking directory exists and entering into */
    BUF_COPY2(buffer, inb, "/tmpunpack");
    chdir(buffer);

    /* Reading files into inbound directory */
    if( ! (dp = opendir(inb)) )
    {
	debug(7,"can't open directory %s", inb);
	return;
    }

    while((dir = readdir(dp)))
    {
	/* Is it DOS 8-3 file */
	if(strlen(dir->d_name) == 12 && dir->d_name[8] == '.')
	{
	    
	    BUF_COPY(type, &dir->d_name[9]);
	    str_lower(type);

	    if(!pkt_flag && !strcmp(type, "pkt"))
	    {
		pkt_flag = TRUE;
		    continue;
	    }

	    if(!tic_flag && !strcmp(type, "tic"))
	    {
		tic_flag = TRUE;
		continue;
	    }

	    if(!strncmp(type, "su", 2) || !strncmp(type, "mo", 2) ||
	       !strncmp(type, "tu", 2) || !strncmp(type, "we", 2) ||
	       !strncmp(type, "th", 2) || !strncmp(type, "fr", 2) ||
	       !strncmp(type, "sa", 2))
	    {
		debug(1, "found %s/%s", inb, dir->d_name);

		if(!unpack_init)
		{
		    unpacking_init(cf_p_packing());
		    unpack_init = TRUE;
		}
		    
		BUF_COPY3(archive, inb, "/", dir->d_name);
		a = arch_type(archive);

		if(!a)
		{
		    fglog("unknown archive %s, moving archive to %s/bad", 
			    dir->d_name, inb);
		    move(archive, inb, "/bad/", dir->d_name);
		    continue;
		}

		debug(5, "found %s (%s)", dir->d_name, a->name);
		fglog("archive %s (%s)", archive, a->name);

		if(run_unpack(a->list, a->unarc, archive, a->name))
		{
		    unpack_flag = TRUE;
		    unlink(archive);
		}
		else
		{
		    fglog("WARNING: moving to bad");
		    move(archive, inb, "/bad/", dir->d_name);
		    continue;
		}
	    }
	}
    }
    dir_close();
    if(!unpack_flag)
	return;
    
    BUF_COPY2(buffer, inb, "/tmpunpack");
    if( ! (dp = opendir(buffer)) )
	return;

    while((dir = readdir(dp)))
    {
	if(wildmatch(dir->d_name, "*.pkt", TRUE))
	{
	    BUF_COPY3(archive, buffer, "/", dir->d_name);
	    debug(10, "%s -> %s", archive, inb);
	    move(archive, inb, "/", dir->d_name);
	}
    }
    dir_close();

    pkt_flag = TRUE;

    return;
}

/*
 * Calculate archive type from identification sequens
 */
Unpacking *arch_type( char *name)
{
    FILE *fp;
    char s[10];
    Unpacking *p;
    
    /* Reading chars from archive */
    if( (fp = fopen(name, "r")) )
    {
	fgets(s, 10, fp);
	fclose(fp);

	/*
	 * Search archive unpack program
	*/
	for(p=unpacking_first; p;)
	{
	    if(strncmp(p->iden, s, strlen(p->iden)) == 0)
		return p;

	    if(p->next)
		p=p->next;
	    else
		return NULL;
	}
    }

    return NULL;
}

/*
 * Runing unpacking program
 */
int run_unpack(char *cmd_list, char *cmd_unarc, char *archive, char *type)
{
    char line[MAXPATH];
    FILE *fp;
    char *s;
    int ret;


    debug(5, "cmd - %s, archive - %s", cmd_list, archive);
    str_printf(line, sizeof(line), cmd_list, archive);

    if(!(fp = popen(line, R_MODE)))
    {
    	fglog("ERROR: can't open pipe to %s", line);
    }
    if(strcmp(type, "ZIP") == 0)
    {
	while(fgets(line, MAXPATH, fp));

	pclose(fp);

	if( (s = xstrtok(line, " \t")) )
	{
	    if( 58 > *s && *s > 47)
	    {
		if(atol(s)/10 > check_size(archive))
		{
		    fglog("WARNING: compression archive %s is biggest then 10", archive);
		    return FALSE;
		}
	    }
	    else
	    {
		fglog("WARNING: unpack failed, compresser returns:");
		str_printf(line, sizeof(line), cmd_list, archive);

		if(!(fp = popen(line, R_MODE)))
		{
    		    fglog("ERROR: can't open pipe to %s", line);
		}
		else
		{
		    while(fgets(line, MAXPATH, fp))
		    {
			line[strlen(line)-1] = 0;
			fglog("WARNING: %s", line);
		    }
		    pclose(fp);
		    return FALSE;
		}
	    }	

	}
    }

    debug(5, "cmd - %s, archive - %s", cmd_unarc, archive);
    str_printf(line, sizeof(line), cmd_unarc, archive);
    BUF_APPEND(line, " > /dev/null");

    if( (ret = system_run(line)) == 0 )
    {
	return TRUE;
    }

    debug(5, "exit %d", ret);

    return FALSE;

}

/*
 * Read PACKING config file
 */
Unpacking *unpacking_parse_line(char *buf1)
{

    Unpacking *r;
    char *f, *p;
    short n, i = 0;


    debug(5, "Reading line %s", buf1);

    f = xstrtok(buf1, " \t");

    if(f == NULL)
	return NULL;

    if(!stricmp(f, "include"))
    {
	if((f = xstrtok(NULL, " \t")))
	    unpacking_init(f);
	return NULL;
    }

#ifdef HAVE_STRICMP
    if(!stricmp(f ,"unarc"))
	return NULL;
#else
    if(strcasecmp(f ,"unarc"))
	return NULL;
#endif /* HAVE_STRICMP */
    /* Create new entry and put into list */
    r = (Unpacking *)xmalloc(sizeof(Unpacking));
    f = xstrtok(NULL, " \t");
    p = xstrtok(NULL, " \t");
    if(p && f)
    {
	r->name  = strsave(f);
	f = strsave(p);
    }
    if(!strstr(f ,"0x"))
	r->iden = f;
    else
    {
	r->iden = xmalloc(strlen(f));
	while(*f)
	{
	    /* Recode hex chars */
	    if(*f == '0' && f[1] == 'x')
	    {
		if(f[2] > 58)
		    n = f[2] - 87;
		else
		    n = f[2] - 48;
		n *=16;
		if(f[3] > 58)
		    n += f[3] - 87;
		else
		    n += f[3] - 48;

		(r->iden)[i] = n;
		f+=3;
	    }
	    else
		(r->iden)[i] = *f;
	    
	    f++;
	    i++;
	}
	(r->iden)[i] = '\0';
    }
    r->unarc = strsave(xstrtok(NULL, " \t"));
    r->list  = strsave(xstrtok(NULL, " \t"));
    r->next  = NULL;

    debug(15, "unpack: name=%s unarc=%s list=%s, ident=%s",
	  r->name, r->unarc, r->list, r->iden);


    if(!r->name || !r->unarc || !r->iden)
	return NULL;

    return r;
}

/*
 * Initialize unpacking program and identification sequens
 */
void unpacking_init(char *name)
{
    FILE *fp;
    Unpacking *r;


    debug(5, "Reading packing file %s", name);

    fp = xfopen(name, R_MODE);

    while(cf_getline(buffer, BUFSIZ, fp))
    {

	r = unpacking_parse_line(buffer);

	if(!r)
	    continue;
	
	/* Put into linked list */
	if(unpacking_first)
	    unpacking_last->next = r;
	else
	    unpacking_first      = r;
	unpacking_last = r;
    }

    fclose(fp);

    return;
}

/*
 * Process tick files in input directory
 */
int run_tick(char *inbound)
{
    sprintf(buffer,"%s/ftntick -x %s/ftntickpost -I %s %s", libexecdir, libexecdir,
		    inbound, verbose_flag);

    /* Process tic files */
    return system_run(buffer);
}

/*
 * Remove files
 */
void move(char *old_name, char *dir, char *subdir, char *filename)
{
    char buf[MAXPATH];

    BUF_COPY3(buf, dir, subdir, filename);
    debug(1,"%s -> %s", old_name, buf);
    rename(old_name, buf);

}

/*
 * Make data base inbound directory and runing flags
 */
int toss_init(void)
{
    char *spool = cf_p_spooldir();
    char pathbuffer[MAXPATH];
    unsigned short i;
    
    toss[0].name = "pin";
    BUF_EXPAND(pathbuffer,cf_p_pinbound());
    toss[0].inbound = strsave(pathbuffer);
    sprintf(buffer, "-F%s", toss[0].inbound);
    toss[0].fadir = strsave(buffer);
    toss[0].grade = "-gp";
    toss[0].flags = "-s";

    toss[1].name = "in";
    BUF_EXPAND(pathbuffer,cf_p_inbound());
    toss[1].inbound = strsave(pathbuffer);
    sprintf(buffer, "-F%s", toss[1].inbound);
    toss[1].fadir = strsave(buffer);
    toss[1].grade = "-gi";
    toss[1].flags = "-s";

    toss[2].name = "outpkt";
    BUF_COPY3(buffer, spool, "/", toss[2].name);
    toss[2].inbound = strsave(buffer);
    toss[2].fadir = NULL;
    toss[2].grade = "-go";
    toss[2].flags = "-n -t -p";

    toss[3].name = "outpkt/mail";
    BUF_COPY3(buffer, spool, "/", toss[3].name);
    toss[3].inbound = strsave(buffer);
    toss[3].fadir = NULL;
    toss[3].grade = "-gm";
    toss[3].flags = "-n -t -p";
    
    toss[4].name = "outpkt/news";
    BUF_COPY3(buffer, spool, "/", toss[4].name);
    toss[4].inbound = strsave(buffer);
    toss[4].fadir = NULL;
    toss[4].grade = "-gn";
    toss[4].flags = "-n -t -p";
    
    toss[5].name = "uuin";
    BUF_COPY3(buffer, spool, "/", toss[5].name);
    toss[5].inbound = strsave(buffer);
    toss[5].fadir = NULL;
    toss[5].grade = "-gu";
    toss[5].flags = "-s";
    
    toss[6].name = "ftpin";
    BUF_COPY3(buffer, spool, "/", toss[6].name);
    toss[6].inbound = strsave(buffer);
    toss[6].fadir = NULL;
    toss[6].grade = "-gf";
    toss[6].flags = "-s";

    for(i=0;i<2;i++)
    {
	BUF_COPY2(buffer, toss[i].inbound, "/tmpunpack");
	if(check_access(buffer, CHECK_DIR) == ERROR)
	{
	    if(mkdir(buffer, 0750) == -1)
	    {
		fglog("$ERROR: can't create directory %s", buffer);
		return;
	    }
	    else
	    {
		chmod(buffer, 0750);
		fglog("create directory %s", buffer);
	    }
	}
    }

    return;
}

/*
 * Runing tosting programs
 */
void run_toss(Runtoss *a)
{
    char ftntoss[MAXPATH];
    int ret, state;
    char *p;
#ifndef NO_FS
#ifdef VFS
    struct statfs drive;
#else
    struct statvfs drive;
#endif
#else
    FILE *fp;
    char buf[90];
    char *s;
#endif
    int minfree;


    if( (p = cf_get_string("DiskFreeMin", TRUE)) )
	minfree = atoi(p) * 1024;
    else if( (p = cf_get_string("MinDiskFree", TRUE)) )
	minfree = atoi(p) * 1024;
    else 
	minfree = 1048576;
    debug(8, "DiskFreeMin %d", minfree);
    
    /* Calculate free memore size and number inodes */

#ifndef NO_FS
#ifdef VFS 
    if(statfs(a->inbound, &drive) && (minfree < drive.f_bavail * drive.f_bsize) &&
#else
    if(statvfs(a->inbound, &drive) && (minfree < drive.f_bavail * drive.f_bsize) &&
#endif
	    (drive.f_ffree > 100))
    {
	fglog("ERROR: MinDiskfree limit. Disk is full!");
	return;
    }
#else
    if( (p = cf_get_string("DiskFreeProg", TRUE)) )
    {
	 char *str;
	 if(!(str = subs(p,"%p",a->inbound))) {
	      fglog("ERROR: can't subs macros");
	      return;
	 }
	 
	if( !(fp = popen(str, R_MODE)) )
	{
	    fglog("ERROR: can't open pipe");
	}
	else
	{
	    while(fgets(buf, strlen(buf), fp));
	    p = xstrtok(buf, " \t");
	    s = xstrtok(NULL, " \t");
	    if((s = xstrtok(NULL, " \t")))
		if(atol(s) > minfree)
		{
	    	    fglog("ERROR: MinDiskfree limit. Disk %s is full!", p);
		    pclose(fp);
		    return;
		}
	}
	free(str);
	pclose(fp);
    }
#endif
    /* Runing scripts */
    sprintf(ftntoss, "%s/ftntoss -l -m 400 -x -I %s %s %s %s", libexecdir, a->inbound,
		    a->grade, a->flags, verbose_flag);
    while(1)
    {
	ret = system_run(ftntoss);
	if(ret == 2) continue;
	if(ret != 11 && ret != 0 && ret != 768 && ret != 512)
	{
	    fglog("$WARNING: ftntoss returned %d", ret);
	    unlock_program(PROGRAM);
	    exit_free();
	    exit(1);
	}
	sprintf(buffer,"%s/ftnroute %s %s", libexecdir, a->grade, verbose_flag);
	state = system_run(buffer);
	if(state !=0 && state != 11)
	{
	    fglog("$WARNING: ftnroute returned %d", state);
	    unlock_program(PROGRAM);
	    exit_free();
	    exit(1);
	}

	sprintf(buffer, "%s/ftnpack %s %s %s", libexecdir, a->fadir ? a->fadir : ""
				, a->grade, verbose_flag);
	if((state = system_run(buffer)) !=0 )
	{
	    fglog("$WARNING: ftnpack returned %d", state);
	    unlock_program(PROGRAM);
	    exit_free();
	    exit(1);
	}
	if(ret == 0) break;
    }
    return;
}

#if defined(USE_RUNINC_SFGT)
void send_fidogate(void)
{
    void cat (char *, char *);

    char work[30];
    char batch[30];
#ifdef OLD_BATCHER
    char buf[MAXPATH*2];
#endif /* OLD_BATCHER */    
    struct stat st;

    BUF_COPY2(work, site, ".work");
    BUF_COPY2(batch, site, ".fidogate");

    /* FIXME: processing lock */

    chdir(DEFAULT_INN_BATCHDIR);
    
    if(access(batch, F_OK) != ERROR)
    {
	cat(work, batch);
	unlink(work);
    }
    rename(site,work);

    sprintf(buffer, "%s/ctlinnd -s -t30 flush %s", cf_p_newsbindir(), site);
    system_run(buffer);

    if(stat(site, &st) == ERROR || st.st_size == 0)
    {
	unlink(batch);
    	return;
    }

    cat(work, batch);
    unlink(work);

    sprintf(buffer,"%s/log-news", cf_p_logdir());
    log_file(buffer);

    fglog("begin %s", batch);
#ifdef OLD_BATCHER
    sprintf(buf,"%s/batcher -N 20 -b500000 -p\"%s/rfc2ftn -b \
	    -n %s\" fidogate %s", cf_p_newsbindir(), libexecdir, verbose_flag, batch);
    system_run(buf);
#else
    sprintf(buffer,"%s/rfc2ftn -f %s -m 500 %s", libexecdir, site,
		verbose_flag);
    system_run(buffer);
#endif /* OLD_BATCHER */

    return;

}
#endif /* USE_RUNINC_SFGT */

void cat(char *in, char *out)
{
    FILE *fp, *fo;
    unsigned int i;

    if( (fp = fopen(in, "r")) )
    {
	if((fo = fopen(out, "a+")))
	{
	    while( (i = fgetc(fp)) && i != EOF )
		fputc(i, fo);
	    fclose(fo);
	}
	fclose(fp);
	unlink(in);
    }

    return;
}


/*
 * Process inbound directory
 */
void do_dir( char *input)
{
    Runtoss *p;
    int ret = 0;
    
    /* Search correspond shot inbound name */
    for(p=toss; p->name; p++)
    {
#ifdef HAVE_STRICMP
	if(!stricmp(p->name, input))
#else
	if(!strcasecmp(p->name, input))
#endif /* HAVE_STRICMP */
	{
	    /* Make sure inbound dir exists */
	    if(p->inbound && chdir(p->inbound) == 0)
	    {
		debug(1, "entering %s", p->inbound);

		/* Search process files and runing process if need */
		unpack(p->inbound);

		if (pkt_flag)
		{
		    if(b_flag)
			if ( (ret = system_run(b_flag)) != 0)
			{
			    fglog("$WARNING: before toss script return %d", ret);
			    unlock_program(PROGRAM);
			    exit_free();
			    exit(2);
			}
			    
		    run_toss(p);

		    if(a_flag)
			if( (ret = system_run(a_flag)) != 0)
			{
			    fglog("$WARNING: after toss script return %d", ret);
		    	    unlock_program(PROGRAM);
			    exit_free();
			    exit(2);
			}

		    pkt_flag = FALSE;
		}
		else
		    debug(5, "pkt's not found - ftntoss not started");

		if (tic_flag)
		{
		    if( (ret = run_tick(p->inbound)) != 0)
		    {
			fglog("$WARNING: ftntick return %d status", ret);
		    }
		}
		else
		    debug(5, "tick's not found - ftntick not started");
	
	    }
	    else
		debug(1, "directory %s - %s not found", input, p->inbound);

	    return;
	}
    }

    tmps_freeall();

    return;
}

void* subs(char *str,char *macro,char *expand) {
     char *pos;
     char *newstr;
     pos = strstr(str,macro);
     if(!(newstr = malloc(strlen(str)+strlen(expand) - strlen(macro) + 1)))
	  return NULL;
     if(!strncpy(newstr,str,strlen(str)))
	  return NULL;
     if(!strncpy(newstr+(pos-str), expand, strlen(expand)))
	  return NULL;
     if(!strcpy(newstr+(pos-str)+strlen(expand), pos + strlen(macro)))
	  return NULL;
     return newstr;
}

int system_run(char *cmd)
{
    debug(5,"exec: %s", cmd);
    return system(cmd);

}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );

    fprintf(stderr, "usage:	%s [DIR] [-options]\n", PROGRAM);
    fprintf(stderr, "\
options:\n\
	  DIR			inbound short name: \"in, pin, uuin,ftpin,\n\
	                        outpkt, outpkt/mail, outpkt/news\"\n\
	  -c --config		main configuration file\n\
	  -b --before SCRIPT	exec script before tosting (if packets need)\n\
	  -a --after SCRIPT	exec script after tosting (if packets need)\n\
	  -o --outpkt		process outpkt, outpkt/mail, outpkt/news dirs\n\
	  -s --site SITE	site name for ctlinnd\n\
	  -v --verbose                 verbose\n\
	  -h --help                    this help\n");

    exit(1);
}

int main(int argc, char **argv)
{

    char input[10];
    int option_index, c;
    short out_flag = FALSE;
    char *c_flag = NULL;

    static struct option long_options[] =
    {
	{ "config",       1, 0, 'c' },  /* Config file */
	{ "before",       1, 0, 'b' },  /* Exec script before tosting */
	{ "after",        1, 0, 'a' },  /* Exec script after tosting */
	{ "outpkt",       0, 0, 'o' },  /* Exec packing */
	{ "site",	  1, 0, 's' },  /* Site name */

	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ 0,              0, 0, 0  }
    };

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "c:b:a:os:vh",
			    long_options, &option_index     )) != EOF)
	switch (c) {

	/***** runinc options *****/
	case 'c':
	    c_flag = optarg;
	    break;
	case 'b':
	    b_flag = optarg;
	    break;
	case 'a':
	    a_flag = optarg;
	    break;
	case 'o':
	    out_flag = TRUE;
	    break;
	case 's':
	    site = optarg;
	    break;
	/***** Common options *****/
	case 'v':
	    verbose++;
	    break;
	case 'h':
	    short_usage();
	    break;
#ifndef IGNORE_PARM_RUNINC
	default:
	    short_usage();
	    break;
#endif /* IGNORE_PARM_RUNINC */
	}

    log_program(PROGRAM);

    /*
     * Read config file
     */
    cf_read_config_file(c_flag ? c_flag : DEFAULT_CONFIG_MAIN);

    sprintf(buffer,"%s/log-in", cf_p_logdir());
    log_file(buffer);

    /* More verbose for runing subprograms */
    if(verbose)
    {
        log_file("stdout");
	BUF_APPEND(verbose_flag, " -");
	for(c = verbose; c; c--)
	    BUF_APPEND(verbose_flag, "v");
    }

    libexecdir = cf_p_libexecdir();
    bindir = cf_p_bindir();

    /* Initialize tossing directory and runing flags */
    if(toss_init() == ERROR)
    {
	exit_free();
	exit(1);
    }

    /* Lock it */
    if(lock_program(PROGRAM ,NOWAIT) == ERROR )
    {
	exit_free();
	exit(1);
    }

#ifndef IGNORE_PARM_RUNINC
    if(argv[optind])
    {
	strcpy(input, argv[optind]);
	debug(5,"do_dir(): %s", input);
	do_dir(input);
    }
    else
#endif /* IGNORE_PARM_RUNINC */
    {
	if(out_flag)
	{
#if defined(USE_RUNINC_SFGT)
	    send_fidogate();
#endif /* USE_RUNINC_SFGT */

	    debug(5,"do_dir(): outpkt");
	    do_dir("outpkt");
	    debug(5,"do_dir(): outpkt/mail");
	    do_dir("outpkt/mail");
	    debug(5,"do_dir(): outpkt/news");
	    do_dir("outpkt/news");
	}
	else
	{
	    debug(5,"do_dir(): in");
	    do_dir("in");
	    debug(5,"do_dir(): pin");
	    do_dir("pin");
	}
    }
	
    /* Unlock it */
    unlock_program(PROGRAM);

    exit_free();
    exit(0);
}

