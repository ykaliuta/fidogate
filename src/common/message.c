/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: message.c,v 4.24 2004/08/22 20:19:11 n0ll Exp $
 *
 * Reading and processing FTN text body
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
 * Global flag for ignoring (removing) 0x8d (Soft-CR)
 */
int msg_ignore_0x8d = 1;



/*
 * Debug output of line
 */
static void debug_line(FILE *out, char *line, int crlf)
{
    int c;
    
    while( (c = *line++) )
	if( !(c & 0xe0) )
	{
	    if(crlf || (c!='\r' && c!='\n'))
		fprintf(out, "^%c", '@' + c);
	}
	else
	    putc(c, out);
    putc('\n', out);
}



/*
 * Read one "line" from FTN text body. A line comprises arbitrary
 * characters terminated with a CR '\r'. None, one, or many LFs '\n'
 * may follow:
 *
 *    x x x x x x x x x \r [\n...]
 *
 * This function checks for orphan 0-bytes in the message body and
 * recognizes grunged messages produced by SQUISH 1.01.
 *
 * Return values:
 *     -1  ERROR     an error occured
 *      1            next line follows
 *      0  MSG_END   end of message, end of packet
 *      2  MSG_TYPE  end of message, next message header follows
 */
int pkt_get_line(FILE *fp, char *buf, int size)
{
    char *p;
    int c, c1, c2;
    int read_lf=FALSE;
    long pos;
    
    p = buf;

    while (size > 3)			/* Room for current + 2 extra chars */
    {
	c = getc(fp);
	
	if(read_lf && c!='\n')		/* No more LFs, this is end of line */
	{
	    ungetc(c, fp);
	    *p = 0;
	    return 1;
	}
	   
	switch(c)
	{
	case EOF:			/* premature EOF */
	    return ERROR;
	    break;

	case 0:				/* end of message or orphan */
	    c1 = getc(fp);
	    c2 = getc(fp);
	    if(c1==EOF || c2==EOF)
		return ERROR;
	    if(c1==2 && c2==0)		/* end of message */
	    {
		*p = 0;
		return MSG_TYPE;
	    }
	    if(c1==0 && c2==0)		/* end of packet */
	    {
		*p = 0;
		return MSG_END;
	    }
	    /* orphan 0-byte, skip */
	    pos = ftell(fp);
	    if(pos == ERROR)
		logit("pkt_get_line(): orphan 0-char (can't determine offset)");
	    else
		logit("pkt_get_line(): orphan 0-char (offset=%ld)", pos);
	    if(c1)
	    {
		size--;
		*p++ = c1;
	    }
	    if(c2)
	    {
		size--;
		*p++ = c2;
	    }
	    continue;
	    break;
	    
#if 1 /***** Work around for a SQUISH bug **********************************/
	case 2:	    			/* May be grunged packet: start of */
	    c1 = getc(fp);		/* new message                     */
	    if(c1 == EOF)
		return ERROR;
	    if(c1 == 0)			/* Looks like it is ... */
	    {
		*p = 0;
		logit("pkt_get_line(): grunged packet");
		return MSG_TYPE;
	    }
	    *p++ = c;
	    *p++ = c1;
	    size--;
	    size--;
	    break;
	    
#endif /********************************************************************/

	case '\r':			/* End of line */
	    read_lf = TRUE;
	    /**fall thru**/
		
	default:
	    *p++ = c;
	    size--;
	    break;
	}
    }

    /* buf too small */
    *p = 0;
    return 1;
}



/*
 * Read text body from packet into Textlist
 *
 * Return values:
 *     -1  ERROR     an error occured
 *      0  MSG_END   end of packet
 *      2  MSG_TYPE  next message header follows
 */
int pkt_get_body(FILE *fp, Textlist *tl)
{
    int type;

    tl_clear(tl);

    /* Read lines and put into textlist */
    while( (type=pkt_get_line(fp, buffer, sizeof(buffer))) == 1 )
	tl_append(tl, buffer);
    /* Put incomplete last line into textlist, if any */
    if( (type==MSG_END || type==MSG_TYPE) && buffer[0] )
    {
	/* Make sure that this line is terminated by \r\n */
	BUF_APPEND(buffer, "\r\n");

	tl_append(tl, buffer);
    }
    
    return type;
}



