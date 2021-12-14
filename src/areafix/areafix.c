/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: areafix.c,v 1.23 2004/08/26 20:56:18 n0ll Exp $
 *
 * Common Areafix functions
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



#define MY_NAME_AF	"Areafix"
#define MY_CONTEXT_AF	"af"
#define MY_AREASBBS_AF	"AreasBBS"

#define MY_NAME_FF	"Filefix"
#define MY_CONTEXT_FF	"ff"
#define MY_AREASBBS_FF	"FAreasBBS"

#define MY_NAME		my_name
#define MY_CONTEXT	my_context



int	areafix_tlprintf	(const char *fmt, ...);
int	areafix_stdprintf	(const char *fmt, ...);

int	is_wildcard		(char *);
int	rewrite_areas_bbs	(void);
int	areafix_do_cmd		(Node *, char *, Textlist *);
int	cmd_new		(Node *, char *);
int	cmd_vacation		(Node *, char *);
int	cmd_list		(Node *);
int	cmd_listall		(Node *);
int	cmd_query		(Node *);
int	cmd_unlinked		(Node *);
int	cmd_sub			(Node *, char *);
int	cmd_unsub		(Node *, char *);
int	cmd_help		(Node *);
int	cmd_passwd		(Node *, char *);
int	cmd_delete		(Node *, char *);



/*
 * Number of old AREAS.BBS to keep as AREAS.Onn
 */
#define N_HISTORY	5



/*
 * Global vars
 */

/* Areafix (TRUE) / Filefix (FALSE) mode */
static int areafix              = TRUE;

/* Program name, context, config areas.bbs name */
static char *my_name	        = MY_NAME_AF;
static char *my_context         = MY_CONTEXT_AF;
static char *my_areasbbs        = MY_AREASBBS_AF;

/* Name of areas.bbs file */
static char *areas_bbs          = NULL;
static int   areas_bbs_changed  = FALSE;

static int   authorized     	= FALSE;
static int   authorized_lvl 	= 1;
static char *authorized_key 	= "";
static char *authorized_name    = "Sysop";
static Node  authorized_node    = { -1, -1, -1, -1, "" };
static int   authorized_cmdline = FALSE;
static int   authorized_new     = FALSE;
static int   authorized_delete  = FALSE;


/*
 * Output functions
 */
typedef int (*OFuncP)(const char *, ...);

static OFuncP    areafix_printf = areafix_stdprintf;
static Textlist *areafix_otl    = NULL;


int areafix_tlprintf(const char *fmt, ...)
{
    static char buf[4096];
    int n;
    va_list args;

    va_start(args, fmt);
    
#ifdef HAS_SNPRINTF    
    n = vsnprintf(buf, sizeof(buf), fmt, args);
#else
    n = vsprintf(buf, fmt, args);
    if(n >= sizeof(buf))
    {
        fatal("Internal error - areafix_tlprintf() buf overflow", EX_SOFTWARE);
        /**NOT REACHED**/
        return ERROR;
    }
#endif
    tl_append(areafix_otl, buf);

    va_end(args);

    return OK;
}


int areafix_stdprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    vprintf(fmt, args);
    printf("\n");
    
    va_end(args);

    return OK;
}



/*
 * Common Areafix init
 */
int areafix_init(int mode)
{
    areafix = mode;

    if(mode)
    {
	/* Areafix */
	my_name     = MY_NAME_AF;
	my_context  = MY_CONTEXT_AF;
	my_areasbbs = MY_AREASBBS_AF;
    }
    else
    {
	/* Filefix */
	my_name     = MY_NAME_FF;
	my_context  = MY_CONTEXT_FF;
	my_areasbbs = MY_AREASBBS_FF;
    }
    
    /* Get name of areas.bbs file from config file */
    if(!areas_bbs)
	if( (areas_bbs = cf_get_string(my_areasbbs, TRUE)) )
	{
	    debug(8, "config: %s %s", my_areasbbs, areas_bbs);
	}
    if(!areas_bbs)
    {
	fprintf(stderr, "%s: no areas.bbs specified\n", my_name);
	exit(EX_USAGE);
    }

    return OK;
}



