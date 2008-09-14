/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: xalloc.c,v 4.9 2004/08/22 20:19:11 n0ll Exp $
 *
 * Safe memory allocation functions
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
 * Global buffer for temporary usage
 */
char buffer[BUFFERSIZE];



/*
 * xmalloc(), xrealloc()  ---  safe versions of malloc() and realloc()
 */
void *xmalloc(int size)
{
    char *p;

    if((p = malloc(size)))
	return(p);
    fprintf(stderr, "Memory exhausted.");
    exit(EX_OSERR);

    /**NOT REACHED**/
    return NULL;
}

void *xrealloc(void *ptr, int size)
{
    char *p;

    if((p = realloc(ptr, size)))
	return(p);
    fprintf(stderr, "Memory exhausted.");
    exit(EX_OSERR);

    /**NOT REACHED**/
    return NULL;
}


/*
 * xfree() --- free() with check for NULL pointer (is safe according to
 *             Standard-C, but older libraries may not check this)
 */
void xfree(void *p)
{
    if(p)
	free(p);
}



/*
 * strsave()  ---  make a copy of a string
 */
char *strsave(char *s)
{
    char *p;
    size_t len;

    if(!s)
	return NULL;

    len = strlen(s) + 1;
    p = xmalloc(len);
    str_copy(p, len, s);

    return p;
}

char *strsave2(char *s1, char *s2)
{
    char *p;
    size_t len;

    if(!s1 || !s2)
	return NULL;
    
    len = strlen(s1) + strlen(s2) + 1;
    p = xmalloc(len);
    str_copy2(p, len, s1, s2);

    return p;
}



/*
 * strsaveline() --- like strsave(), but remove '\r\n' at end of line if any
 */

char *strsaveline(char *s)
{
    char *r;

    r = strsave(s);
    strip_crlf(r);

    return r;
}