/*
 * Initialize MsgBody
 */
void msg_body_init(MsgBody *body)
{
    body->area = NULL;
    tl_init(&body->kludge);
    tl_init(&body->rfc);
    tl_init(&body->body);
    body->tear = NULL;
    body->origin = NULL;
    tl_init(&body->seenby);
    tl_init(&body->path);
    tl_init(&body->via);
}



/*
 * Clear MsgBody
 */
void msg_body_clear(MsgBody *body)
{
    xfree(body->area);
    body->area = NULL;
    tl_clear(&body->kludge);
    tl_clear(&body->rfc);
    tl_clear(&body->body);
    xfree(body->tear);
    body->tear = NULL;
    xfree(body->origin);
    body->origin = NULL;
    tl_clear(&body->seenby);
    tl_clear(&body->path);
    tl_clear(&body->via);
}



/*
 * Convert message body from Textlist to MsgBody struct
 *
 * Return: -1  error during parsing, but still valid control info
 *         -2  fatal error, can't process message
 */

static char *rfc_headers[] =
{
    FTN_RFC_HEADERS, NULL
};


static int msg_body_parse_echomail(MsgBody *body)
{
    Textline *p, *pn;

    /*
     * Work our way backwards from the end of the body to the tear line
     */
    /* Search for last ^APath or SEEN-BY line */
    for(p=body->body.last;
	p && strncmp(p->line, "\001PATH", 5) && strncmp(p->line, "SEEN-BY", 7);
	p=p->prev) ;
    if(p == NULL)
    {
	logit("ERROR: parsing echomail message: no ^APATH or SEEN-BY line");
	return -2;
    }
    /* ^APATH */
    for(; p && !strncmp(p->line, "\001PATH", 5); p=p->prev) ;
    /* SEEN-BY */
    for(; p && !strncmp(p->line, "SEEN-BY", 7); p=p->prev) ;
    /* Some systems generate empty line[s] between Origin and SEEN-BY :-( */
    for(; p && *p->line=='\r'; p=p->prev );
    /*  * Origin: */
    if(p && !strncmp(p->line, " * Origin:", 10))
	p = p->prev;
    /* --- Tear line */
    if(p && (!strncmp(p->line, "---\r", 4) ||
	     !strncmp(p->line, "--- ", 4)    ))
	p = p->prev;
    /* Move back */
    p = p ? p->next : body->body.first;
    
    /*
     * Copy to MsgBody
     */
    /* --- Tear line */
    if(p && (!strncmp(p->line, "---\r", 4) ||
	     !strncmp(p->line, "--- ", 4)    ))
    {
	pn = p->next;
	body->tear = strsave(p->line);
	tl_delete(&body->body, p);
	p = pn;
    }
    /*  * Origin: */
    if(p && !strncmp(p->line, " * Origin:", 10))
    {
	pn = p->next;
	body->origin = strsave(p->line);
	tl_delete(&body->body, p);
	p = pn;
    }
    /* There may be empty lines after Origin ... more FIDO brain damage */
    while(p && *p->line=='\r')
    {
	pn = p->next;
	tl_delete(&body->body, p);
	p = pn;
    }
    /* SEEN-BY */
    while(p)
    {
	pn = p->next;
	if(!strncmp(p->line, "SEEN-BY", 7))
	{
	    tl_remove(&body->body, p);
	    tl_add(&body->seenby, p);
	    p = pn;
	}
	else
	    break;
    }
    /* ^APATH */
    while(p)
    {
	pn = p->next;
	if(!strncmp(p->line, "\001PATH", 5))
	{
	    tl_remove(&body->body, p);
	    tl_add(&body->path, p);
	    p = pn;
	}
	else
	    break;
    }
    /* Delete any following lines */
    while(p)
    {
	pn = p->next;
	tl_delete(&body->body, p);
	p = pn;
    }

    if(body->seenby.n==0 /*|| body->path.n==0*/)
    {
	logit("ERROR: parsing echomail message: no SEEN-BY line");
	return -2;
    }
    if(body->tear==NULL || body->origin==NULL /** || body->path.n==0 **/)
	return -1;
    
    return OK;
}


