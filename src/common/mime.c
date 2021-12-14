/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: mime.c,v 4.18 2004/08/22 20:19:11 n0ll Exp $
 *
 * MIME stuff
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


static int is_qpx		(int);
static int x2toi		(char *);


static int is_qpx(int c)
{
    return isxdigit(c) /**is_digit(c) || (c>='A' && c<='F')**/ ;
}


static int x2toi(char *s)
{
    int val = 0;
    int n;

    n = toupper(*s) - (isalpha(*s) ? 'A'-10 : '0');
    val = val*16 + n;
    s++;
    n = toupper(*s) - (isalpha(*s) ? 'A'-10 : '0');
    val = val*16 + n;

    return val;
}



/*
 * Dequote string with MIME-style quoted-printable =XX
 */
char *mime_dequote(char *d, size_t n, char *s, int flags)
{
    int i, c=0;
    char *xl;

    for(i=0; i<n-1 && *s; i++, s++)
    {
	if( (flags & MIME_QP) && (s[0] == '=') )/* MIME quoted printable */
	{
	    if(is_qpx(s[1]) && is_qpx(s[2]))	/* =XX */
	    {
	        c = x2toi(s+1);
		s += 2;
	    }
	    else if(s[1]=='\n'           ||	/* =<LF> and */
	       (s[1]=='\r' && s[2]=='\n')  )	/* =<CR><LF> */
	    {
		break;
	    }
	}
	else if( (flags & MIME_US) && (s[0] == '_') ) /* Underscore */
	{
	    c = ' ';
	}
	else {					/* Nothing special to do */
	    c = *s;
	}

	if(c & 0x80)
	{
	    /* Translate special characters according to charset */
	    if( (xl = charset_map_c(c, FALSE)) )
	    {
		while(*xl && i<n-1)
		{
		    d[i] = *xl++;
		    if(*xl)
		        i++;
		}
	    }
	}
	else
	  d[i] = c;
    }
    d[i] = 0;

    return d;
}



/*
 * Decode MIME RFC1522 header
 *
 **FIXME: currently always assumes ISO-8859-1 char set
 **FIXME: optional flag for conversion to 7bit ASCII replacements
 */
#define MIME_HEADER_CODE_START	"=?ISO-8859-1?Q?"
#define MIME_HEADER_CODE_END	"?="

char *mime_deheader(char *d, size_t n, char *s, int flags)
{
    int i;
    int mime_flag = FALSE;

    for(i=0; i<n-1 && *s; )
    {
	if(strnieq(s, MIME_HEADER_CODE_START, strlen(MIME_HEADER_CODE_START)))
	{
	    mime_flag = TRUE;
	    s += strlen(MIME_HEADER_CODE_START);
	    continue;
	}

	if(mime_flag)
	{
	    if(strneq(s, MIME_HEADER_CODE_END, strlen(MIME_HEADER_CODE_END)))
	    {
		mime_flag = FALSE;
		s += strlen(MIME_HEADER_CODE_END);
		continue;
	    }
	    
	    if(s[0] == '=')	/* Maybe MIME quoted printable */
	    {
		if(is_qpx(s[1]) && is_qpx(s[2]))	/* =XX */
		{
		    d[i++] = x2toi(s+1);
		    s += 3;
		    continue;
		}
	    }

	    if(s[0] == '_')
	    {
		d[i++] = ' ';
		s++;
		continue;
	    }
	}
	
	/* Nothing special to do */
	d[i++] = *s++;
    }
    d[i] = 0;

    return d;
}