/*
 * Get/set name of areas.bbs file
 */
char *areafix_areasbbs(void)
{
    return areas_bbs;
}

void areafix_set_areasbbs(char *name)
{
    areas_bbs = name;
}



/*
 * Authorize functions
 */
void areafix_auth_init(void)
{
    authorized         = FALSE;
    authorized_lvl     = 1;
    authorized_key     = "";
    authorized_name    = "Sysop";
    node_invalid(&authorized_node);
    authorized_cmdline = FALSE;
    authorized_new     = FALSE;
    authorized_delete  = FALSE;
}

int areafix_auth_check(Node *node, char *passwd)
{
    Passwd *pwd;
    char *p, *s;
    
    /* Init */
    areafix_auth_init();

    /* Check password */
    debug(3, "Node %s, passwd %s", znfp1(node), passwd);
    pwd = passwd_lookup(MY_CONTEXT, node);
    debug(3, "passwd entry: %s", pwd ? pwd->passwd : "-NONE-");
	
    if(passwd && pwd && !stricmp(passwd, pwd->passwd))
    {
	debug(3, "passwd check o.k. - authorized");
	authorized = TRUE;
    }

    if(!authorized)
	return FALSE;
    
    /* Extract level, key, and real name from pwd->args */
    authorized_node = *node;
    s = strsave(pwd->args);
    if( (p = xstrtok(s, " \t")) )
	authorized_lvl = atoi(p);
    if( (p = xstrtok(NULL, " \t")) )
	authorized_key = strsave(p);
    if( (p = xstrtok(NULL, " \t")) )
	authorized_name = strsave(p);
    xfree(s);
    
    debug(3, "passwd lvl : %d", authorized_lvl);
    debug(3, "passwd key : %s", authorized_key);
    debug(3, "passwd name: %s", authorized_name);
    
    if(strchr(authorized_key, '&'))
    {
	debug(3, "authorized for NEW command");
	authorized_new = TRUE;
    }
    if(strchr(authorized_key, '~'))
    {
	debug(3, "authorized for DELETE command");
	authorized_delete = TRUE;
    }
       
    return TRUE;
}

void areafix_auth_cmd(void)
{
    authorized = authorized_cmdline = authorized_new = authorized_delete = TRUE;
}



/*
 * Areafix name
 */
char *areafix_name(void)
{
    return my_name;
}



/*
 * Return authorized node
 */
Node *areafix_auth_node(void)
{
    return &authorized_node;
}



/*
 * Process Areafix command from stdin
 */
int areafix_do(Node *node, char *subj, Textlist *tl, Textlist *out)
{
    char *passwd;
    char *p, *q;
    int q_flag=FALSE, l_flag=FALSE;
    Textline *tp;

    areafix_auth_init();
    
    /* Check password in Subject and process options */
    passwd = strtok(subj, " \t");
    while( (q = strtok(NULL, " \t")) )
    {
	if(!stricmp(q, "-q"))		/* -q = QUERY */
	    q_flag = TRUE;
	if(!stricmp(q, "-l"))		/* -l = LIST */
	    l_flag = TRUE;
    }
    areafix_auth_check(node, passwd);

    /* Execute commands for subject options */
    if(q_flag)
	cmd_query(node);
    if(l_flag)
	cmd_list(node);
    
    /* Execute commands from stdin */
    for(tp=tl->first; tp; tp=tp->next)
    {
	p = tp->line;
	
	strip_crlf(p);				/* Strip CR/LF */
	strip_space(p);				/* Strip spaces */
	if(strneq(p, " * ", 3))			/* Skip " * blah" lines */
	    continue;
	if(strneq(p, "---", 3))			/* Ignore cmds after --- */
	    break;
	if(strneq(p, "--", 2))			/* Ignore cmds after --  */
	    break;				/* (signature start)     */
	if(strneq(p, "--=20", 5))		/* dito, MIME            */
	    break;
	for(; *p && is_space(*p); p++) ;	/* Skip white space */
	if(!*p)					/* Skip empty lines */
	    continue;

	areafix_do_cmd(node, p, out);
    }


    return OK;
}



