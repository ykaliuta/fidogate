/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ffxqt.c,v 4.27 2004/08/22 20:19:11 n0ll Exp $
 *
 * Process incoming ffx control and data files
 *
 * With full supporting cast of busy files and locking. ;-)
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

#include <sys/wait.h>



#define PROGRAM		"ffxqt"
#define VERSION		"$Revision: 4.27 $"
#define CONFIG		DEFAULT_CONFIG_FFX


#define MAXFFXCMD	16



/*
 * Info from ffx control file
 */
typedef struct st_ffx
{
    char *job;			/* Job name */
    char *name;			/* .ffx file name */
    Node from, to;		/* FTN addresses */
    char *fqdn;			/* Sender FQDN */
    char *passwd;		/* Password */
    char *cmd;			/* Command with args */
    char *in;			/* stdin file */
    char *decompr;		/* Decompressor */
    char *file;
    int  status;		/* Status: TRUE=read # EOF */
} FFX;



/*
 * List of command from config.ffx
 */
typedef struct st_ffxcmd
{
    char type;			/* 'C'=command, 'U'=uncompress */
    char *name;			/* Command name */
    char *cmd;			/* Command to execute */
} FFXCmd;



/*
 * Prototypes
 */
void	parse_ffxcmd		(void);
char   *find_ffxcmd		(int, char *);
int	do_ffx			(int);
void	remove_bad		(char *);
FFX    *parse_ffx		(char *);
int	exec_ffx		(FFX *);
int	run_ffx_cmd		(char *, char *, char *, char *, char *);

void	short_usage		(void);
void	usage			(void);



/*
 * Command line options
 */
static int g_flag = 0;				/* Processing grade */


/*
 * Commands
 */
static FFXCmd l_cmd[MAXFFXCMD];
static int n_cmd = 0;



/*
 * Parse FFXCommand / FFXUncompress config.ffx parameters
 */
void parse_ffxcmd()
{
    char *p, *name, *cmd;

    /* Commands */
    for(p = cf_get_string("FFXCommand", TRUE);
	p && *p;
	p = cf_get_string("FFXCommand", FALSE) )
    {
	if(n_cmd >= MAXFFXCMD)
	    continue;
	name = xstrtok(p   , "\n\t ");
	cmd  = xstrtok(NULL, "\n");
	if(!name || !cmd)
	    continue;
	while(isspace(*cmd))
	    cmd++;
	
	debug(8, "config: FFXCommand %s %s", name, cmd);

	BUF_EXPAND(buffer, cmd);
	l_cmd[n_cmd].type = 'C';
	l_cmd[n_cmd].name = name;
	l_cmd[n_cmd].cmd  = strsave(buffer);
	n_cmd++;
    }
}



/*
 * Find FFXCommand / FFXUncompress
 */
char *find_ffxcmd(int type, char *name)
{
    int i;

    for(i=0; i<n_cmd; i++)
	if(type==l_cmd[i].type && strieq(name, l_cmd[i].name))
	    return l_cmd[i].cmd;

    return NULL;
}



/*
 * Processs *.ffx control files in Binkley inbound directory
 */
int do_ffx(int t_flag)
{
    FFX *ffx;
    char *name;
    Passwd *pwd;
    char *passwd;
    char buf[MAXPATH];
    char pattern[16];
    
    BUF_COPY(pattern, "f???????.ffx");
    if(g_flag)
	pattern[1] = g_flag;

    BUF_EXPAND(buffer, cf_p_pinbound());
    if( chdir(buffer) == -1 )
    {
	logit("$ERROR: can't chdir %s", buffer);
	return ERROR;
    }

    dir_sortmode(DIR_SORTMTIME);
    if(dir_open(".", pattern, TRUE) == ERROR)
    {
	logit("$ERROR: can't open directory .");
	return ERROR;
    }
    
    for(name=dir_get(TRUE); name; name=dir_get(FALSE))
    {
	debug(1, "ffxqt: control file %s", name);

	ffx = parse_ffx(name);
	if(!ffx)
	    /* No error, this might be a file still being written to */
	    continue;

	if(!ffx->status)		/* BSY test, if not EOF */
	    if(bink_bsy_test(&ffx->from))	/* Skip if busy */
	    {
		debug(3, "ffxqt: %s busy, skipping", znfp1(&ffx->from));
		continue;
	    }
	
	/*
	 * Get password for from node
	 */
	if( (pwd = passwd_lookup("ffx", &ffx->from)) )
	    passwd = pwd->passwd;
	else
	    passwd = NULL;
	if(passwd)
	    debug(3, "ffxqt: password %s", passwd);
	
	/*
	 * Require password unless -t option is given
	 */
	if(!t_flag && !passwd)
	{
	    logit("ERROR: %s: no password for %s in PASSWD",
		name, znfp1(&ffx->from)  );
	    goto rename_to_bad;
	}
	
	/*
	 * Check password
	 */
	if(passwd)
	{
	    if(ffx->passwd)
	    {
		if(stricmp(passwd, ffx->passwd))
		{
		    logit("ERROR: %s: wrong password from %s: ours=%s his=%s",
			name, znfp1(&ffx->from), passwd,
			ffx->passwd                              );
		    goto rename_to_bad;
		}
	    }
	    else
	    {
		logit("ERROR: %s: no password from %s: ours=%s", name,
		    znfp1(&ffx->from), passwd );
		goto rename_to_bad;
	    }
	}

	logit("job %s: from %s data %s (%ldb) / %s",
	    ffx->job, znfp1(&ffx->from), ffx->file, check_size(ffx->file),
	    ffx->cmd);
	
	if(exec_ffx(ffx) == ERROR)
	{
	    logit("%s: command failed", name);
	rename_to_bad:
	    /*
	     * Error: rename .ffx -> .bad
	     */
	    str_change_ext(buf, sizeof(buf), name, "bad");
	    rename(name, buf);
	    logit("%s: renamed to %s", name, buf);
	}
	
	tmps_freeall();
    }
    
    dir_close();

    return OK;
}



