/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * Read 'spyes list' from file. The spyes format is as follows:
 *	Z:N/F.P   Z:N/F.P
 *
 *****************************************************************************
 * Copyright (C) 2000
 *  
 * Oleg Derevenetz	     FIDO:	2:5025/3.4
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

#ifdef SPYES



/*
 * Local prototypes
 */
static int    anodeeq		(Node *, Node *);
static Spy    *spyes_parse_line	(char *);
static int    spyes_do_file	(char *);
static int    spyes_check_dups  (Node *);


/*
 * Spyes list
 */
static Spy *spyes_list = NULL;
static Spy *spyes_last = NULL;



/*
 * Read list of spy nodes from CONFIGDIR/SPYES file.
 *
 * Format:
 *     NODE	FORWARD_NODE
 */
static Spy *spyes_parse_line(char *buf)
{
    Spy *p;
    char *n, *r;
    Node node, forward_node;
    
    n = xstrtok(buf,  " \t");	/* Node */
    r = xstrtok(NULL, " \t");	/* Forward node */
    if(n==NULL || r==NULL)
	return NULL;
    if(strieq(n, "include")) 
    {
	spyes_do_file(r);
	return NULL;
    }
    
    if( asc_to_node(n, &node, TRUE) == ERROR )
    {
	log("spyes: illegal FTN address %s", n);
	return NULL;
    }
    if( spyes_check_dups(&node) )
    {
	log("spyes: duplicate spy entry %s", n);
	return NULL;
    }
    if( asc_to_node(r, &forward_node, TRUE) == ERROR )
    {
	log("spyes: illegal FTN address %s", r);
	return NULL;
    }
    
    p = (Spy *)xmalloc(sizeof(Spy));
    p->next     	= NULL;
    p->node     	= node;
    p->forward_node     = forward_node;
    
    debug(15, "spyes: %s %s", znfp1(&p->node), znfp2(&p->forward_node));

    return p;
}


static int spyes_do_file(char *name)
{
    FILE *fp;
    Spy *p;

    debug(14, "Reading spyes file %s", name);
    
    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if(!fp)
	return ERROR;
    
    while(cf_getline(buffer, BUFFERSIZE, fp))
    {
	p = spyes_parse_line(buffer);
	if(!p)
	    continue;
	
	/*
	 * Put into linked list
	 */
	if(spyes_list)
	    spyes_last->next = p;
	else
	    spyes_list       = p;
	spyes_last       = p;
    }
    
    fclose(fp);

    return OK;
}

static int spyes_check_dups(Node *node)
{
    Spy *a;
    
    for(a=spyes_list; a; a=a->next)
    {
	if (anodeeq(node, &a->node))
	    return TRUE;
    }
    
    return FALSE;
}


void spyes_init(void)
{
    spyes_do_file( cf_p_spyes() );
}



/*
 * Lookup spy in spyes_list
 *
 * Parameters:
 *     node --- lookup by FTN node for forward_node
 *
 */
Spy *spyes_lookup(Node *node)
{
    Spy *a;
    
    for(a=spyes_list; a; a=a->next)
    {
	if (wild_compare_node(node, &a->node))
	    return a;
    }
    
    return NULL;
}

/*
 * anodeeq() --- compare node adresses
 *
 * Compare complete FTN addresses.
 *
 */
static int anodeeq(Node *a, Node *b)
{
    return a->zone==b->zone && a->net==b->net && a->node==b->node &&
	   a->point==b->point;
}

#endif /* SPYES */
