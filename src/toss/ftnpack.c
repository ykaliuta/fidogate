/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: ftnpack.c,v 4.33 2004/08/22 20:19:14 n0ll Exp $
 *
 * Pack output packets of ftnroute for Binkley outbound (ArcMail)
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
#include <signal.h>




#define PROGRAM 	"ftnpack"
#define VERSION 	"$Revision: 4.33 $"
#define CONFIG		DEFAULT_CONFIG_MAIN



/*
 * Command line options
 */
int  g_flag   = 0;
int  pkt_flag = FALSE;			/* -P --pkt */
int  ffx_flag = FALSE;			/* -f --ffx NODE */
Node ffx_node;				/*           ^   */
long maxarc   = 0;			/* -m --maxarc SIZE */

static char in_dir [MAXPATH];
static char out_dir [MAXPATH];
static char file_attach_dir[MAXPATH];

static int severe_error = OK;		/* ERROR: exit after error */

static int signal_exit = FALSE;		/* Flag: TRUE if signal received */


/* "noarc" packer program */
static ArcProg noarc = 
{
    PACK_ARC, "noarc", NULL, NULL
};

/* packer programs linked list */
static ArcProg *arcprog_first = &noarc;
static ArcProg *arcprog_last  = &noarc;


/* packing commands linked list */
static Packing *packing_first = NULL;
static Packing *packing_last  = NULL;



/*
 * Prototypes
 */
int	parse_pack		(char *);
ArcProg *parse_arc		(char *);
void	new_arc			(int);
Packing *packing_parse_line	(char *);
int	packing_do_file		(char *);
void	packing_init		(char *);
char   *arcmail_name		(Node *, char *);
char   *packing_pkt_name		(char *, char *);
int	arcmail_search		(char *);
int	do_arcmail		(char *, Node *, Node *, PktDesc *,
				 FILE *, char *, char *);
int	do_noarc		(char *, Node *, PktDesc *, FILE *, char *);
void	set_zero		(Node *);
int	do_pack			(PktDesc *, char *, FILE *, Packing *);
int	do_dirpack		(PktDesc *, char *, FILE *, Packing *);
int	do_packing		(char *, FILE *, Packet *);
int	do_packet		(FILE *, Packet *, PktDesc *);
void	add_via			(Textlist *, Node *);
int	do_file			(char *);
void	prog_signal		(int);
void	short_usage		(void);
void	usage			(void);



/*
 * Parse PACKING commands
 */
int parse_pack(char *s)
{
    if(!stricmp(s, "pack"))
	return PACK_NORMAL;
    if(!stricmp(s, "rpack"))
	return PACK_ROUTE;
    if(!stricmp(s, "fpack"))
	return PACK_FLO;
    if(!stricmp(s, "dirpack"))
	return PACK_DIR;
    if(!stricmp(s, "dirmove"))
	return PACK_MOVE;
    if(!stricmp(s, "arc"))
	return PACK_ARC;
    if(!stricmp(s, "prog"))
	return PACK_PROG;
    if(!stricmp(s, "progn"))
	return PACK_PROGN;
    
    return ERROR;
}



/*
 * Parse archiver/program name
 */
ArcProg *parse_arc(char *s)
{
    ArcProg *p;
    
    for(p=arcprog_first; p; p=p->next)
	if(!stricmp(p->name, s))
	    return p;
    return NULL;
}



/*
 * Define new archiver/program
 */
void new_arc(int cmd)
{
    char *name, *prog;
    ArcProg *a;
    
    name = xstrtok(NULL, " \t");
    prog = xstrtok(NULL, " \t");
    if(!name || !prog)
    {
	logit("packing: missing argument for arc/prog definition");
	return;
    }
    
    /* Create new entry and put into list */
    a = (ArcProg *)xmalloc(sizeof(ArcProg));
    a->pack = cmd;
    a->name = strsave(name);
    a->prog = strsave(prog);
    a->next = NULL;

    if(arcprog_first)
	arcprog_last->next = a;
    else
	arcprog_first      = a;
    arcprog_last = a;
	
    debug(15, "packing: pack=%c name=%s prog=%s",
	  a->pack, a->name, a->prog             );
}