/*
 * Check for wildcard char in area name
 */
int is_wildcard(char *s)
{
    return
	strchr(s, '*') ||
	strchr(s, '?') ||
	strchr(s, '[')    ;
}



/*
 * Rewrite AREAS.BBS if changed
 */
static void change_num(char *new, size_t len, char *old, int num)
{
    char ext[4];
    str_printf(ext, sizeof(ext), "o%02d", num);
    str_change_ext(new, len, old, ext);
}

#define NEWEXT(new, old, ext)	str_change_ext(new, MAXPATH, old, ext)
#define NEWNUM(new, old, n)	change_num(new, MAXPATH, old, n)


int rewrite_areas_bbs(void)
{
    char old[MAXPATH], new[MAXPATH];
    int i;
    FILE *fp;
    
    if(!areas_bbs_changed)
    {
	debug(4, "AREAS.BBS not changed");
	return OK;
    }

    /* Base name */
    str_expand_name(buffer, MAXPATH, areas_bbs);

    /* Write new one as AREAS.NEW */
    NEWEXT(new, buffer, "new");
    debug(4, "Writing %s", new);

    if( (fp = fopen(new, W_MODE)) == NULL )
    {
	logit("$ERROR: can't open %s for writing AREAS.BBS", new);
	return ERROR;
    }
    if( areasbbs_print(fp) == ERROR )
    {
	logit("$ERROR: writing to %s", new);
	fclose(fp);
	unlink(new);
	return ERROR;
    }
    if( fclose(fp) == ERROR )
    {
	logit("$ERROR: closing %s", new);
	unlink(new);
	return ERROR;
    }

    /* Renumber saved AREAS.Onn */
    NEWNUM(old, buffer, N_HISTORY);
    debug(4, "Removing %s", old);
    unlink(old);

    for(i=N_HISTORY-1; i>=1; i--)
    {
	NEWNUM(old, buffer, i);
	NEWNUM(new, buffer, i+1);
	debug(4, "Renaming %s -> %s", old, new);
	rename(old, new);
    }
    
    /* Rename AREAS.BBS -> AREAS.O01 */
    NEWEXT(old, buffer, "bbs");
    NEWNUM(new, buffer, 1);
    debug(4, "Renaming %s -> %s", old, new);
    rename(old, new);
    
    /* Rename AREAS.NEW -> AREAS.BBS */
    NEWEXT(old, buffer, "new");
    NEWEXT(new, buffer, "bbs");
    debug(4, "Renaming %s -> %s", old, new);
    rename(old, new);

    logit("%s changed", buffer);

    return OK;
}



/*
 * Process command line
 */
#define CMD_LIST	1
#define CMD_QUERY	2
#define CMD_UNLINKED	3
#define CMD_SUB		4
#define CMD_UNSUB	5
#define CMD_HELP	6
#define CMD_PASSWD	7
#define CMD_LISTALL     8
#define CMD_NEW		9
#define CMD_VACATION	10
#define CMD_DELETE	11
#define CMD_COMMENT	12

