/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: rfcheader.c,v 4.14 2004/08/22 20:19:11 n0ll Exp $
 *
 * Functions to process RFC822 header lines from messages
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
 * Textlist to hold header of message
 */
static Textlist header = { NULL, NULL };
static Textline *last_header = NULL;



/*
 * header_ca_rfc() --- Output ^ARFC-Xxxx kludges
 */
int header_ca_rfc(FILE *out, int rfc_level)
{
    static char *rfc_lvl_1[] = { RFC_LVL_1_HEADERS, NULL };
    
    /* RFC level 0 - no ^ARFC-Xxxx kludges */
    if(rfc_level <= 0)
    {
	;
    }
    
    /* RFC level 1 - selected set of ^ARFC-Xxxx kludges */
    else if(rfc_level == 1)
    {
	char **name;
	Textline *p;
	int len;
	int ok = FALSE;
	
	for(p=header.first; p; p=p->next)
	{
	    if(!is_space(p->line[0])) 
	    {
		ok = FALSE;
		for(name=rfc_lvl_1; *name; name++)
		{
		    len  = strlen(*name);
		    if(!strnicmp(p->line, *name, len) && p->line[len]==':')
		    {
			ok = TRUE;		/* OK to output */
			break;
		    }
		}
	    }
	    if(ok)
		fprintf(out, "\001RFC-%s\r\n", p->line);
	}
    }
    
    /* RFC level 2 - all ^ARFC-Xxxx kludges */
    else if(rfc_level >= 2)
    {
	Textline *p;

	for(p=header.first; p; p=p->next)
	    fprintf(out, "\001RFC-%s\r\n", p->line);
    }
    
    return ferror(out);
}



/*
 * header_delete() --- Delete headers
 */
void header_delete(void)
{
    tl_clear(&header);
    last_header = NULL;
}



/*
 * header_read() --- read header lines from file
 */
void header_read(FILE *file)
{
    static char buf[BUFFERSIZE];
    
    tl_clear(&header);

    while(read_line(buf, sizeof(buf), file))
    {
	if(buf[0] == '\n')
	    break;
	strip_crlf(buf);
	tl_append(&header, buf);
    }
}



/*
 * header_hops() --- return # of hops (Received headers) of message
 */
int header_hops(void)
{
    char *name = "Received";
    Textline *p;
    int len, hops;
    
    len  = strlen(name);
    hops = 0;
    
    for(p=header.first; p; p=p->next)
    {
#ifdef RECEIVED_BY_MAILER
	if(!strnicmp(p->line, RECEIVED_BY_MAILER, strlen(RECEIVED_BY_MAILER)))
	    continue;
#endif
	if(!strnicmp(p->line, name, len) && p->line[len]==':')
	    hops++;
    }

    return hops;
}



/*
 * rfcheader_get() --- get header line
 */
char *rfcheader_get(Textlist *tl, char *name)
{
    Textline *p;
    int len;
    char *s;
    
    len = strlen(name);
    
    for(p=tl->first; p; p=p->next)
    {
	if(!strnicmp(p->line, name, len) && p->line[len]==':')
	{
	    for(s=p->line+len+1; is_space(*s); s++) ;
	    last_header = p;
	    return s;
	}
    }
    
    last_header = NULL;
    return NULL;
}



/*
 * header_get() --- get header line
 */
char *header_get(char *name)
{
    return rfcheader_get(&header, name);
}



/*
 * rfcheader_geth() --- get 1st/next header line from Textlist
 *
 * Modi:   name="X",  first=TRUE	return 1st X header line
 *	   name=NULL, first=FALSE	return continuation header lines only
 *	   name="X",  first=FALSE       return continuation header lines and
 *					new X header lines
 *
 * Return: contents of header lines or NULL.
 */