/*
 * Read PACKING config file
 */
Packing *packing_parse_line(char *buf)
{
    Packing *r;
    ArcProg *a;
    char *p;
    char *dir;
    Node old, node;
    LON lon;
    int cmd;

    /* Command */
    p = xstrtok(buf, " \t");
    if(!p)
	return NULL;
    if(strieq(p, "include"))
    {
	p = xstrtok(NULL, " \t");
	packing_do_file(p);
	return NULL;
    }
    if((cmd = parse_pack(p)) == ERROR)
    {
	logit("packing: unknown command %s", p);
	return NULL;
    }

    /* Definition of new archiver/program */
    if(cmd==PACK_ARC || cmd==PACK_PROG || cmd==PACK_PROGN)
    {
	new_arc(cmd);
	return NULL;
    }

    /* Directory argument */
    if(cmd==PACK_DIR || cmd==PACK_MOVE)
    {
	p = xstrtok(NULL, " \t");
	if(!p)
	{
	    logit("packing: directory argument missing");
	    return NULL;
	}
	dir = strsave(p);
    }
    else
	dir = NULL;

    /* Archiver name argument */
    if(cmd!=PACK_MOVE)
    {
	p = xstrtok(NULL, " \t");
	if(!p)
	{
	    logit("packing: archiver name argument missing");
	    return NULL;
	}
	if((a = parse_arc(p)) == NULL)
	{
	    logit("packing: unknown archiver/program %s", p);
	    return NULL;
	}
    }
    else
	a = NULL;
    
    /* List of nodes follows */
    node_invalid(&old);
    old.zone = cf_zone();
    lon_init(&lon);
	
    p = xstrtok(NULL, " \t");
    if(!p)
    {
	logit("packing: node address argument missing");
	return NULL;
    }
    while(p)
    {
	if(znfp_parse_diff(p, &node, &old) == ERROR)
	{
	    logit("packing: illegal node address %s", p);
	}
	else
	{
	    old = node;
	    lon_add(&lon, &node);
	}
	    
	p = xstrtok(NULL, " \t");
    }

    /* Create new entry and put into list */
    r = (Packing *)xmalloc(sizeof(Packing));
    r->pack  = cmd;
    r->dir   = dir;
    r->arc   = a;
    r->nodes = lon;
    r->next  = NULL;
	
    debug(15, "packing: pack=%c dir=%s arc=%s",
	  r->pack, r->dir ? r->dir : "", (r->arc ? r->arc->name : NULL));
    lon_debug(15, "packing: nodes=", &r->nodes, TRUE);

    return r;
}


int packing_do_file(char*name)
{
    FILE *fp;
    Packing *r;

    debug(14, "Reading packing file %s", name);

    fp = xfopen(name, R_MODE);

    while(cf_getline(buffer, BUFFERSIZE, fp))
    {
	r = packing_parse_line(buffer);
	if(!r)
	    continue;
	
	/* Put into linked list */
	if(packing_first)
	    packing_last->next = r;
	else
	    packing_first      = r;
	packing_last = r;
    }

    fclose(fp);

    return OK;
}


void packing_init(char *name)
{
    packing_do_file(name);
}



/*
 * Return name for ArcMail archive
 */