static int msg_body_parse_netmail(MsgBody *body)
{
    Textline *p, *pn;

    /*
     * Work our way backwards from the end of the body to the tear line
     */
    /* ^AVia, there may be empty lines within the ^AVia lines */
    for(p=body->body.last;
	p && ( !strncmp(p->line, "\001Via", 4) || *p->line=='\r' );
	p=p->prev) ;
    /*  * Origin: */
    if(p && !strncmp(p->line, " * Origin:", 10))
	p = p->prev;
    /* --- Tear line */
    if(p && (!strncmp(p->line, "---\r", 4) ||
	     !strncmp(p->line, "--- ", 4)    ))
	p = p->prev;
    /* Move back */
    p = p ? p->next : body->body.first;
    
    /*
     * Copy to MsgBody
     */
    /* --- Tear line */
    if(p && (!strncmp(p->line, "---\r", 4) ||
	     !strncmp(p->line, "--- ", 4)    ))
    {
	pn = p->next;
	body->tear = strsave(p->line);
	tl_delete(&body->body, p);
	p = pn;
    }
    /*  * Origin: */
    if(p && !strncmp(p->line, " * Origin:", 10))
    {
	pn = p->next;
	body->origin = strsave(p->line);
	tl_delete(&body->body, p);
	p = pn;
    }
    /* ^AVia */
    while(p)
    {
	pn = p->next;
	if(!strncmp(p->line, "\001Via", 4))
	{
	    tl_remove(&body->body, p);
	    tl_add(&body->via, p);
	    p = pn;
	}
	else if(*p->line == '\r')
	{
	    tl_remove(&body->body, p);
	    p = pn;
	}
	else
	    break;
    }
    /* Delete any following lines */
    while(p)
    {
	pn = p->next;
	tl_delete(&body->body, p);
	p = pn;
    }

    return OK;
}


int is_blank_line(char *s)
{
    if(!s)
	return TRUE;
    while(*s)
    {
	if(!is_space(*s))
	    return FALSE;
	s++;
    }
    return TRUE;
}


int msg_body_parse(Textlist *text, MsgBody *body)
{
    Textline *p, *pn;
    int i;
    int look_for_AREA;
    
    msg_body_clear(body);
    
    p = text->first;
    
    /*
     * 1st, look for ^A kludges and AREA: line
     */
    look_for_AREA = TRUE;
    while(p)
    {
	pn = p->next;

	if(p->line[0] == '\001') 		/* ^A kludge,    */
	{
	    tl_remove(text, p);
#if 0
	    if(! strncmp(p->line, "\001Via", 4))
		tl_add(&body->via, p);
	    else
#endif
		tl_add(&body->kludge, p);
	    p = pn;
	    continue;
	}
	if(look_for_AREA                  &&	/* Only 1st AREA line */
	   !strncmp(p->line, "AREA:", 5)    )	/* AREA:XXX */
	{
	    look_for_AREA = FALSE;
	    body->area = strsave(p->line);
	    tl_delete(text, p);
	    p = pn;
	    continue;
	}

	break;
    }

    /*
     * Next, look for supported RFC header lines. Up to 3 blank
     * lines before the RFC headers are allowed
     */
    if(p && is_blank_line(p->line))
	p = p->next;
    if(p && is_blank_line(p->line))
	p = p->next;
    if(p && is_blank_line(p->line))
	p = p->next;

    while(p)
    {
	int found;
	pn = p->next;

	for(found=FALSE, i=0; rfc_headers[i]; i++)
	    if(!strnicmp(p->line, rfc_headers[i], strlen(rfc_headers[i])))
	    {
		tl_remove(text, p);
		tl_add(&body->rfc, p);
		p = pn;
		found = TRUE;
		break;
	    }

	if(!found)
	    break;
    }

    /*
     * Now, text contains just the message body after kludges
     * and RFC headers, copy to body->body.
     */
    body->body = *text;
    tl_init(text);

    /*
     * Call function for EchoMail and NetMail, respectively, parsing
     * control info at end of text body.
     */
    return body->area ? msg_body_parse_echomail(body)
	              : msg_body_parse_netmail (body) ;
}



