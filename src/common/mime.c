/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id$
 *
 * MIME stuff
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
 *  _____ _____
 * |     |___  |   Martin Junius             <mj@fidogate.org>
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
#define MIME_HEADER_CODE_START	"=?"
#define MIME_HEADER_CODE_MIDDLE_QP	"?Q?"
#define MIME_HEADER_CODE_MIDDLE_B64	"?B?"
#define MIME_HEADER_CODE_END	"?="
#define MIME_HEADER_STR_DELIM	"\r\n "
#define MIME_STRING_LIMIT 74
/* A..Z -- 0x00..0x19
 * a..z -- 0x1a..0x33
 * 0..9 -- 0x34..0x3d
 * +    -- 0x3e
 * /    -- 0x3f
 * =    -- special
 */
int mime_b64toint(char c)
{
         if (('A' <= c) && ('Z' >= c)) return (       c - 'A');
    else if (('a' <= c) && ('z' >= c)) return (0x1a + c - 'a');
    else if (('0' <= c) && ('9' >= c)) return (0x34 + c - '0');
    else if ('+' == c)                 return 0x3e;
    else if ('/' == c)                 return 0x3f;
    else if ('=' == c)                 return 0x40;
    else                               return ERROR;
}

char mime_inttob64(int a)
{
    char c = (char)(a & 0x0000003f);
    if(c <= 0x19)
        return c + 'A';
    if(c <= 0x33)
        return c - 0x1a + 'a';
    if(c <= 0x3d)
        return c - 0x34 + '0';
    if(c == 0x3e)
        return '+';
    else
        return '/';
}

int mime_qptoint(char c)
{
         if (('0' <= c) && ('9' >= c)) return (       c - '0');
    else if (('A' <= c) && ('F' >= c)) return (0x0a + c - 'A');
    else if (('a' <= c) && ('f' >= c)) return (0x0a + c - 'a');
    else                               return ERROR;
}

#define B64_ENC_CHUNK 3
#define B64_NLET_PER_CHUNK 4
#define B64_MAX_ENC_LEN 31

int mime_enheader(char **dst, unsigned char *src, size_t len, char *encoding)
{
    int buflen, delimlen = 0;
    char *buf = NULL;
    int padding;
    int i;
    int outpos = 0;
    char *delim = NULL;
    
    padding = (B64_ENC_CHUNK - len % B64_ENC_CHUNK) % B64_ENC_CHUNK;
    
    /* Round it up and find len
     * (here (val) *  B64_ENC_CHUNK * B64_NLET_PER_CHUNK / B64_ENC_CHUNK )*/
    buflen = ((len + (B64_ENC_CHUNK - 1)) / B64_ENC_CHUNK )  * B64_NLET_PER_CHUNK + 1; 
    
    if(encoding == NULL)
    {
        delimlen = strlen(MIME_HEADER_STR_DELIM);
    }
    else
    {
        delimlen = strlen(MIME_HEADER_CODE_START)
            + xstrnlen(encoding, B64_MAX_ENC_LEN)
            + strlen(MIME_HEADER_CODE_MIDDLE_B64)
            + strlen(MIME_HEADER_CODE_END);
        buflen += delimlen;
        delimlen += strlen(MIME_HEADER_STR_DELIM);
    }

    if((delim = xmalloc(delimlen + 1)) == NULL)
        return ERROR;

    buflen += (buflen / MIME_STRING_LIMIT) * delimlen;
    
    if((buf = xmalloc(buflen)) == NULL)
    {
        xfree(delim);
        return ERROR;
    }

    memset(buf, 0, buflen);
    delim[0] = '\0';

    *dst = buf;
    
    if(encoding == NULL)
    {
        strcpy(delim, MIME_HEADER_STR_DELIM);
    }
    else
    {
        strcat(buf, MIME_HEADER_CODE_START);
        strncat(buf, encoding, B64_MAX_ENC_LEN);
        strcat(buf, MIME_HEADER_CODE_MIDDLE_B64);
        outpos += strlen(buf);

        strcat(delim, MIME_HEADER_CODE_END);
        strcat(delim, MIME_HEADER_STR_DELIM);
        strcat(delim, buf);
    }

    len += padding; 

    for(i = 0; i < len; i += 3)
    {
        if((outpos % MIME_STRING_LIMIT) < 4 )
        {
            strcat(buf+outpos, delim);
            outpos += delimlen;
        }
        /*
          Use this set of shifts, because loop and pointer is
          endian-dependend way
        */
        buf[outpos++] = mime_inttob64(src[i] >> 2);
        buf[outpos++] = mime_inttob64((src[i] << 4) | (src[i+1] >> 4));
        buf[outpos++] = mime_inttob64((src[i+1] << 2) | (src[i+2] >> 6));
        buf[outpos++] = mime_inttob64(src[i+2]);
    }
    while(padding > 0)
        buf[outpos - padding--] = '=';

    if(encoding != NULL)
        strcat(buf, MIME_HEADER_CODE_END);

    xfree(delim);
    return OK;
}