char *arcmail_name(Node *node, char *dir)
{
    static char buf[MAXPATH];
    static char *wk0[] = { "su0", "mo0", "tu0", "we0", "th0", "fr0", "sa0" };
    char *wk, *base;
    TIMEINFO ti;
    struct tm *tm;
#ifndef AMIGADOS_4D_OUTBOUND
    int d1, d2;
#endif
    size_t rest;

    cf_set_zone(node->zone);

    if(dir)
    {
	/* Passed directory name */
	BUF_COPY(buf, dir);
	if(buf[str_last(buf, sizeof(buf))] != '/')
	    BUF_APPEND(buf, "/");
    }
    else
    {
	/* Outbound dir + zone dir */
	BUF_COPY2(buf, cf_p_btbasedir(), "/");
	if((base = cf_zones_out(node->zone)) == NULL)
	    return NULL;
	BUF_APPEND(buf, base);
	BUF_APPEND(buf, "/");
    }
    base = buf + strlen(buf);
    rest = sizeof(buf) - strlen(buf);

    /*
     * Get weekday archive extension
     */
    GetTimeInfo(&ti);
    tm = localtime(&ti.time);
    wk = wk0[tm->tm_wday];

    /*
     * Create name of archive file
     */
#ifdef AMIGADOS_4D_OUTBOUND
    str_printf(base, rest, "%d.%d.%d.%d.%s", node->zone, node->net,
	       node->node, node->point, wk);
#else    
    if(node->point)
    {
	d1 = 0;
	d2 = (cf_main_addr()->point - node->point) & 0xffff;

	if(dir)
	    str_printf(base, rest, "%04x%04x.%s", d1, d2, wk );
	else
	    str_printf(base, rest, "%04x%04x.pnt/%04x%04x.%s",
		       node->net, node->node, d1, d2, wk );
    }
    else
    {
	d1 = (cf_main_addr()->net  - node->net ) & 0xffff;
	d2 = (cf_main_addr()->node - node->node) & 0xffff;
	
	str_printf(base, rest, "%04x%04x.%s", d1, d2, wk);
    }
#endif /**AMIGADOS_4D_OUTBOUND**/
    
    return buf;
}



/*
 * Return packet name for adding to archive
 */
char *packing_pkt_name(char *dir, char *name)
{
    static char buf[MAXPATH];
    char *p;
    
#if 0
    /* Return nnnnnnnn.pkt in dir */
    str_printf(buf, sizeof(buf), "%s/%08ld.pkt",
	       (dir ? dir : out_dir), sequencer(DEFAULT_SEQ_PACK));
#endif

    /* Same base name in dir */
    p = strrchr(name, '/');
    if(!p)				/* Just to be sure */
	p = name;
    else
	p++;
    BUF_COPY3(buf, (dir ? dir : out_dir), "/", p);
    
    return buf;
}



/*
 * Search for existing ArcMail archives in the outbound and changes
 * name to the latest one. If an empty, truncated ArcMail archive
 * exists, it is deleted and the name is set to the next number.
 */
int arcmail_search(char *name)
{
    int c, cc;
    char *p;
    long size;
    int is_old;
    
    p  = name + strlen(name) - 1;		/* Position of digit */
    cc = 0;
    
    for(c='9'; c>='0'; c--)			/* Digits `9' ... `0' */
    {
	*p = c;
	
	if(check_access(name, CHECK_FILE) == TRUE)	/* Found it */
	{
	    if( (size = check_size(name)) == ERROR )
	    {
		logit("$ERROR: can't stat %s", name);
		continue;
	    }
	    if(size == 0)			/* Empty archive */
	    {
		is_old = check_old(name, 24*60*60L);/* > 24 h */

		if(unlink(name) == ERROR)
		    logit("$ERROR: can't remove %s", name);
		debug(4, "Removed %s", name);
		if(is_old)
		    continue;

		if(!cc)
		    cc = c < '9' ? c + 1 : c;	/* Next number, mo0 -> mo1 */
	    }
	    if(maxarc && size>=maxarc)		/* Max size exceeded */
	    {
		if(!cc)
		    cc = c < '9' ? c + 1 : c;	/* Next number, mo0 -> mo1 */
	    }
	    if(!cc)
		cc = c;
	}
    }

    if(cc)
	*p = cc;				/* Set archive name digit */

    return OK;
}



/*
 * Pack packet to ArcMail archive
 */
