/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: areasbbs.c,v 4.22 2004/08/22 20:19:11 n0ll Exp $
 *
 * Function for processing AREAS.BBS EchoMail distribution file.
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



static char     *areasbbs_1stline = NULL;
static AreasBBS *areasbbs_list    = NULL;
static AreasBBS *areasbbs_last    = NULL;



/*
 * Remove area from areas.bbs
 */
void areasbbs_remove(AreasBBS *cur, AreasBBS *prev)
{
    if(!cur)
	return;
    
    if(prev)
	prev->next = cur->next;
    else
	areasbbs_list = cur->next;
    if(areasbbs_last == cur)
	areasbbs_last = prev;
}



/*
 * Alloc and init new AreasBBS struct
 */
AreasBBS *areasbbs_new(void)
{
    AreasBBS *p;
    
    p = (AreasBBS *)xmalloc(sizeof(AreasBBS));

    /* Init */
    p->next  = NULL;
    p->flags = 0;
    p->dir   = NULL;
    p->area  = NULL;
    p->zone  = -1;
    node_invalid(&p->addr);
    p->lvl   = -1;
    p->key   = NULL;
    p->desc  = NULL;
    p->state = NULL;
    lon_init(&p->nodes);

    return p;
}



/*
 * Add nodes from string to list of nodes
 */
static int areasbbs_add_string(LON *lon, char *p)
{
    Node node, old;
    int ret;
    
    old.zone = cf_zone();
    old.net = old.node = old.point = -1;

    ret = OK;
    for(; p; p=xstrtok(NULL, " \t\r\n"))
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

    return ret;
}



/*
 * Create new AreasBBS struct for line from AREAS.BBS
 */
static AreasBBS *areasbbs_parse_line(char *line)
{
    AreasBBS *p;
    char *dir, *tag, *nl, *o2;
   
    dir = xstrtok(line, " \t\r\n");
    tag = xstrtok(NULL, " \t\r\n");
    if(!dir || !tag)
	return NULL;

    /* New areas.bbs entry */
    p = areasbbs_new();

    if(*dir == '#')
    {
	p->flags |= AREASBBS_PASSTHRU;
	dir++;
    }
    p->dir   = strsave(dir);
    p->area  = strsave(tag);
    
    /*
     * Options:
     *
     *     -a Z:N/F.P    alternate AKA for this area
     *     -z ZONE       alternate zone AKA for this area
     *     -l LVL        Areafix access level
     *     -k KEY        Areafix access key
     *     -d DESC       Area description text
     *     -#            Passthru
     +     -r            Read-only for new downlinks
     */
    nl  = xstrtok(NULL, " \t\r\n");
    while(nl && *nl=='-')
    {
	if(streq(nl, "-a"))		/* -a Z:N/F.P */
	{
	    o2 = xstrtok(NULL, " \t\r\n");
	    asc_to_node(o2, &p->addr, FALSE);
	}
	
	if(streq(nl, "-z"))		/* -z ZONE */
	{
	    o2 = xstrtok(NULL, " \t\r\n");
	    p->zone = atoi(o2);
	}
	
	if(streq(nl, "-l"))		/* -l LVL */
	{
	    o2 = xstrtok(NULL, " \t\r\n");
	    p->lvl = atoi(o2);
	}
	
	if(streq(nl, "-k"))		/* -k KEY */
	{
	    o2 = xstrtok(NULL, " \t\r\n");
	    p->key = strsave(o2);
	}
	
	if(streq(nl, "-d"))		/* -d DESC */
	{
	    o2 = xstrtok(NULL, " \t\r\n");
	    p->desc = strsave(o2);
	}
	
	if(streq(nl, "-s"))		/* -s STATE */
	{
	    o2 = xstrtok(NULL, " \t\r\n");
	    p->state = strsave(o2);
	}
	
	if(streq(nl, "-#"))		/* -# */
	{
	    p->flags |= AREASBBS_PASSTHRU;
	}
	
	if(streq(nl, "-r"))		/* -r */
	{
	    p->flags |= AREASBBS_READONLY;
	}

	nl  = xstrtok(NULL, " \t\r\n");
    }	
    
    areasbbs_add_string(&p->nodes, nl);

    if(p->zone == -1)
	p->zone = p->nodes.first ? p->nodes.first->node.zone : 0;
    
    return p;
}



/*
 * Read area distribution list from AREAS.BBS file
 *
 * Format:
 *    [#$]DIR AREA [-options] Z:N/F.P Z:N/F.P ...
 */
int areasbbs_init(char *name)
{
    FILE *fp;
    AreasBBS *p;

    if(!name)
	return ERROR;
    
    debug(14, "Reading %s file" , name);
    
    fp = fopen_expand_name(name, R_MODE, FALSE);
    if(!fp)
	return ERROR;
    
    /*
     * 1st line is special
     */
    if(fgets(buffer, BUFFERSIZE, fp))
    {
	strip_crlf(buffer);
	areasbbs_1stline = strsave(buffer);
    }

    /*
     * The following lines are areas and linked nodes
     */
    while(fgets(buffer, BUFFERSIZE, fp))
    {
	strip_crlf(buffer);
	p = areasbbs_parse_line(buffer);
	if(!p)
	    continue;
	
	debug(15, "areas.bbs: %s %s Z%d", p->dir, p->area, p->zone);

	/*
	 * Put into linked list
	 */
	if(areasbbs_list)
	    areasbbs_last->next = p;
	else
	    areasbbs_list       = p;
	areasbbs_last       = p;
    }

    fclose(fp);

    return OK;
}



/*
 * Output AREAS.BBS, format short sorted list of downlink
 */
int areasbbs_print(FILE *fp)
{
    AreasBBS *p;
    
    fprintf(fp, "%s\r\n", areasbbs_1stline);
    
    for(p=areasbbs_list; p; p=p->next)
    {
	if(p->flags & AREASBBS_PASSTHRU)
	    fprintf(fp, "#");
	fprintf(fp, "%s %s ", p->dir, p->area);
	if(p->zone != -1)
	    fprintf(fp, "-z %d ", p->zone);
	if(p->addr.zone != -1)
	    fprintf(fp, "-a %s ", znfp1(&p->addr));
	if(p->lvl != -1)
	    fprintf(fp, "-l %d ", p->lvl);
	if(p->key)
	    fprintf(fp, "-k %s ", p->key);
	if(p->desc)
	    fprintf(fp, "-d \"%s\" ", p->desc);
	if(p->state)
	    fprintf(fp, "-s %s ", p->state);
	lon_print_sorted(&p->nodes, fp, TRUE);
	fprintf(fp, "\r\n");
    }
    
    return ferror(fp);
}



/*
 * Return areasbbs_list
 */
AreasBBS *areasbbs_first(void)
{
    return areasbbs_list;
}



/*
 * Lookup area
 */
AreasBBS *areasbbs_lookup(char *area)
{
    AreasBBS *p;
    
    /**FIXME: the search method should use hashing or similar**/
    for(p=areasbbs_list; p; p=p->next)
    {
	if(area  && !stricmp(area,  p->area ))
	    return p;
    }
    
    return NULL;
}



/*
 * Add areas.bbs entry
 */
void areasbbs_add(AreasBBS *p)
{
    /* Put into linked list */
    if(areasbbs_list)
	areasbbs_last->next = p;
    else
	areasbbs_list       = p;
    areasbbs_last       = p;
}