int mime_b64_decode(char **dst, char *src, size_t len)
{
    char *buf;
    size_t buflen;
    size_t i;
    int v1, v2, v3, v4;
    char *d;
    int rc = OK;

    if (0 != (len % 4))
	return ERROR;

    buflen = ((len / 4) * 3) + 1;
    buf = xmalloc(buflen);
    memset(buf, 0, buflen);
    i = 0;
    d = buf;
    while (len > i)
    {
	v1 = mime_b64toint(src[i++]);
	if (ERROR == v1)
	{
	    rc = ERROR;
	    break;
	}

	v2 = mime_b64toint(src[i++]);
	if (ERROR == v2)
	{
	    rc = ERROR;
	    break;
	}

	v3 = mime_b64toint(src[i++]);
	if (ERROR == v3)
	{
	    rc = ERROR;
	    break;
	}

	v4 = mime_b64toint(src[i++]);
	if (ERROR == v4)
	{
	    rc = ERROR;
	    break;
	}

	*d++ = (v1 << 2) | (v2 >> 4);
	if (0x40 > v3)
	{
	    *d++ = ((v2 << 4) & 0xf0) | (v3 >> 2);
	    if (0x40 > v4)
		*d++ = ((v3 << 6) & 0xc0) | v4;
	}
	else if (0x40 > v4)
	{
	    rc = ERROR;
	    break;
	}
    }

    if (ERROR == rc)
	xfree(buf);
    else
	*dst = buf;
    return rc;
}

int mime_qp_decode(char **dst, char *src, size_t len)
{
    char *buf;
    size_t i;
    int vh, vl;
    char *d;
    char *p1, *p2;
    int j;
    int rc = OK;

    p1 = src;
    j = 0;
    while (NULL != (p2 = strchr(p1, '_')))
    {
	p1 = ++p2;
	j++;
    }

    buf = xmalloc(len);
    memset(buf, 0, len);

    i = 0;
    d = buf;
    while (len > i)
    {
	if ('_' == src[i])
	{
	    *d++ = ' ';
	    i++;
	    continue;
	}

	if ('=' != src[i])
	{
	    *d++ = src[i++];
	    continue;
	}

	i++;

	vh = mime_qptoint(src[i++]);
	if (ERROR == vh)
	{
	    rc = ERROR;
	    break;
	}

	vl = mime_qptoint(src[i++]);
	if (ERROR == vl)
	{
	    rc = ERROR;
	    break;
	}

	*d++ = (char)(((vh << 4) & 0xf0) | (vl & 0x0f));
    }

    if (ERROR == rc)
	xfree(buf);
    else
	*dst = buf;
    return rc;
}