int do_arcmail(char *name, Node *arcnode, Node *flonode,
	       PktDesc *desc, FILE *file, char *prog, char *dir)
{
    char *arcn, *pktn;
    int ret, newfile;
    
    arcn = arcmail_name(arcnode, dir);
    pktn = ffx_flag ? name : packing_pkt_name(NULL, name);
    if(!arcn)
	return ERROR;
    if(arcmail_search(arcn) == ERROR)
	return ERROR;
    
    if(bink_mkdir(arcnode) == ERROR)
    {
	logit("ERROR: can't create outbound dir");
	return ERROR;
    }
    
    debug(4, "Archiving %s for %s arc", name, znfp1(arcnode));
    debug(4, "    Packet  name: %s", pktn);
    debug(4, "    Archive name: %s", arcn);

    newfile = check_access(arcn, CHECK_FILE) == ERROR;

    if(!ffx_flag)
    {
	/* Rename/copy packet */
	if(desc->type == TYPE_ECHOMAIL)
	{
	    fclose(file);
	    /* Simply rename */
	    if(rename(name, pktn) == -1)
	    {
		logit("$ERROR: rename %s -> %s failed", name, pktn);
		return ERROR;
	    }
	}
	else
	{
	    /* Copy and process file attaches */
	    if(do_noarc(name, flonode, desc, file, pktn) == ERROR)
	    {
		logit("ERROR: copying/processing %s -> %s failed", name, pktn);
		return ERROR;
	    }
	}
    }
    
    str_printf(buffer, sizeof(buffer), prog, arcn, pktn);
    debug(4, "Command: %s", buffer);
    ret = run_system(buffer);
    debug(4, "Exit code=%d", ret);
    chmod(arcn, PACKET_MODE);
    if(ret)
    {
	logit("ERROR: %s failed, exit code=%d", buffer, ret);
	return ERROR;
    }
    chmod(arcn, PACKET_MODE);
    if(unlink(pktn) == -1)
	logit("$ERROR: can't remove %s", pktn);
    if(!dir)
	return bink_attach(flonode, '#', arcn,
			   flav_to_asc(desc->flav), FALSE );

    return OK;
}



/*
 * Write packet contents to .OUT packet in Binkley outbound
 */
int do_noarc(char *name, Node *flonode,
	     PktDesc *desc, FILE *pkt_file, char *out_name)
    /* outname --- name of .pkt file, if not NULL */
{
    FILE *fp;
    Message msg;
    Textlist tl;
    int type;

    /*
     * Open .OUT packet in Binkley outbound
     */
    cf_set_zone(desc->to.zone);

    fp = out_name
	     ? pkt_open(out_name, &desc->to, NULL, FALSE)
	     : pkt_open(NULL, &desc->to, flav_to_asc(desc->flav), FALSE);
    if(fp == NULL)
    {
	logit("ERROR: can't open outbound packet for %s",
	    znfp1(&desc->to)      );
	fclose(pkt_file);
	TMPS_RETURN(ERROR);
    }
    
    tl_init(&tl);

    /*
     * Read messages from packet
     */
    type = pkt_get_int16(pkt_file);
    while(type == MSG_TYPE)
    {
	/* Read message header */
	node_clear(&msg.node_from);
	node_clear(&msg.node_to);
	if(pkt_get_msg_hdr(pkt_file, &msg) != OK)
	{
	    logit("$ERROR reading input packet %s", name);
	    pkt_close();
	    fclose(pkt_file);
	    TMPS_RETURN(ERROR);
	}
	/* Read message body */
	type = pkt_get_body(pkt_file, &tl);
	if(type == ERROR)
	{
	    logit("$ERROR: reading input packet %s", name);
	    pkt_close();
	    fclose(pkt_file);
	    TMPS_RETURN(ERROR);
	}

	/* File attaches */
	if(desc->type==TYPE_NETMAIL && (msg.attr & MSG_FILE))
	{
	    char *fa_name = msg.subject;
	    int ret;
	    long sz;

	    /* File attachments from gateway (runtoss out) */
	    if(streq(file_attach_dir, "/"))
	    {
		/* fa_name in subject is complete path */
		    sz  = check_size(fa_name);
		    ret = bink_attach(flonode, 0, fa_name,
				      flav_to_asc(desc->flav), FALSE );
		    if(ret == ERROR)
			logit("ERROR: file attach %s for %s failed",
			    fa_name, znfp1(&desc->to));
		    else
			logit("file attach %s (%ldb) for %s",
			    fa_name, sz, znfp1(&desc->to));
	    }
	    /* File attachments from inbound directory */
	    else if(file_attach_dir[0])
	    {
		if(dir_search(file_attach_dir, fa_name))
		{
		    BUF_COPY3(buffer, file_attach_dir, "/", fa_name);
		    sz  = check_size(buffer);
		    ret = bink_attach(flonode, '^', buffer,
				      flav_to_asc(desc->flav), FALSE );
		    if(ret == ERROR)
			logit("ERROR: file attach %s for %s failed",
			    msg.subject, znfp1(&desc->to));
		    else
			logit("file attach %s (%ldb) for %s",
			    msg.subject, sz, znfp1(&desc->to));
		}
		else
		    logit("file attach %s: no such file", msg.subject);
	    }
	    else
		logit("file attach %s not processed, no -F option", fa_name);
	}

	/* Write message header */
	if( pkt_put_msg_hdr(fp, &msg, FALSE) != OK )
	{
	    logit("$ERROR: writing packet %s", pkt_name());
	    pkt_close();
	    fclose(pkt_file);
	    TMPS_RETURN(ERROR);
	}
	/* Write message body */
	tl_print(&tl, fp);
	/* Terminating 0-byte */
	putc(0, fp);
	if(ferror(fp) != OK)
	{
	    logit("$ERROR: writing packet %s", pkt_name());
	    pkt_close();
	    fclose(pkt_file);
	    TMPS_RETURN(ERROR);
	}
	
	tmps_freeall();
    } /**while**/

    fclose(pkt_file);
    if(pkt_close() != OK)
    {
	logit("$ERROR: can't close outbound packet");
	TMPS_RETURN(ERROR);
    }
    if(unlink(name) != OK)
    {
	logit("$ERROR: can't remove packet %s", name);
	TMPS_RETURN(ERROR);
    }
    
    TMPS_RETURN(OK);
}