/*
 * Remove "bad" character from string
 */
void remove_bad(char *s)
{
    char *p = s;
    
    while(*p)
	if(*p>=' ' && *p < 127)
	    switch(*p)
	    {
	    case '$':
	    case '&':
	    case '(':
	    case ')':
	    case ';':
	    case '<':
	    case '>':
	    case '^':
	    case '`':
	    case '|':
		p++;		/* skip */
		break;
	    default:
		*s++ = *p++;
		break;
	    }
	else
	    p++;
    
    *s = 0;
}



/*
 * Parse control file and read into memory
 */
#define SEP " \t\r\n"

FFX *parse_ffx(char *name)
{
    FILE *fp;
    static FFX ffx;
    char *buf, *p;

    /**FIXME: this isn't really clean**/
    xfree(ffx.job);	ffx.job     = NULL;
    xfree(ffx.name);	ffx.name    = NULL;
    xfree(ffx.fqdn);	ffx.fqdn    = NULL;
    xfree(ffx.passwd);	ffx.passwd  = NULL;
    xfree(ffx.cmd);	ffx.cmd     = NULL;
    xfree(ffx.in);	ffx.in      = NULL;
    xfree(ffx.decompr);	ffx.decompr = NULL;
    xfree(ffx.file);	ffx.file    = NULL;
    ffx.from.zone = ffx.from.net = ffx.from.node = ffx.from.point = 0;
    ffx.to  .zone = ffx.to  .net = ffx.to  .node = ffx.to  .point = 0;
    ffx.status = FALSE;
    ffx.name = strsave(name);

    fp = fopen(ffx.name, R_MODE);
    if(!fp)
    {
	logit("$ERROR: can't open %s", ffx.name);
	return NULL;
    }
    
    while((buf = fgets(buffer, BUFFERSIZE, fp)))
    {
	strip_crlf(buf);
	
	switch(*buf)
	{
	case '#':
	    /* Comment */
	    if(!strncmp(buf, "# EOF", 5))
		ffx.status = TRUE;
	    continue;

	case 'J':			/* Job name */
	    ffx.job = strsave(buf + 2);
	    break;
	    
	case 'U':
	    /* User name */
	    p = strtok(buf+2, SEP);
	    /* From node */
	    p = strtok(NULL , SEP);
	    if(p)
		asc_to_node(p, &ffx.from, FALSE);
	    /* To node */
	    p = strtok(NULL , SEP);
	    if(p)
		asc_to_node(p, &ffx.to, FALSE);
	    /* FQDN */
	    p = strtok(NULL , SEP);
	    if(p)
		ffx.fqdn = strsave(p);
	    break;
	    
	case 'I':
	    /* data file */
	    p = strtok(buf+2, SEP);
	    if(p)
	    {
		ffx.in = strsave(p);
		remove_bad(ffx.in);
	    }
	    /* decompressor */
	    p = strtok(NULL , SEP);
	    if(p)
	    {
		ffx.decompr = strsave(p);
		remove_bad(ffx.decompr);
	    }
	    break;
	    
	case 'F':
	    ffx.file = strsave(buf + 2);
	    remove_bad(ffx.file);
	    break;
	    
	case 'C':
	    /* command */
	    ffx.cmd = strsave(buf + 2);
	    remove_bad(ffx.cmd);
	    break;

	case 'P':
	    ffx.passwd = strsave(buf + 2);
	    break;
	    
	}
    }
	
    fclose(fp);

    if(!ffx.cmd)
	return NULL;
    
    debug(3, "ffx: user=%s fqdn=%s", ffx.name, ffx.fqdn ? ffx.fqdn : "-none-");
    debug(3, "     %s -> %s", znfp1(&ffx.from), znfp2(&ffx.to));
    debug(3, "     J %s", ffx.job ? ffx.job : "");
    debug(3, "     I %s %s",
	  ffx.in      ? ffx.in      : "",
	  ffx.decompr ? ffx.decompr : "" );
    debug(3, "     F %s", ffx.file);
    debug(3, "     C %s", ffx.cmd);
    debug(3, "     P %s", ffx.passwd ? ffx.passwd : "");
    
    return &ffx;
}