int areafix_do_cmd(Node *node, char *line, Textlist *out)
{
    int cmd;
    char *arg;
    char buf[32];
    int i, ret;
    int percent = FALSE;

    /* Output */
    if(out) 
    {
	debug(3, "output via textlist");
	areafix_otl    = out;
	areafix_printf = areafix_tlprintf;
    }
    else
    {
	debug(3, "output via stdout");
	areafix_otl    = NULL;
	areafix_printf = areafix_stdprintf;
    }

    debug(2, "node=%s command=%s", znfp1(node), line);

    if(line[0] == '%')
    {
	percent = TRUE;
	line++;
    }

    if(line[0] == '+')
    {
	cmd = CMD_SUB;
	arg = line + 1;
    }
    else if(line[0] == '-')
    {
	cmd = CMD_UNSUB;
	arg = line + 1;
    }
    else if(line[0] == '&')
    {
	cmd = CMD_NEW;
	arg = line + 1;
    }
    else if(line[0] == '~')
    {
	cmd = CMD_DELETE;
	arg = line + 1;
    }
    else
    {
	for(i=0; line[i] && !is_space(line[i]) && i<sizeof(buf)-1; i++)
	    buf[i] = line[i];
	buf[i] = 0;
	arg = line + i;
	
	if     (!stricmp(buf, "list"))
	    cmd = CMD_LIST;
	else if(!stricmp(buf, "query"))
	    cmd = CMD_QUERY;
	else if(!stricmp(buf, "unlinked"))
	    cmd = CMD_UNLINKED;
	else if(!stricmp(buf, "subscribe"))
	    cmd = CMD_SUB;
	else if(!stricmp(buf, "sub"))
	    cmd = CMD_SUB;
	else if(!stricmp(buf, "unsubscribe"))
	    cmd = CMD_UNSUB;
	else if(!stricmp(buf, "unsub"))
	    cmd = CMD_UNSUB;
	else if(!stricmp(buf, "help"))
	    cmd = CMD_HELP;
	else if(!stricmp(buf, "passwd"))
	    cmd = CMD_PASSWD;
	else if(!stricmp(buf, "password"))
	    cmd = CMD_PASSWD;
	else if(!stricmp(buf, "from"))
	    cmd = CMD_PASSWD;
	else if(!stricmp(buf, "listall"))
	    cmd = CMD_LISTALL;
	else if(!stricmp(buf, "new"))
	    cmd = CMD_NEW;
	else if(!stricmp(buf, "create"))
	    cmd = CMD_NEW;
	else if(!stricmp(buf, "vacation"))
	    cmd = CMD_VACATION;
	else if(!stricmp(buf, "delete"))
	    cmd = CMD_DELETE;
	else if(!stricmp(buf, "comment"))
	    cmd = CMD_COMMENT;
	else
	{
	    if(percent)
	    {
		areafix_printf("Unknown command %%%s", buf);
		return OK;
	    }
	    else
	    {
		/* Interpret line as area to add */
		cmd = CMD_SUB;
		arg = line;
	    }
	}
    }

    while(*arg && is_space(*arg))
	arg++;

    debug(2, "cmd=%d node=%s arg=%s", cmd, znfp1(node), arg);

    ret = OK;
    switch(cmd)
    {
    case CMD_LIST:
	ret = cmd_list(node);
	break;
    case CMD_QUERY:
	ret = cmd_query(node);
	break;
    case CMD_UNLINKED:
	ret = cmd_unlinked(node);
	break;
    case CMD_SUB:
	ret = cmd_sub(node, arg);
	break;
    case CMD_UNSUB:
	ret = cmd_unsub(node, arg);
	break;
    case CMD_HELP:
	ret = cmd_help(node);
	break;
    case CMD_PASSWD:
	ret = cmd_passwd(node, arg);
	break;
    case CMD_LISTALL:
	ret = cmd_listall(node);
	break;
    case CMD_NEW:
	ret = cmd_new(node, arg);
	break;
    case CMD_VACATION:
	ret = cmd_vacation(node, arg);
	break;
    case CMD_DELETE:
	ret = cmd_delete(node, arg);
	break;
    case CMD_COMMENT:
	/* ignore */
	break;
    }	
    
    return ret;
}



/*
 * New command
 */