/*
 * Debug output of message body
 */
void msg_body_debug(FILE *out, MsgBody *body, int crlf)
{
    Textline *p;
    
    fprintf(out, "----------------------------------------"
	         "--------------------------------------\n");
    if(body->area)
	debug_line(out, body->area, crlf);
    for(p=body->kludge.first; p; p=p->next)
	debug_line(out, p->line, crlf);
    fprintf(out, "----------------------------------------"
	            "--------------------------------------\n");
    if(body->rfc.first)
    {
	for(p=body->rfc.first; p; p=p->next)
	    debug_line(out, p->line, crlf);
	fprintf(out, "----------------------------------------"
		     "--------------------------------------\n");
    }
    for(p=body->body.first; p; p=p->next)
	debug_line(out, p->line, crlf);
    fprintf(out, "----------------------------------------"
	         "--------------------------------------\n");
    if(body->tear)
	debug_line(out, body->tear, crlf);
    if(body->origin)
	debug_line(out, body->origin, crlf);
    for(p=body->seenby.first; p; p=p->next)
	debug_line(out, p->line, crlf);
    for(p=body->path.first; p; p=p->next)
	debug_line(out, p->line, crlf);
    for(p=body->via.first; p; p=p->next)
	debug_line(out, p->line, crlf);
    fprintf(out, "========================================"
	         "======================================\n");
}



/*
 * Write single line to packet file, checking for NULL
 */
int msg_put_line(FILE *fp, char *line)
{
    if(line)
	fputs(line, fp);
    return ferror(fp);
}



/*
 * Write MsgBody to packet file
 */
int msg_put_msgbody(FILE *fp, MsgBody *body, int term)
{
    msg_put_line(fp,  body->area  );
    tl_fput     (fp, &body->kludge);
    tl_fput     (fp, &body->rfc   );
    tl_fput     (fp, &body->body  );
    msg_put_line(fp,  body->tear  );
    msg_put_line(fp,  body->origin);
    tl_fput     (fp, &body->seenby);
    tl_fput     (fp, &body->path  );
    tl_fput     (fp, &body->via   );

    if(term)
	putc(0, fp);			/* Terminating 0-byte */
    
    return ferror(fp);
}



/*
 * Convert text line read from FTN message body
 */
char *msg_xlate_line(char *buf, int n, char *line, int qp)
{
    char *s, *p, *xl;
    int c;

    n--;				/* Room for \0 char */

    for(s=line, p=buf; *s; s++)
    {
	c = *s & 0xff;
	
	/*
	 * Special chars require special treatment ...
	 */
	if(c == '\n')			/* Ignore \n */
	    continue;
	if(msg_ignore_0x8d && c==0x8d)	/* Ignore soft \r */
	    continue;
	if(c == '\r')
	    c = '\n';
	else if(c < ' ') {
	    /* Translate control chars to ^X */
	    if(c!='\t' && c!='\f')
	    {
		if(!n--)
		    break;
		*p++ = '^';
		c = c + '@';
	    }
	}
	else if(qp && c=='=')
	{
	    /* Translate '=' to MIME quoted-printable =3D */
	    xl = "=3D";
	    while(*xl)
	    {
		if(!n--)
		    break;
		*p++ = *xl++;
	    }
	    continue;
	}
	else if(c & 0x80)
	{
	    /* Translate special characters according to character set */
	    xl = charset_map_c(c, qp);
	    if(!xl || !*xl)
		continue;
	    while(*xl)
	    {
		if(!n--)
		    break;
		*p++ = *xl++;
	    }
	    continue;
	}

	/*
	 * Put normal char into buf
	 */
	if(!n--)
	    break;
	*p++ = c;
    }

    *p = 0;
    
    return OK;
}