/*
 * Call program for packet file
 */
int do_prog(char *name, PktDesc *desc, Packing *pack)
{
    int ret;
    
    debug(4, "Processing %s", name);

    str_printf(buffer, sizeof(buffer), pack->arc->prog, name);
    debug(4, "Command: %s", buffer);
    ret = run_system(buffer);
    debug(4, "Exit code=%d", ret);
    if(ret)
    {
        logit("ERROR: %s failed, exit code=%d", buffer, ret);
        return ERROR;
    }
    if(pack->arc->pack != PACK_PROGN)
	if(unlink(name)==ERROR)
	    logit("$ERROR: can't remove %s", name);

    return OK;
}



/*
 * Set all -1 Node components to 0
 */
void set_zero(Node *node)
{
    if(node->zone  == -1)
	node->zone  = 0;
    if(node->net   == -1)
	node->net   = 0;
    if(node->node  == -1)
	node->node  = 0;
    if(node->point == -1)
	node->point = 0;
}



/*
 * Pack packet to ArcMail archive or copy to outbound .OUT
 */
int do_pack(PktDesc *desc, char *name, FILE *file, Packing *pack)
{
    int ret = OK;
    Node arcnode, flonode;
    
    arcnode = desc->to;
    flonode = desc->to;
    
    if(pack->pack == PACK_ROUTE)
    {
	/* New archive node */
	arcnode = pack->nodes.first->node;
	/* New FLO node */
	flonode = pack->nodes.first->node;
    }
    if(pack->pack == PACK_FLO)
    {
	/* New FLO node */
	flonode = pack->nodes.first->node;
    }

    /* Set all -1 values to 0 */
    set_zero(&arcnode);
    set_zero(&flonode);
    

    /* Create BSY file(s) */
    if((!ffx_flag || !node_eq(&arcnode, &ffx_node)) &&
       bink_bsy_create(&arcnode, NOWAIT) == ERROR)
    {
	debug(1, "%s busy, skipping", znfp1(&arcnode));
	if(file)
	    fclose(file);
	return OK;			/* This is o.k. */
    }
    if(!node_eq(&arcnode, &flonode) &&
       bink_bsy_create(&flonode, NOWAIT) == ERROR)
    {
	debug(1, "%s busy, skipping", znfp1(&flonode));
	if(file)
	    fclose(file);
	bink_bsy_delete(&arcnode);
	return OK;			/* This is o.k. */
    }
       
    /* Do the various pack functions */
    if(pack->arc && pack->arc->pack==PACK_ARC)
    {
	if( ffx_flag ||
	    (desc->flav!=FLAV_CRASH && pack->arc->prog) )
	{
	    if(pack->pack == PACK_ROUTE)
		logit("archiving packet (%ldb) for %s via %s arc (%s)",
		    check_size(name),
		    znfp1(&desc->to), znfp2(&arcnode),
		    pack->arc->name );
	    else if(pack->pack == PACK_FLO)
		logit("archiving packet (%ldb) for %s via %s flo (%s)",
		    check_size(name),
		    znfp1(&desc->to), znfp2(&flonode),
		    pack->arc->name );
	    else
		logit("archiving packet (%ldb) for %s (%s)",
		    check_size(name),
		    znfp1(&desc->to), pack->arc->name );

	    ret = do_arcmail(name, &arcnode, &flonode, desc,
			     file, pack->arc->prog, NULL);
	}
	else
	{
	    logit("packet (%ldb) for %s (noarc)",
		check_size(name), znfp1(&desc->to));
	    ret = do_noarc(name, &desc->to, desc, file, NULL);
	}
    }
    else
    {
	if(file)
	    fclose(file);
	logit("packet (%ldb) for %s (%s)",
	    check_size(name), znfp1(&desc->to), pack->arc->name);
	ret = do_prog(name, desc, pack);
    }
    
    /* Delete BSY file */
    if(!ffx_flag || !node_eq(&arcnode, &ffx_node))
	bink_bsy_delete(&arcnode);
    if(!node_eq(&arcnode, &flonode))
	bink_bsy_delete(&flonode);
    
    return ret;
}