int cmd_new(Node *node, char *line)
{
    AreasBBS *p;
    char *name, *o1, *o2;

    if(!authorized_new)
    {
	areafix_printf("Command NEW: not authorized.");
	return OK;
    }

    name = xstrtok(line, " \t");

    if( (p = areasbbs_lookup(name)) )
    {
	areafix_printf("%s: area already exists, can't create new one.",
		name);
	return OK;
    }

    /* Create new areas.bbs entry */
    p = areasbbs_new();
    
    p->dir   = "-";
    p->area  = strsave(name);
    p->zone  = node->zone;

    /* Parse options:
     *
     *     -#            passthru
     *     -p            passthru
     *     -r            read-only
     *     -l LVL        Areafix access level
     *     -k KEY        Areafix access key
     *     -z Z          zone                 */
    while( (o1 = xstrtok(NULL, " \t")) )
    {
	if(streq(o1, "-#") || streq(o1, "-p"))		/* -# */
	    p->flags |= AREASBBS_PASSTHRU;
	    
	if(streq(o1, "-r"))				/* -r */
	    p->flags |= AREASBBS_READONLY;
	    
	if(streq(o1, "-l"))				/* -l LVL */
	{
	    if(! (o2 = xstrtok(NULL, " \t")) )
		break;
	    p->lvl = atoi(o2);
	}
	
	if(streq(o1, "-k"))				/* -k KEY */
	{
	    if(! (o2 = xstrtok(NULL, " \t")) )
		break;
	    p->key = strsave(o2);
	}

	if(streq(o1, "-d"))				/* -d DESC */
	{
	    if(! (o2 = xstrtok(NULL, " \t")) )
		break;
	    p->desc = strsave(o2);
	}
	if(streq(o1, "-z"))				/* -z Z */
	{
	    if(! (o2 = xstrtok(NULL, " \t")) )
		break;
	    p->zone = atoi(o2);
	}
    }	
    
    lon_init(&p->nodes);
    lon_add(&p->nodes, node);

    areasbbs_add(p);

    logit("%s: new %s lvl=%d key=%s desc=%s%s%s",
	znfp1(node),
	p->area,
	p->lvl,
	p->key ? p->key : "",
	p->desc ? p->desc : "",
	p->flags & AREASBBS_PASSTHRU ? " passthru" : "",
	p->flags & AREASBBS_READONLY ? " ro" : "");

    areas_bbs_changed = TRUE;

    return OK;
}



/*
 * Vacation command
 */
int cmd_vacation(Node *node, char *area)
{
    logit("%s: vacation", znfp1(node));

    if(!authorized)
    {
	areafix_printf("Command VACATION: not authorized.");
	return OK;
    }

    areafix_printf("Command VACATION: sorry, not yet implemented.");

    return OK;
}



/*
 * ListAll command
 */
int cmd_listall(Node *node)
{
    AreasBBS *p;
    LON *l;
    
    logit("%s: listall", znfp1(node));

    if(!authorized)
    {
	areafix_printf("Command LISTALL: not authorized.");
	return OK;
    }
    
    areafix_printf("");
    areafix_printf("ALL available areas:");
    areafix_printf("");
    
    for(p=areasbbs_first(); p; p=p->next)
    {
	char *mark;

	l = &p->nodes;
	mark = (lon_search(l, node) ? "*" : " ");

	if(p->desc)
	    areafix_printf("%s Z%-3d %-39s: %s",
			   mark, p->zone, p->area, p->desc);
	else
	    areafix_printf("%s Z%-3d %s",
			   mark, p->zone, p->area);
    }
    
    areafix_printf("");
    areafix_printf("* = linked to this area");
    areafix_printf("");

    return OK;
}



/*
 * List command
 */
int cmd_list(Node *node)
{
    AreasBBS *p;
    LON *l;
    char *s;
    int key_ok;
    
    logit("%s: list", znfp1(node));

    if(!authorized)
    {
	areafix_printf("Command LIST: not authorized.");
	return OK;
    }
    
    areafix_printf("");
    areafix_printf("Areas available to %s:", znf1(node));
    areafix_printf("");
    
    for(p=areasbbs_first(); p; p=p->next)
    {
	char *mark;

	l = &p->nodes;

	/* Check permissions */
	if(p->lvl > authorized_lvl)
	    continue;
	if(p->key)
	{
	    key_ok = TRUE;
	    for(s=p->key; *s; s++)
		if(!strchr(authorized_key, *s))
		{
		    key_ok = FALSE;
		    break;
		}
	    if(!key_ok)
		continue;
	}

	/* Check zone */
	if(areafix && p->zone!=node->zone)
	    continue;
	
	mark = (lon_search(l, node) ? "*" : " ");
	if(p->desc)
	    areafix_printf("%s %-39s: %s", mark, p->area, p->desc);
	else
	    areafix_printf("%s %s", mark, p->area);
    }
    
    areafix_printf("");
    areafix_printf("* = linked to this area");
    areafix_printf("");
    
    return OK;
}