char *rfcheader_geth(Textlist *tl, char *name, int first)
{
    static Textline *p_last;
    Textline *p;
    int len;
    char *s;
    
    if(first)
    {
	/* Restart search */
	p_last = NULL;
	p      = tl->first;
    }
    else if(p_last)
    {
	/* Continue search */
	p_last = p_last->next;
        p      = p_last;
	/* Check for continuation header, white space at start of line */
	if(p_last && is_space(p_last->line[0])) 
	{
	    for(s=p_last->line; is_space(*s); s++) ;
	    return s;
	}
    }
    else
    {
	p = NULL;
    }
    
    /* If p or name is NULL, stop here */
    if(!p || !name)
    {
	p_last = NULL;
	return NULL;
    }
    
    /* Search for header line starting with NAME: */
    len = strlen(name);
    for(; p; p=p->next)
    {
	if(!strnicmp(p->line, name, len) && p->line[len]==':')
	{
	    for(s=p->line+len+1; is_space(*s); s++) ;
	    p_last = p;
	    return s;
	}
    }
    
    p_last = NULL;
    return NULL;
}



/*
 * header_geth() --- get header line
 */
char *header_geth(char *name, int first)
{
    return rfcheader_geth(&header, name, first);
}



/*
 * header_getnext() --- get next header line
 */

char *header_getnext(void)
{
    char *s;
    
    if(last_header == NULL)
	return NULL;

    last_header = last_header->next;
    if(last_header == NULL)
	return NULL;
    if(!is_space(last_header->line[0])) 
    {
	last_header = NULL;
	return NULL;
    }
    
    for(s=last_header->line; is_space(*s); s++) ;
    return s;
}



/*
 * Return complete header line, concat continuation lines if necessary.
 */
#define TMPS_LEN_GETCOMPLETE BUFFERSIZE

char *s_header_getcomplete(char *name)
{
    char *p;
    TmpS *s;

    if((p = header_get(name)))
    {
	s = tmps_alloc(TMPS_LEN_GETCOMPLETE);
	str_copy(s->s, s->len, p);

	while( (p = header_getnext()) )
	{
	    str_append(s->s, s->len, " ");
	    str_append(s->s, s->len, p);
	}
	
	tmps_stripsize(s);
	return s->s;
    }

    return NULL;
}



/*
 * addr_token() --- get addresses from string (',' separated)
 */
char *addr_token(char *line)
{
    static char *save_line = NULL;
    static char *save_p	   = NULL;
    int level;
    char *s, *p;
    
    if(line)
    {
	/*
	 * First call
	 */
	xfree(save_line);
	save_p = save_line = strsave(line);
    }

    if(save_p == NULL)
	return NULL;
    if(!*save_p) 
    {
	save_p = NULL;
	return NULL;
    }
    
    level = 0;
    for(p=s=save_p; *p; p++)
    {
	if(*p == '(')
	    level++;
	if(*p == ')')
	    level--;
	if(*p == ',' && level <= 0)
	    break;
    }
    if(*p)
	*p++ = 0;
    save_p = p;
	
    return s;
}   



#ifdef TEST /****************************************************************/

int main(int argc, char *argv[])
{
    char *h, *p;
    
    if(argc != 2)
    {
	fprintf(stderr, "usage: testheader header < RFC-messsage\n");
	exit(1);
    }
    
    h = argv[1];

    header_read(stdin);

    printf("----------------------------------------\n");
    for( p = rfcheader_geth(&header, h, TRUE);
	 p;
	 p = rfcheader_geth(&header, h, FALSE) )
    {
	printf("%s:    %s\n", h, p);
    }
    printf("----------------------------------------\n");
    for( p = header_geth(h, TRUE);
	 p;		 
	 p = header_geth(NULL, FALSE) )
    {
	printf("%s:    %s\n", h, p);
    }
    printf("----------------------------------------\n");
    
    exit(0);
    /**NOT REACHED**/
    return 0;
}

#endif /**TEST***************************************************************/
