/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: kludge.c,v 4.10 2004/08/22 20:19:11 n0ll Exp $
 *
 * Processing of FTN ^A kludges in message body
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
 * Process the addressing kludge lines in the message body:
 * ^ATOPT, ^AFMPT, ^AINTL. Remove these kludges from MsgBody and put
 * the information in Message.
 */
void kludge_pt_intl(MsgBody *body, Message *msg, int del)
{
    Textline *line;
    Textlist *list;
    char *p, *s;
    Node node;
    
    list = &body->kludge;
    
    /* ^AINTL */
    if( (p = kludge_get(list, "INTL", &line)) )
    {
	p = strsave(p);

	/* Retrieve addresses from ^AINTL */
	if( (s = strtok(p, " \t\r\n")) )	/* Destination */
	    if( asc_to_node(s, &node, FALSE) == OK )
		msg->node_to = node;
	if( (s = strtok(NULL, " \t\r\n")) )	/* Source */
	    if( asc_to_node(s, &node, FALSE) == OK )
		msg->node_from = node;

	xfree(p);
	
	if(del)
	    tl_delete(list, line);
    }
    
    /* ^AFMPT */
    if( (p = kludge_get(list, "FMPT", &line)) )
    {
	msg->node_from.point = atoi(p);

	if(del)
	    tl_delete(list, line);
    }
    
    /* ^ATOPT */
    if( (p = kludge_get(list, "TOPT", &line)) )
    {
	msg->node_to.point = atoi(p);

	if(del)
	    tl_delete(list, line);
    }
}



/*
 * Get first next kludge line
 */
char *kludge_getn(Textlist *tl, char *name, Textline **ptline, int first)
{
    static Textline *last_kludge;
    char *s, *r;
    int len;

    len = strlen(name);

    if(first)
	last_kludge = tl->first;
    
    while(last_kludge)
    {
	s = last_kludge->line;
	if(s[0] == '\001'                     &&
	   !strnicmp(s+1, name, len)          &&
	   ( s[len+1]==' ' || s[len+1]==':' )    )	/* Found it */
	{
	    r = s + 1 + len;
	    /* Skip ':' and white space */
	    if(*r == ':')
		r++;
	    while( is_space(*r) )
		r++;
	    if(ptline)
		*ptline = last_kludge;
	    last_kludge = last_kludge->next;
	    
	    return r;
	}

	last_kludge = last_kludge->next;
    }
    
    /* Not found */
    if(ptline)
	*ptline = NULL;
    return NULL;
}



/*
 * Get first kludge line from a Textlist of all kludge lines.
 */
char *kludge_get(Textlist *tl, char *name, Textline **ptline)
{
    return kludge_getn(tl, name, ptline, TRUE);
}