/*
 * Query command
 */
int cmd_query(Node *node)
{
    AreasBBS *p;
    LON *l;
    
    logit("%s: query", znfp1(node));

    if(!authorized)
    {
	areafix_printf("Command QUERY: not authorized.");
	return OK;
    }
    
    areafix_printf("");
    areafix_printf("%s is linked to the following areas:", znf1(node));
    areafix_printf("");
    
    for(p=areasbbs_first(); p; p=p->next)
    {
	l = &p->nodes;

	if(lon_search(l, node))
	    areafix_printf("  %s", p->area);
    }
    
    areafix_printf("");
    
    return OK;
}



/*
 * Unlinked command
 */
int cmd_unlinked(Node *node)
{
    AreasBBS *p;
    LON *l;
    char *s;
    int key_ok;
    
    logit("%s: unlinked", znfp1(node));

    if(!authorized)
    {
	areafix_printf("Command UNLINKED: not authorized.");
	return OK;
    }
    
    areafix_printf("");
    areafix_printf("%s is not linked to the following available areas:",
		   znf1(node));
    areafix_printf("");
    
    for(p=areasbbs_first(); p; p=p->next)
    {
	l = &p->nodes;

	/* Check permissions */
	if(p->lvl > authorized_lvl)
	    continue;
	if(p->key)
	{
	    key_ok = TRUE;
	    for(s=p->key; *s; s++)
		if(!strchr(authorized_key, *s))
		{
		    key_ok = FALSE;
		    break;
		}
	    if(!key_ok)
		continue;
	}

	/* Check zone */
	if(areafix && p->zone!=node->zone)
	    continue;
	
	if(! lon_search(l, node))
	    areafix_printf("  %s", p->area);
    }
    
    areafix_printf("");

    return OK;
}



/*
 * Add command
 */
int cmd_sub(Node *node, char *area)
{
    AreasBBS *p;
    LON *l;
    int match = FALSE;
    char *s;
    int key_ok;
    int iswc;
    
    if(!authorized)
    {
	areafix_printf("Command SUBSCRIBE: not authorized.");
	return OK;
    }

    iswc = is_wildcard(area);
    
    for(p=areasbbs_first(); p; p=p->next)
    {
	l = &p->nodes;

	if(wildmatch(p->area, area, TRUE))
	{
	    match = TRUE;

	    if(!authorized_cmdline)	/* Command line may do everything */
	    {
		/* Check permissions */
		if(p->lvl > authorized_lvl)
		{
		    if(!iswc)
			areafix_printf("%-41s: access denied (level)",
				       p->area);
		    continue;
		}
		if(p->key)
		{
		    key_ok = TRUE;
		    for(s=p->key; *s; s++)
			if(!strchr(authorized_key, *s))
			{
			    key_ok = FALSE;
			    break;
			}
		    if(!key_ok)
		    {
			if(!iswc)
			    areafix_printf("%-41s: access denied (key)",
					   p->area);
			continue;
		    }
		}

		/* Check zone */
		if(areafix && p->zone!=node->zone)
		{
		    if(!iswc)
			areafix_printf("%-41s: different zone (Z%d), not added",
				       p->area, p->zone);
		    continue;
		}
	    }
	    
	    if(lon_search(l, node))
		areafix_printf("%-41s: already active", p->area);
	    else 
	    {
		if(l->first)
		{
		    lon_add(l, node);
		    areas_bbs_changed = TRUE;
		    areafix_printf("%-41s: subscribed", p->area);
		    
		    if(p->state && strchr(p->state, 'U'))
		    {
			/* Not subscribed at uplink, print note */
			areafix_printf("        (this area is currently not subscribed at uplink %s)", znf1(&l->first->node));
			logit("%s: +%s (not subscribed at uplink)",
			    znfp1(node), p->area);
		    }
		    else
			logit("%s: +%s", znfp1(node), p->area);
		}
		else
		{
		    areafix_printf("%s: no uplink, dead area", p->area);
		    logit("%s: dead area %s", znfp1(node), p->area);
		}
	    }
	}
    }

    if(!match)
    {
	areafix_printf("%s: no such area, or no area matching pattern",
		       area);
    }
    
    return OK;
}