char *mime_deheader(char *d, size_t n, char *s)
{
    int	i;
    char *p, *beg, *end, *buf;

    memset(d, 0, n);
    for (i = 0; (n - 1 > i) && ('\0' != *s);)
    {
	if (strnieq(s, MIME_HEADER_CODE_START, strlen(MIME_HEADER_CODE_START)))
	{
	    /* May be MIME (b64 or qp) */
	    p = strchr(s + strlen(MIME_HEADER_CODE_START), '?');
	    if (NULL != p)
	    {
		/* Check if b64 or qp */
		if (strnieq(p, MIME_HEADER_CODE_MIDDLE_QP, strlen(MIME_HEADER_CODE_MIDDLE_QP)))
		{
		    /* May be qp */
		    beg = p + strlen(MIME_HEADER_CODE_MIDDLE_QP);
		    end = strchr(beg, '?');
		    if (NULL != end &&
			strnieq(end, MIME_HEADER_CODE_END, strlen(MIME_HEADER_CODE_END)) &&
			ERROR != mime_qp_decode(&buf, beg, end - beg))
		    {
			strncpy(&(d[i]), buf, n - i - 1);
			free(buf);
			i += strlen(&(d[i]));
			s = end + strlen(MIME_HEADER_CODE_END);
			continue;
		    }
		}
		else if (strnieq(p, MIME_HEADER_CODE_MIDDLE_B64, strlen(MIME_HEADER_CODE_MIDDLE_B64)))
		{
		    /* May be b64 */
		    beg = p + strlen(MIME_HEADER_CODE_MIDDLE_B64);
		    end = strchr(beg, '?');
		    if (NULL != end &&
			strnieq(end, MIME_HEADER_CODE_END, strlen(MIME_HEADER_CODE_END)) &&
			ERROR != mime_b64_decode(&buf, beg, end - beg))
		    {
			strncpy(&(d[i]), buf, n - i - 1);
			free(buf);
			i += strlen(&(d[i]));
			s = end + strlen(MIME_HEADER_CODE_END);
			continue;
		    }
		}
	    }
	}

	/* Nothing special to do */
	d[i++] = *s++;
    }
    d[i] = 0;

    return d;
}


/*
 * Return MIME header
 */
MIMEInfo *get_mime(char *ver, char *type, char *enc)
{
    static MIMEInfo mime;
    char *s, *p, *q;
    
    mime.version       = ver;
    mime.type          = type;
    mime.type_type     = NULL;
    mime.type_charset  = NULL;
    mime.type_boundary = NULL;
    mime.encoding      = enc;

    s = mime.type ? s_copy(mime.type) : NULL;
    if( s && (p = strtok(s, ";")) )
    {
	p = strip_space(p);
	mime.type_type = s_copy(p);

	for(p=strtok(NULL, ";"); p; p=strtok(NULL, ";"))
	{
	    p = strip_space(p);
	    if(strnieq(p, "charset=", strlen("charset=")))
	    {
		p += strlen("charset=");
		if(*p == '\"') 					//"'
		  p++;
		for(q=p; *q; q++)
		  if(*q=='\"' || is_space(*q))			//"'
		    break;
		*q = 0;
		mime.type_charset = s_copy(p);
	    }
	    if(strnieq(p, "boundary=", strlen("boundary=")))
	    {
		p += strlen("boundary=");
		if(*p == '\"')					//"'
		  p++;
		for(q=p; *q; q++)
		  if(*q=='\"' || is_space(*q))			//"'
		    break;
		*q = 0;
		mime.type_boundary = s_copy(p);
	    }
	}
    }

    debug(6, "RFC MIME-Version:              %s",
	  mime.version ? mime.version : "-NONE-");
    debug(6, "RFC Content-Type:              %s",
	  mime.type ? mime.type : "-NONE-");
    debug(6, "                        type = %s",
	  mime.type_type ? mime.type_type : "");
    debug(6, "                     charset = %s",
	  mime.type_charset ? mime.type_charset : "");
    debug(6, "                    boundary = %s",
	  mime.type_boundary ? mime.type_boundary : "");
    debug(6, "RFC Content-Transfer-Encoding: %s",
	  mime.encoding ? mime.encoding : "-NONE-");

    return &mime;
}