/*
 * Execute command in ffx
 */
int exec_ffx(FFX *ffx)
{
    int ret;
    char *name, *args=NULL, *cmd_c=NULL;
    
    /* Extract command name and args */
    name = strtok(ffx->cmd, "\n\t ");
    args = strtok(NULL,     "\n"   );
    if(!name)
	return ERROR;
    if(!args)
	args = "";
    while(isspace(*args))
	args++;

    /* Find command and uncompressor */
    cmd_c = find_ffxcmd('C', name);
    if(!cmd_c)
    {
	logit("ERROR: no FFXCommand found for \"%s\"", name);
	return ERROR;
    }
    if(ffx->decompr) 
    {
	logit("ERROR: uncompressing no longer supported in this version");
	return ERROR;
    }

    /* Execute */
    debug(2, "Command: %s", cmd_c);
    ret = run_ffx_cmd(cmd_c, name, ffx->fqdn, args, ffx->in);
    debug(2, "Exit code=%d", ret);

    if(ret == 0)
    {
	unlink(ffx->name);
	if(ffx->file)
	    unlink(ffx->file);
	return OK;
    }
    
    return ERROR;
}



/*
 * Run ffx cmd using fork() and exec()
 */
#define MAXARGS		256

int run_ffx_cmd(char *cmd,
		char *cmd_name, char *cmd_fqdn, char *cmd_args, char *data)
{
    char *args[MAXARGS];
    int n=0;
    char *p;
    pid_t pid;
    int status;

    /* compile args[] vector */
    args[n++] = cmd_name;
    if(cmd_fqdn && streq(cmd_name, "rmail")) 
    {
	args[n++] = "-f";
	args[n++] = cmd_fqdn;
    }

    for(p=strtok(cmd_args, SEP); p; p=strtok(NULL, SEP)) 
    {
	if(n >= MAXARGS-1) 
	{
	    logit("ERROR: too many args in run_ffx_cmd()");
	    return ERROR;
	}
	args[n++] = p;
    }

    args[n] = NULL;

#if 0
    {
	int i;
	debug(7, "cmd=%s", cmd);
	for(i=0; i<=n; i++)
	    debug(7, "args[%d] = %s", i, args[i]);
	debug(7, "data=%s", data);
    }
#endif

    /* fork() and exec() */
    pid = fork();
    if(pid == ERROR) 
    {
	logit("$ERROR: fork failed");
	return ERROR;
    }

    if(pid) 
    {
	/* parent */
	if(waitpid(pid, &status, 0) == ERROR) 
	{
	    logit("$ERROR: waitpid failed");
	    return ERROR;
	}
	if(WIFEXITED(status))
	    return WEXITSTATUS(status);
	if(WIFSIGNALED(status)) 
	    logit("ERROR: child %s caught signal %d", cmd, WTERMSIG(status));
	else
	    logit("ERROR: child %s, exit status=%04x", cmd, status);
	return ERROR;
    }
    else 
    {
	/* child */
	if(! freopen(data, R_MODE, stdin)) 
	{
	    logit("ERROR: can't freopen stdin to %s", data);
	    exit(1);
	}
	if( execv(cmd, args) == ERROR )
	    logit("ERROR: execv %s failed", cmd);
	exit(1);
    }

    /**NOT REACHED**/
    return ERROR;
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
options:  -g --grade G                 processing grade\n\
          -I --inbound dir             set inbound dir (default: PINBOUND)\n\
          -t --insecure                process ffx files without password\n\
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
    char *I_flag=NULL;
    int   t_flag=FALSE;
    char *c_flag=NULL;
    char *a_flag=NULL, *u_flag=NULL;
    
    int option_index;
    static struct option long_options[] =
    {
	{ "grade",        1, 0, 'g'},	/* grade */
	{ "insecure",     0, 0, 't'},	/* Insecure */
	{ "inbound",      1, 0, 'I'},	/* Set Binkley inbound */

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


    while ((c = getopt_long(argc, argv, "g:tI:vhc:a:u:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ffxqt options *****/
	case 'g':
	    g_flag = *optarg;
	    break;
	case 't':
	    t_flag = TRUE;
	    break;
	case 'I':
	    I_flag = optarg;
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

    /* Read config file */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /* Process config options */
    if(a_flag)
	cf_set_addr(a_flag);
    if(u_flag)
	cf_set_uplink(u_flag);

    cf_debug();

    /*
     * Process additional config statements
     */
    parse_ffxcmd();

    if(I_flag)
	cf_s_pinbound(I_flag);

    passwd_init();

    
    if(lock_program(PROGRAM, NOWAIT) == ERROR)
	exit(EXIT_BUSY);
    
    do_ffx(t_flag);

    unlock_program(PROGRAM);


    exit(0);
}
