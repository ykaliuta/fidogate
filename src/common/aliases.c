/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: aliases.c,v 4.15 2004/08/22 20:19:11 n0ll Exp $
 *
 * Read user name aliases from file. The alias.users format is as follows:
 *	username    Z:N/F.P    Full Name
 *
 *****************************************************************************
 * Copyright (C) 1990-2004
 *  _____ _____
 * |	 |___  |   Martin Junius	     
 * | | | |   | |   Radiumstr. 18  	     Internet:	mj.at.n0ll.dot.net
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FIDOGATE; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *****************************************************************************/

#include "fidogate.h"



/*
 * Local prototypes
 */
static int    anodeeq		(Node *, Node *);
static Alias *alias_parse_line	(char *);
static int    alias_do_file	(char *);


/*
 * Alias list
 */
static Alias *alias_list = NULL;
static Alias *alias_last = NULL;



/*
 * Read list of aliases from LIBDIR/ALIASES file.
 *
 * Format:
 *     ALIAS	NODE	"FULL NAME"
 */
static Alias *alias_parse_line(char *buf)
{
    Alias *p;
    char *u, *n, *f;
    Node node;
    char *un, *ud;
    
    u = xstrtok(buf,  " \t");	/* User name */
    n = xstrtok(NULL, " \t");	/* FTN node */
    f = xstrtok(NULL, " \t");	/* Full name */
    if(u==NULL || n==NULL)
	return NULL;
    if(strieq(u, "include")) 
    {
	alias_do_file(n);
	return NULL;
    }
    if(f==NULL)
	return NULL;
    
    if( asc_to_node(n, &node, FALSE) == ERROR )
    {
	logit("hosts: illegal FTN address %s", n);
	return NULL;
    }
    
    p = (Alias *)xmalloc(sizeof(Alias));
    p->next     = NULL;
    p->node     = node;
    un = xstrtok(u,  "@");	/* User name */
    ud = xstrtok(NULL, " \t");	/* User domain */
    p->username = strsave(un);
    p->userdom  = ud ? strsave(ud) : NULL;
    p->fullname = strsave(f);
    
    if(p->userdom)
	debug(15, "aliases: %s@%s %s %s", p->username, p->userdom,
	      znfp1(&p->node), p->fullname);
    else
	debug(15, "aliases: %s %s %s", p->username, 
	      znfp1(&p->node), p->fullname);

    return p;
}


static int alias_do_file(char *name)
{
    FILE *fp;
    Alias *p;

    debug(14, "Reading aliases file %s", name);
    
    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if(!fp)
	return ERROR;
    
    while(cf_getline(buffer, BUFFERSIZE, fp))
    {
	p = alias_parse_line(buffer);
	if(!p)
	    continue;
	
	/*
	 * Put into linked list
	 */
	if(alias_list)
	    alias_last->next = p;
	else
	    alias_list       = p;
	alias_last       = p;
    }
    
    fclose(fp);

    return OK;
}


void alias_init(void)
{
    alias_do_file( cf_p_aliases() );
}



/*
 * Lookup alias in alias_list
 *
 * Parameters:
 *     node, username, NULL     --- lookup by FTN node and username
 *     node, NULL    , fullname --- lookup by FTN node and fullname
 *
 * The lookup for node is handled somewhat special: if node->point !=0,
 * then the alias matching the complete address including point will
 * be found. If not, then the alias comparison ignores the point address.
 * e.g.:
 *     mj    2:2452/110.1    "Martin Junius"
 * An alias_lookup(2:2452/110.1, "mj", NULL) as well as
 * alias_lookkup(2:2452/110, "mj", NULL) will return this alias entry.
 */
Alias *alias_lookup(Node *node, char *username, char *fullname)
{
    Alias *a;
    
    for(a=alias_list; a; a=a->next)
    {
	if(username)
	    if(!stricmp(a->username, username) && anodeeq(node, &a->node))
		return a;
	if(fullname)
	    if(!stricmp(a->fullname, fullname) && anodeeq(node, &a->node))
		return a;
    }
    
    return NULL;
}



Alias *alias_lookup_strict(Node *node, char *username, char *fullname)
{
    Alias *a;

    for(a=alias_list; a; a=a->next)
    {
	if(username)
	    if(!stricmp(a->username, username) && node_eq(node, &a->node))
		return a;
	if(fullname)
	    if(!stricmp(a->fullname, fullname) && node_eq(node, &a->node))
		return a;
    }
    for(a=alias_list; a; a=a->next)
    {
	if(username)
	    if(!stricmp(a->username, username) && node_np_eq(node, &a->node))
		return a;
	if(fullname)
	    if(!stricmp(a->fullname, fullname) && node_np_eq(node, &a->node))
		return a;
    }

    return NULL;
}

Alias *alias_lookup_userdom(Node *node, RFCAddr *rfc, char *fullname)
{
    Alias *a;

    if(!rfc)
	return NULL;

    for(a=alias_list; a; a=a->next)
    {
	if( a->userdom && (!stricmp(a->username, rfc->user) && !stricmp(a->userdom, rfc->addr)) )
	    return a;
    }

    return NULL;
}



/*
 * anodeeq() --- compare node adresses
 *
 * Special point treatment: if a->point != 0 and b->point !=0, compare the
 * complete FTN address including point. If not, compare ignoring the point
 * address.
 */
static int anodeeq(Node *a, Node *b)
            			/* Sender/receiver address */
            			/* ALIASES address */
{
    return a->point && b->point
	   ?
	   a->zone==b->zone && a->net==b->net && a->node==b->node &&
	   a->point==b->point
	   :
	   a->zone==b->zone && a->net==b->net && a->node==b->node
	   ;
}