/*
 * Pack packet to ArcMail archive in separate directory
 */
int do_dirpack(PktDesc *desc, char *name, FILE *file, Packing *pack)
{
    int ret = OK;
    Node arcnode, flonode;
    char *pktn;
    
    arcnode = desc->to;
    flonode = desc->to;
    
    /* Set all -1 values to 0 */
    set_zero(&arcnode);
    set_zero(&flonode);
    
    /* Create BSY file(s) */
    if(!ffx_flag)
	if(bink_bsy_create(&arcnode, NOWAIT) == ERROR)
	{
	    debug(1, "%s busy, skipping", znfp1(&arcnode));
	    if(file)
		fclose(file);
	    return OK;			/* This is o.k. */
	}

    /* dirpack */
    if(pack->pack==PACK_DIR && pack->arc && pack->arc->pack==PACK_ARC)
    {
	if(pack->arc->prog)
	{
	    logit("archiving packet (%ldb) for %s (%s) in %s",
		check_size(name), znfp1(&desc->to), pack->arc->name, pack->dir);

	    ret = do_arcmail(name, &arcnode, &flonode, desc,
			     file, pack->arc->prog, pack->dir);
	}
    }

    /* dirmove */
    if(pack->pack==PACK_MOVE)
    {
	logit("moving packet (%ldb) for %s to %s",
	    check_size(name), znfp1(&desc->to), pack->dir);

	pktn = packing_pkt_name(pack->dir, name);
	ret = do_noarc(name, &flonode, desc, file, pktn);
    }
    
    /* Delete BSY file */
    if(!ffx_flag)
	bink_bsy_delete(&arcnode);
    
    return ret;
}



/*
 * Pack packet
 */