/*
 * Unsubscribe command
 */
int cmd_unsub(Node *node, char *area)
{
    AreasBBS *p;
    LON *l;
    int match = FALSE;
    int iswild = strchr(area, '*') || strchr(area, '?');
					     
    if(!authorized)
    {
	areafix_printf("Command UNSUBSCRIBE: not authorized.");
	return OK;
    }
    
    for(p=areasbbs_first(); p; p=p->next)
    {
	l = &p->nodes;

	if(wildmatch(p->area, area, TRUE))
	{
	    match = TRUE;

	    if(!lon_search(l, node))
	    {
		if(!areafix || p->zone==node->zone)
		    if(!iswild)
			areafix_printf("%-41s: not active", p->area);
	    }
	    else 
	    {
		lon_remove(l, node);
		areas_bbs_changed = TRUE;
		areafix_printf("%-41s: unsubscribed", p->area);

		logit("%s: -%s", znfp1(node), p->area);
	    }
	}
    }
    
    if(!match)
	areafix_printf("%s: no such area, or no area matching pattern",
		       area);
	
    return OK;
}



/*
 * Help command
 */
int cmd_help(Node *node)
{
    FILE *fp;
    char *helpfile;

    logit("%s: help", znfp1(node));

    if( (helpfile = cf_get_string("AreaFixHelp", TRUE)) )
    {
	if( (fp = fopen_expand_name(helpfile, R_MODE, FALSE)) ) 
	{
	    while(fgets(buffer, sizeof(buffer), fp))
	    {
		strip_crlf(buffer);
		areafix_printf(buffer);
	    }
	    fclose(fp);
	    return OK;
	}
	else
	    logit("$ERROR: can't open %s", helpfile);
    }
    else
	logit("WARNING: AreaFixHelp not defined");

    areafix_printf("Sorry, no help available.");

    return OK;
}



/*
 * Passwd command
 */
int cmd_passwd(Node *node, char *arg)
{
    char *p;
    Node n;
    
    logit("%s: passwd", znfp1(node));

    authorized = FALSE;

    p = strtok(arg, " \t");			/* Node address */
    if(!p)
    {
	areafix_printf("Command PASSWORD: missing Z:N/F.P address.");
	return OK;
    }	
    if( asc_to_node(p, &n, FALSE) == ERROR )
    {
	areafix_printf("Command PASSWORD: illegal address %s.", p);
	return OK;
    }
    *node = n;
    cf_set_zone(node->zone);
    
    p = strtok(NULL, " \t");			/* Password */
    if(!p)
    {
	areafix_printf("Command PASSWORD: no password given!");
	authorized = FALSE;
	return OK;
    }

    areafix_auth_check(node, p);
    if(!authorized)
	areafix_printf("Command PASSWORD: authorization for %s failed.",
		       znfp1(node));
    
    return OK;
}



/*
 * Delete command
 */
int cmd_delete(Node *node, char *area)
{
    if(!authorized_new)
    {
	areafix_printf("Command DELETE: not authorized.");
	return OK;
    }

    areafix_printf("Command DELETE: sorry, not yet implemented.");
	
    return OK;
}



/*
 * Set areas_bbs_changed flag
 */
void areafix_set_changed(void)
{
    areas_bbs_changed = TRUE;
}