/*
 * Format buffer line and put it into Textlist. Returns number of
 * lines.
 */
#define DEFAULT_LINE_LENGTH	72
#define NOBREAK_LINE_LENGTH	79
#define MAX_LINE_LENGTH		200

static int msg_get_line_length(void)
{
    static int message_line_length = 0;

    if(!message_line_length) 
    {
	char *p;
	if( (p = cf_get_string("MessageLineLength", TRUE)) )
	{
	    debug(8, "config: MessageLineLength %s", p);
	    message_line_length = atoi(p);
	    if(message_line_length < 20 ||
	       message_line_length > MAX_LINE_LENGTH) 
	    {
		logit("WARNING: illegal MessageLineLength value %d",
		    message_line_length);
		message_line_length = DEFAULT_LINE_LENGTH;
	    }
	}
	else
	    message_line_length = DEFAULT_LINE_LENGTH;
    }
    return message_line_length;
}


int msg_format_buffer(char *buffer, Textlist *tlist)
{
    int max_linelen;
    char *p, *np;
    char localbuffer[MAX_LINE_LENGTH + 16];	/* Some extra space */
    int i;
    int lines;

    max_linelen = msg_get_line_length();
    
    if(strlen(buffer) <= NOBREAK_LINE_LENGTH)	/* Nothing to do */
    {
	tl_append(tlist, buffer);
	return 1;
    }
    else
    {
	/* Break line with word wrap */
	lines = 0;
	p     = buffer;

	while(TRUE)
	{
	    /* Search backward for a whitespace to break line. If no
	     * proper point is found, the line will not be split.  */
	    for(i=max_linelen-1; i>=0; i--)
		if(is_blank(p[i]))	/* Found a white space */
		    break;
	    if(i < max_linelen/2)	/* Not proper space to split found, */
	    {				/* put line as is                   */
		tl_append(tlist, p);
		lines++;
		return lines;
	    }
	    for(; i>=0 && is_blank(p[i]); i--);	/* Skip more white space */
	    i++;				/* Return to last white sp. */

	    /* Cut here and put into textlist */
	    np = p + i;
	    *np++ = 0;
	    BUF_COPY2(localbuffer, p, "\n");
	    tl_append(tlist, localbuffer);
	    lines++;
	    
	    /* Advance buffer pointer and test length of remaining
	     * line */
	    p = np;
	    for(; *p && is_blank(*p); p++);	/* Skip white space */
	    if(*p == 0)				/* The end */
		return lines;
	    if(strlen(p) <= max_linelen)	/* No more wrappin' */
	    {
		tl_append(tlist, p);
		lines++;
		return lines;
	    }

	    /* Play it again, Sam! */
	}
    }
    /**NOT REACHED**/
    return 0;
}



/*
 * check_origin() --- Analyse ` * Origin: ...' line in FIDO message
 *		      body and parse address in ()s into Node structure
 *
 * Origin line is checked for the rightmost occurence of
 * ([text] z:n/n.p).
 */
int msg_parse_origin(char *buffer, Node *node)
{
    char *left, *right;
    char *buf;

    if(buffer == NULL)
	return ERROR;
    
    buf   = strsave(buffer);
    right = strrchr(buf, ')');
    if(!right) {
	xfree(buf);
	return ERROR;
    }
    left  = strrchr(buf, '(');
    if(!left) {
	xfree(buf);
	return ERROR;
    }

    *right = 0;
    *left++ = 0;

    /* Parse node info */
    while(*left && !is_digit(*left))
	left++;
    if(asc_to_node(left, node, FALSE) != OK)
	/* Not a valid FIDO address */
	node_invalid(node);

    xfree(buf);
    return node->zone != -1 ? OK : ERROR;
}