int do_packing(char *name, FILE *fp, Packet *pkt)
{
    PktDesc *desc;
    PktDesc pktdesc;
    Packing *r;
    LNode *p;
    int ret;

    if(pkt_flag || ffx_flag)
    {
	/* Unknown grade/type for .pkt's or ffx files */
	pktdesc.from  = pkt->from;
	pktdesc.to    = pkt->to;
	pktdesc.grade = '-';
	pktdesc.type  = '-';
	pktdesc.flav  = FLAV_NORMAL;
	
	desc = &pktdesc;
    }
    else 
    {
	/* Parse description from filename */
	desc = parse_pkt_name(name, &pkt->from, &pkt->to);
	if(desc == NULL)
	    TMPS_RETURN(ERROR);
    }
    
    debug(2, "Packet: from=%s to=%s grade=%c type=%c flav=%c",
	  znfp1(&desc->from), znfp2(&desc->to),
	  desc->grade, desc->type, desc->flav);

    /*
     * Search for matching packing commands
     */
    for(r=packing_first; r; r=r->next)
	for(p=r->nodes.first; p; p=p->next)
	    if(node_match(&desc->to, &p->node))
	    {
		debug(3, "packing: pack=%c dir=%s arc=%s",
		      r->pack, r->dir ? r->dir : "",
		      (r->arc ? r->arc->name : NULL)      );
		ret = r->dir ? do_dirpack(desc, name, fp, r)
		             : do_pack   (desc, name, fp, r);
		TMPS_RETURN(ret);
	    }

    TMPS_RETURN(OK);
}



/*
 * Process one packet file
 */
