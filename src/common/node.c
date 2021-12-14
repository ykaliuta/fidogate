/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: node.c,v 4.16 2004/08/22 20:19:11 n0ll Exp $
 *
 * Conversion Node structure <-> Z:N/F.P / pP.fF.nN.zZ
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



/*
 * pfnz_to_node() --- Convert p.f.n.z domain address to Node struct
 */
int pfnz_to_node(char *pfnz, Node *node, int point_flag)
{
    char *s;
    int val, c;

    node->zone = node->net = node->node = node->point = -1;
    
    s = pfnz;

    debug(8, "pfnz_to_node(): %s", s);

    /*
     * Convert to Node
     */
    while(*s)
	if(strchr("pPfFnNzZ", *s))
	{
	    c = *s++;
	    c = tolower(c);

	    /* Value */
	    if( (val = znfp_get_number(&s)) < 0 )
		return ERROR;
	    /* '.' or '\0' expected */
	    if(*s!='.' && *s!='\0')
		return ERROR;
	    if(*s)
		s++;
	    
	    /* Put into Node field */
	    switch(c)
	    {
	    case 'p':
		node->point = val;
		break;
	    case 'f':
		node->node  = val;
		break;
	    case 'n':
		node->net   = val;
		break;
	    case 'z':
		node->zone  = val;
		break;
	    default:
		/**NOT REACHED**/
		return ERROR;
	    }
	}
	else
	    return ERROR;

    /*
     * Check address
     */
    if(node->zone == -1)
	node->zone = cf_defzone();
    if(node->net == -1)
    {
	return ERROR;
    }
    if(node->node == -1)
    {
	return ERROR;
    }
    if(node->point == -1 && !point_flag)
	node->point = 0;

    debug(8, "pfnz_to_node(): %d:%d/%d.%d", node->zone, node->net,
	  node->node, node->point );
    
    return OK;
}



/*
 * asc_to_node() --- Convert Z:N/F.P address to Node struct
 */
int asc_to_node(char *asc, Node *node, int point_flag)
{
    if(znfp_parse_partial(asc, node) == ERROR)
	return ERROR;

    /* Wildcards are not allowed here */
    if(node->zone==WILDCARD || node->net==WILDCARD ||
       node->node==WILDCARD || node->point==WILDCARD)
	return ERROR;

    /* Empty zone, use default */
    if(node->zone==EMPTY)
	node->zone = cf_defzone();
    
    /* Net, node must be set */
    if(node->net==EMPTY || node->node==EMPTY)
	return ERROR;

    /* Empty point, set to 0 if point_flag==FALSE */
    if(node->point==EMPTY && !point_flag)
	node->point = 0;

    return OK;
}



/*
 * node_to_pfnz() --- Convert Node struct to p.f.n.z notation
 */
char *node_to_pfnz(Node *node, int point_flag)
{
    static char buf[MAXINETADDR];

    if(node->point!=-1 && (node->point!=0 || point_flag))
	str_printf(buf, sizeof(buf), "p%d.f%d.n%d.z%d",
		   node->point, node->node, node->net, node->zone );
    else
	str_printf(buf, sizeof(buf), "f%d.n%d.z%d",
		   node->node, node->net, node->zone);

    return buf;
}



/*
 * node_eq() --- compare node adresses
 */
int node_eq(Node *a, Node *b)
{
    return
	a->zone == b->zone && a->net   ==b->net   &&
	a->node == b->node && a->point ==b->point   ;
}

int node_np_eq(Node *a, Node *b)
{
    return
	a->zone == b->zone && a->net ==b->net && a->node == b->node;
}



/*
 * node_clear() --- clear node structure
 */
void node_clear(Node *n)
{
    n->zone = n->net = n->node = n->point = 0;
    n->domain[0] = 0;
}



/*
 * Set node to -1
 */
void node_invalid(Node *n)
{
    n->zone = n->net = n->node = n->point = -1;
    n->domain[0] = 0;
}



/*
 * Convert partial Z:N/F.P address to Node, using previous node address
 */
int asc_to_node_diff(char *asc, Node *node, Node *oldnode)
{
    if(znfp_parse_diff(asc, node, oldnode) == ERROR)
	return ERROR;
    
    /* Wildcards are not allowed here */
    if(node->zone==WILDCARD || node->net==WILDCARD ||
       node->node==WILDCARD || node->point==WILDCARD)
	return ERROR;

    /* Zone, net, node must be set */
    if(node->zone==EMPTY || node->net==EMPTY || node->node==EMPTY)
	return ERROR;
	    
    /* Empty point is 0 */
    if(node->point==EMPTY)
	node->point = 0;

    return OK;
}



/*
 * Output Node as Z:N/F.P in short form using diff to previous output
 */
char *node_to_asc_diff(Node *node, Node *oldnode)
{
    static char buf[MAXINETADDR];

    if(node->zone == oldnode->zone)
    {
	if(node->net == oldnode->net)
	{
	    if(node->node == oldnode->node)
		str_printf(buf, sizeof(buf), node->point ? ".%d" : "",
			   node->point                      );
	    else
		str_printf(buf, sizeof(buf), node->point ? "%d.%d" : "%d",
			node->node, node->point              );
	}
	else
	    str_printf(buf, sizeof(buf), node->point ? "%d/%d.%d" : "%d/%d",
		       node->net, node->node, node->point         );
    }
    else
	str_printf(buf, sizeof(buf), node->point ? "%d:%d/%d.%d" : "%d:%d/%d",
		   node->zone, node->net, node->node, node->point   );
    
    return buf;
}



