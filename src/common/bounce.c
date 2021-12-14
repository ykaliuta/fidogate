/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: bounce.c,v 4.14 2004/08/22 20:19:11 n0ll Exp $
 *
 * Bounce mails for various reasons, using LIBDIR/bounce.XXX messages
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
 * Set CC address for bounced messages
 */
static char *bounce_ccmail = NULL;

void bounce_set_cc(char *cc)
{
    bounce_ccmail = cc;
}



/*
 * Print text file with substitutions for %x
 */
int print_file_subst(FILE *in, FILE *out, Message *msg, char *rfc_to, Textlist *body)
{
    int c;
#ifdef AI_8
    char *hg;
#endif

    while((c = getc(in)) != EOF)
    {
	if(c == '%') 
	{
	    c = getc(in);
	    switch(c) 
	    {
	    case 'F':			/* From node */
		fputs( znfp1(&msg->node_from), out);	break;
	    case 'T':			/* To node */
		fputs( znfp1(&msg->node_to), out);		break;
	    case 'O':			/* Orig node */
		fputs( znfp1(&msg->node_orig), out);	break;
	    case 'd':			/* Date */
		fputs( date(NULL, &msg->date), out);			break;
	    case 't':			/* To name */
		fputs( msg->name_to, out);				break;
	    case 'f':			/* From name */
		fputs( msg->name_from, out);				break;
	    case 's':			/* Subject */
		fputs( msg->subject, out);				break;
	    case 'R':			/* RFC To: */
		fputs( rfc_to, out);					break;
	    case 'M':			/* Message */
		tl_print(body, out);				break;
#ifdef AI_8
	    case 'A':			/* RFC From: */
		if((hg = s_header_getcomplete("From")))
		    fputs( hg, out);					break;
	    case 'D':			/* RFC Date: */
	    	if((hg = header_get("Date")))
		    fputs( hg, out);					break;
	    case 'N':			/* RFC Newsgroups: */
		if((hg = header_get("Newsgroups")))
		    fputs( hg, out);					break;
	    case 'S':			/* RFC Subject: */
		if((hg = header_get("Subject")))
		    fputs( hg, out);					break;
#endif
	    }
	}
	else 
	    putc(c, out);
    }
    
    return ferror(in);
}



/*
 * Create header for bounced mail
 */
int bounce_header(char *to)
             				/* To: */
{
    /*
     * Open new mail
     */
    if(mail_open('m') == ERROR)
	return ERROR;

    /*
     * Create RFC header
     */
    fprintf(mail_file('m'),
	    "From Mailer-Daemon %s\n", date(DATE_FROM, NULL) );
    fprintf(mail_file('m'),
	    "Date: %s\n", date(NULL, NULL) );
    fprintf(mail_file('m'),
	    "From: Mailer-Daemon@%s (Mail Delivery Subsystem)\n", cf_fqdn() );
    fprintf(mail_file('m'), "To: %s\n", to);
    if(bounce_ccmail)
	fprintf(mail_file('m'), "Cc: %s\n", bounce_ccmail);
    /* Additional header may follow in message file */

    return OK;
}



/*
 * Bounce mail
 */
int bounce_mail(char *reason, RFCAddr *addr_from, Message *msg, char *rfc_to, Textlist *body)
{
    char *to;
    FILE *in;
    
    to = s_rfcaddr_to_asc(addr_from, TRUE);

    if(bounce_header(to) == ERROR)
	return ERROR;

    BUF_COPY3(buffer, cf_p_configdir(), "/bounce.", reason);
    
    in = xfopen(buffer, R_MODE);
    print_file_subst(in, mail_file('m'), msg, rfc_to, body);
    fclose(in);
    
    return mail_close('m');
}