int do_file(char *pkt_name)
{
    Packet pkt;
    FILE *pkt_file;

    if(ffx_flag)
    {
	/* ffx files are not packets, really ;-) */
	pkt.to = ffx_node;
	cf_set_zone(ffx_node.zone);
	pkt.from = cf_n_addr();
	pkt_file = NULL;
    }
    else 
    {
	/* Open packet and read header */
	pkt_file = fopen(pkt_name, R_MODE);
	if(!pkt_file) {
	    logit("$ERROR: can't open packet %s", pkt_name);
	    TMPS_RETURN(severe_error);
	}
	if(pkt_get_hdr(pkt_file, &pkt) == ERROR)
	{
	    logit("ERROR: reading header from %s", pkt_name);
	    TMPS_RETURN(severe_error);
	}
    }
    
    /* Pack it */
    if(do_packing(pkt_name, pkt_file, &pkt) == ERROR) 
    {
	logit("ERROR: processing %s", pkt_name);
	TMPS_RETURN(severe_error);
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
options: -B --binkley NAME            set Binkley outbound directory\n\
         -f --ffx Z:N/F.P             process ffx control and data files\n\
         -F --file-dir NAME           set directory to search for f/a\n\
         -g --grade G                 processing grade\n\
         -I --in-dir NAME             set input packet directory\n\
         -l --lock-file               create lock file while processing\n\
         -m --maxarc KSIZE            set max archive size (KB)\n\
         -O --out-dir NAME            set output packet directory\n\
         -p --packing-file NAME       read packing file\n\
         -P --pkt                     process .pkt's in input directory\n\
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
    char *I_flag=NULL, *O_flag=NULL, *B_flag=NULL, *p_flag=NULL;
    char *c_flag=NULL;
    char *a_flag=NULL, *u_flag=NULL;
    char *pkt_name;
    char pattern[16];
    
    int option_index;
    static struct option long_options[] =
    {
	{ "binkley",      1, 0, 'B'},	/* Binkley outbound base dir */
	{ "ffx",          1, 0, 'f'},	/* ffx mode */
	{ "file-dir",     1, 0, 'F'},	/* Dir to search for file attaches */
	{ "grade",        1, 0, 'g'},	/* grade */
	{ "in-dir",       1, 0, 'I'},	/* Set inbound packets directory */
	{ "lock-file",    0, 0, 'l'},	/* Create lock file while processing */
	{ "out-dir",      1, 0, 'O'},	/* Set packet directory */
	{ "packing-file", 1, 0, 'p'},	/* Set packing file */
	{ "maxarc",       1, 0, 'm'},	/* Set max archive size */
	{ "pkt",          0, 0, 'P'},	/* Process .pkt's */
	
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


    while ((c = getopt_long(argc, argv, "B:f:F:g:I:O:lp:m:Pvhc:a:u:",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	/***** ftnpack options *****/
	case 'B':
	    B_flag = optarg;
	    break;
	case 'f':
	    ffx_flag = TRUE;
	    if(asc_to_node(optarg, &ffx_node, FALSE) == ERROR)
	    {
		fprintf(stderr, "%s: invalid -f %s", PROGRAM, optarg);
		exit(EX_USAGE);
	    }
	    break;
	case 'F':
	    BUF_COPY(file_attach_dir, optarg);
	    break;
	case 'g':
	    g_flag = *optarg;
	    break;
	case 'I':
	    I_flag = optarg;
	    break;
	case 'O':
	    O_flag = optarg;
	    break;
        case 'l':
            l_flag = TRUE;
            break;
	case 'p':
	    p_flag = optarg;
	    break;
	case 'm':
	    maxarc = atol(optarg) * 1024L;
	    break;
	case 'P':
	    pkt_flag = TRUE;
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
    if(B_flag)
	cf_s_btbasedir(B_flag);
    if(a_flag)
	cf_set_addr(a_flag);
    if(u_flag)
	cf_set_uplink(u_flag);

    cf_debug();
    
    /*
     * Process optional config statements
     */
    if(!maxarc && (p = cf_get_string("MaxArc", TRUE)))
    {
	debug(8, "config: MaxArc %s", p);
	maxarc = atol(p) * 1024L;
    }

    /*
     * Process local options
     */
    BUF_EXPAND(in_dir , I_flag ? I_flag : DEFAULT_TOSS_ROUTE);
    BUF_EXPAND(out_dir, O_flag ? O_flag : DEFAULT_TOSS_PACK);

    packing_init(p_flag ? p_flag : cf_p_packing() );
    passwd_init();

    /* Install signal/exit handlers */
    signal(SIGHUP,  prog_signal);
    signal(SIGINT,  prog_signal);
    signal(SIGQUIT, prog_signal);


    ret = EXIT_OK;
    
    if(optind >= argc)
    {
	if(ffx_flag)
	{
	    BUF_COPY(pattern, "f???????*");
	}
	else if(pkt_flag)
	{
	    BUF_COPY(pattern, "*.pkt");
	}
	else 
	{
	    BUF_COPY(pattern, "????????.pkt");
	    if(g_flag)
		pattern[0] = g_flag;
	}
	
	/* Lock file */
	if(l_flag)
	    if(lock_program(PROGRAM, FALSE) == ERROR)
		exit(EXIT_BUSY);
	/* BSY file */
	if(ffx_flag)
	    if(bink_bsy_create(&ffx_node, NOWAIT) == ERROR)
		exit(EXIT_BUSY);

	/* Process packet files in directory */
	dir_sortmode(DIR_SORTMTIME);
	if(dir_open(in_dir, pattern, TRUE) == ERROR)
	{
	    logit("$ERROR: can't open directory %s", in_dir);
	    ret = EX_OSERR;
	}
	else 
	{
	    for(pkt_name=dir_get(TRUE); pkt_name; pkt_name=dir_get(FALSE))
	    {
		if(do_file(pkt_name) == ERROR)
		{
		    ret = EXIT_ERROR;
		    break;
		}
		tmps_freeall();
	    }
	    dir_close();
	}
	
	/* Lock file */
	if(l_flag)
	    unlock_program(PROGRAM);
	/* BSY file */
	if(ffx_flag)
	    bink_bsy_delete(&ffx_node);
    }
    else
    {
	/* Lock file */
	if(l_flag)
	    if(lock_program(PROGRAM, FALSE) == ERROR)
		exit(EXIT_BUSY);
	/* BSY file */
	if(ffx_flag)
	    if(bink_bsy_create(&ffx_node, NOWAIT) == ERROR)
		exit(EXIT_BUSY);
	
	/* Process packet files on command line */
	for(; optind<argc; optind++)
	{
	    if(do_file(argv[optind]) == ERROR)
	    {
		ret = EXIT_ERROR;
		break;
	    }
	    tmps_freeall();
	}
	
	/* Lock file */
	if(l_flag)
	    unlock_program(PROGRAM);
	/* BSY file */
	if(ffx_flag)
	    bink_bsy_delete(&ffx_node);
    }
    
    exit(ret);
}