/*
 * Compare to node addresses for sorting
 *
 * -1  -  a < b
 *  0  -  a = b
 *  1  =  a > b
 */
int node_cmp(Node *a, Node *b)
{
    if(a->zone  < b->zone )
	return -1;
    if(a->zone  > b->zone )
	return 1;
    if(a->net   < b->net  )
	return -1;
    if(a->net   > b->net  )
	return 1;
    if(a->node  < b->node )
	return -1;
    if(a->node  > b->node )
	return 1;
    if(a->point < b->point)
	return -1;
    if(a->point > b->point)
	return 1;
    return 0;
}


/***** List Of Nodes handling ************************************************/

/*
 * Initialize list of nodes
 */
void lon_init(LON *lon)
{
    lon->size = 0;
    lon->sorted = NULL;
    lon->first = lon->last = NULL;
}



/*
 * Delete list of nodes
 */
void lon_delete(LON *lon)
{
    LNode *p, *n;
    
    xfree(lon->sorted);
    for(p=lon->first; p; )
    {
	n = p->next;
	xfree(p);
	p = n;
    }
    
    lon_init(lon);
}



/*
 * Add node to list of nodes
 */
void lon_add(LON *lon, Node *node)
{
    LNode *p;
    
    p = (LNode *)xmalloc(sizeof(LNode));
    
    p->node = *node;
    
    if(lon->first)
	lon->last->next = p;
    else
	lon->first = p;
    p->next = NULL;
    p->prev = lon->last;
    lon->last = p;

    lon->size++;
}



/*
 * Remove node from list of nodes
 */
int lon_remove(LON *lon, Node *node)
{
    LNode *p;
    
    for(p=lon->first; p; p=p->next)
	if(node_eq(&p->node, node))
	{
	    if(p == lon->first)
		lon->first = p->next;
	    if(p == lon->last)
		lon->last  = p->prev;
	    if(p->prev)
		p->prev->next = p->next;
	    if(p->next)
		p->next->prev = p->prev;

	    xfree(p);
	    lon->size--;

	    return TRUE;
	}
    
    return FALSE;
}



/*
 * Search node in list of nodes
 */
int lon_search(LON *lon, Node *node)
{
    LNode *p;
    
    for(p=lon->first; p; p=p->next)
	if(node_eq(&p->node, node))
	    return TRUE;
    
    return FALSE;
}



/*
 * Add nodes from string to list of nodes
 */
int lon_add_string(LON *lon, char *s)
{
    Node node, old;
    char *p, *save;
    int ret;
    
    old.zone = cf_zone();
    old.net = old.node = old.point = -1;

    save = strsave(s);
    
    ret = OK;
    for(p=strtok(save, " \t\r\n"); p; p=strtok(NULL, " \t\r\n"))
    {
	if( asc_to_node_diff(p, &node, &old) == OK )
	{
	    old = node;
	    lon_add(lon, &node);
	}
	else
	{
	    ret = ERROR;
	    break;
	}
    }

    xfree(save);
    return ret;
}



/*
 * Output list of nodes
 */
int lon_print(LON *lon, FILE *fp, int short_flag)
{
    LNode *p;
    Node old;

    node_invalid(&old);
    
    for(p=lon->first; p; p=p->next)
    {
	if(short_flag)
	{
	    fputs(node_to_asc_diff(&p->node, &old), fp);
	    old = p->node;
	}
	else
	    fputs(znf1(&p->node), fp);
	if(p->next)
	    fputs(" ", fp);
    }

    return ferror(fp);
}



/*
 * Create sorted array of nodes in list of nodes
 */
static int lon_sort_compare	(const void *, const void *);

static int lon_sort_compare(const void *a, const void *b)
#ifdef __STDC__
                      
#else
                
#endif
{
    return node_cmp(*(Node **)a, *(Node **)b);
}

void lon_sort(LON *lon, int off)
{
    LNode *p;
    int i, n = lon->size;

    xfree(lon->sorted);
    lon->sorted = NULL;
    if(n <= 0)				/* Really nothing to do */
	return;
    
    lon->sorted = (Node **)xmalloc(n * sizeof(Node *));
    
    for(i=0, p=lon->first; i<n && p; i++, p=p->next)
	lon->sorted[i] = &p->node;
    
    if(n <= off)			/* Nothing to do */
	return;
    
    qsort((void *)&lon->sorted[off], n-off, sizeof(Node *), lon_sort_compare);
}



/*
 * Output sorted list of nodes
 */
int lon_print_sorted(LON *lon, FILE *fp, int off)
{
    int i;
    Node old;
    
    node_invalid(&old);

    lon_sort(lon, off);
    
    for(i=0; i<lon->size; i++)
    {
	fputs(node_to_asc_diff(lon->sorted[i], &old), fp);
	old = *lon->sorted[i];
	if(i < lon->size - 1)
	    fputs(" ", fp);
    }

    xfree(lon->sorted);
    lon->sorted = NULL;
    
    return ferror(fp);
}



/*
 * Debug output for LON
 */
void lon_debug(int lvl, char *text, LON *lon, int short_flag)
{
    if(verbose >= lvl)
    {
	fputs(text, stderr);
	lon_print(lon, stderr, short_flag);
	fputs("\n", stderr);
    }
}



/*
 * Add entire LON
 */
void lon_join(LON *lon, LON *add)
{
    LNode *p;
    
    for(p=add->first; p; p=p->next)
	lon_add(lon, &p->node);
}
