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

    buf = xmalloc(len + 1);
    memset(buf, 0, len + 1);

    i = 0;
    d = buf;
    while (len > i)
    {
	if(src[i] == '\n' || src[i] == '\r')
	{
	    *d = '\n';
	    break;
	}

	if ('_' == src[i])
	{
	    *d++ = ' ';
	    i++;
	    continue;
	}

	if ('=' != src[i])
	{
	    *d++ = src[i++] & 0x7F;
	    continue;
	}

	i++;

	if(src[i] == '\n' || src[i] == '\r')
	{
	    *d = '\n';
	    break;
	}
	
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

/* takes space-stripped string */
/* Now is not used */
/*
static char* mime_fetch_attribute(char *str, char *attr)
{
     int attr_len = strlen(attr);
     char *tmp_str = NULL;

     do
     {
	  if(strnieq(str, attr, attr_len))
	  {
	       tmp_str = xmalloc(attr_len + 1);
	       strncpy(tmp_str, str, attr_len);
	       tmp_str[attr_len] = '\0';
	       break;
	  }
	  str = strchr(str, ';');
	  if(str != NULL)
	       str++;
     } while (str != NULL);

     return tmp_str;
}
*/
static int mime_parse_header(Textlist *line, char *str)
{
     char *p = NULL;

     if(line == NULL || str == NULL)
	  return ERROR;

     debug(6, "Parsing header %s", str);
     for(p = strtok(str, ";"); p != NULL; p = strtok(NULL, ";"))
     {
	  debug(6, "Recording header attribute %s", p);
	  p = strip_space(p);
	  tl_append(line, p);
     }
     return OK;
}

static char* mime_attr_value(char *str)
{
     char *p, *q = NULL;
     
     if(str == NULL)
	  return NULL;

     p = strchr(str, '=');
     if(p != NULL) {
	  if(*(++p) == '\"')
	       p++;
	  for(q = p; *q != '\0'; q++)
	       if(*q == '\"' || is_space(*q))
		    break;
	  *q = 0;
	  p = strsave(p);
     }
     return p;
}

/*
 * Return MIME header
 */

static MIMEInfo *get_mime_disposition(char *ver, char *type, char *enc, char *disp)
{
    MIMEInfo *mime;
    
    Textlist header_line = { NULL, NULL };
    Textline *tmp_line;
    char *tmp_str = NULL;

    mime = (MIMEInfo*)s_alloc(sizeof(*mime));
    
    mime->version       = ver;
    mime->type          = type;
    mime->type_type     = NULL;
    mime->type_charset  = NULL;
    mime->type_boundary = NULL;
    mime->encoding      = enc;
    mime->disposition   = disp;
    mime->disposition_filename = NULL;

    if(type != NULL)
    {
	tmp_str = s_copy(type);
	mime_parse_header(&header_line, tmp_str);
	if(header_line.first != NULL && header_line.first->line != NULL)
	    mime->type_type = s_copy(header_line.first->line);
	
/*    tmp_str = tl_get_str(&header_line, "charset", strlen("charset")); */
	tmp_line = tl_get(&header_line, "charset", strlen("charset"));
	if(tmp_line != NULL)
	{
	    tmp_str = mime_attr_value(tmp_line->line);
	    mime->type_charset = s_copy(tmp_str);
	    xfree(tmp_str);
	}
/*    tmp_str = tl_get_str(&header_line, "boundary", */
/*    strlen("boundary")); */
	tmp_line = tl_get(&header_line, "boundary", strlen("boundary"));
	if(tmp_line != NULL)
	{
	    tmp_str = mime_attr_value(tmp_line->line);
	    mime->type_boundary = s_copy(tmp_str);
	    xfree(tmp_str);
	}
	tl_clear(&header_line);
    }

    if(disp != NULL)
    {
	tmp_str = s_copy(disp);
	mime_parse_header(&header_line, tmp_str);
/*	tmp_str = tl_get_str(&header_line, "filename", strlen("filename")); */
	tmp_line = tl_get(&header_line, "filename", strlen("filename"));
	if(tmp_line != NULL)
	    tmp_str = mime_attr_value(tmp_line->line);
	mime->disposition_filename = s_copy(tmp_str);
	xfree(tmp_str);
	tl_clear(&header_line);
    }
    
    debug(6, "RFC MIME-Version:              %s",
	  mime->version ? mime->version : "-NONE-");
    debug(6, "RFC Content-Type:              %s",
	  mime->type ? mime->type : "-NONE-");
    debug(6, "                        type = %s",
	  mime->type_type ? mime->type_type : "");
    debug(6, "                     charset = %s",
	  mime->type_charset ? mime->type_charset : "");
    debug(6, "                    boundary = %s",
	  mime->type_boundary ? mime->type_boundary : "");
    debug(6, "RFC Content-Transfer-Encoding: %s",
	  mime->encoding ? mime->encoding : "-NONE-");
    debug(6, "RFC Content-Disposition: %s",
	  mime->disposition ? mime->disposition : "-NONE-");
    debug(6, "                    filename = %s",
	  mime->disposition_filename ? mime->disposition_filename : "");

    return mime;
}

 
MIMEInfo *get_mime(char *ver, char *type, char *enc)
{
     return get_mime_disposition(ver, type, enc, NULL);
}

/* TODO smth like s_header_getcomplete() */

MIMEInfo* get_mime_from_header (Textlist *header)
{
    if(header == NULL)
	return get_mime_disposition(header_get("Mime-Version"),
				    header_get("Content-Type"),
				    header_get("Content-Transfer-Encoding"),
				    header_get("Content-Disposition"));
    else
	return get_mime_disposition(rfcheader_get(header, "Mime-Version"),
				    rfcheader_get(header, "Content-Type"),
				    rfcheader_get(header, "Content-Transfer-Encoding"),
				    rfcheader_get(header, "Content-Disposition"));
}

int put_mime(MIMEInfo *mime)
{
    s_free((char*)mime);
    return 0;
}

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
		fglog("WARNING: illegal MessageLineLength value %d",
		      message_line_length);
		message_line_length = DEFAULT_LINE_LENGTH;
	    }
	}
	else
	    message_line_length = DEFAULT_LINE_LENGTH;
    }
    return message_line_length;
}


/* Garantee [^\r]\n\0 in the end of string since our
   output subroutine adds '\r'
   str should have space for 1 extra symbol */

static char* mime_debody_flush_str(Textlist *body, char *str)
{
    int len;
    if(str == NULL || body == NULL)
	return NULL;
    
    len = strlen(str);
    if(len < 2)
	return NULL;

    if(str[len - 2] == '\r') {
	 str[len - 2] = '\n';
	 str[len - 1] = '\0';
    }
    else if(str[len - 1] != '\n')
    {
	str[len] = '\n';
	str[len + 1] = '\0';
    }

    tl_append(body, str); 
    str[0] = '\0';

    return str;
}

static Textlist* mime_debody_qp(Textlist *body)
{
    char *dec_str, *tmp_str;
    long len;
    
    Textlist *dec_body;
    Textlist *full_line;

    Textline *line, *subline;

    dec_body = xmalloc(sizeof(*dec_body));
    memset(dec_body, 0, sizeof(*dec_body));

    full_line = xmalloc(sizeof(*full_line));
    memset(full_line, 0, sizeof(*full_line));
    
    for(line = body->first; line != NULL; line = line->next)
    {
	len = strlen(line->line);
	if(mime_qp_decode(&dec_str, line->line, len) == ERROR)
	    goto fail;

	strip_crlf(dec_str);
	tl_append(full_line, dec_str);
	xfree(dec_str);

	if(streq(line->line + (len - 2), "=\n") || streq(line->line + (len - 3), "=\r\n"))
	    continue;
	
	len = tl_size(full_line);

	tmp_str = xmalloc(len + 2);
	memset(tmp_str, 0, len + 2);

	for(subline = full_line->first; subline != NULL; subline = subline->next)
	    strcat(tmp_str, subline->line);

	tmp_str[len] = '\n';

	tl_append(dec_body, tmp_str);
	xfree(tmp_str);
	tl_clear(full_line);
    }
    
    tl_clear(full_line);
    xfree(full_line);
    return dec_body;

fail:
    fglog("ERROR: decoding quoted-printable, skipping section");
    tl_clear(dec_body);
    xfree(dec_body);
    tl_clear(full_line);
    xfree(full_line);
    return NULL;
}

/* decode base64 body */

#define MIME_MAX_ENC_LEN 80

static Textlist* mime_debody_base64(Textlist *body)
{
    char *out_str;
    char *dec_str;
    char *out_ptr;
    char *dec_ptr, *dec_prev_ptr;

    Textlist *dec_body = NULL;
    Textline *line;

    int max_len = msg_get_line_length();
    int left;
    int enc_len = 0, dec_str_len = 0;

    dec_body = xmalloc(sizeof(*dec_body));
    memset(dec_body, 0, sizeof(*dec_body));
    
    /* len + \n + \0 */
    out_ptr = out_str = xmalloc(max_len + 2);
    out_str[0] = '\0';
    left = max_len;

    for(line = body->first; line != NULL; line = line->next)
    {

	enc_len = (xstrnlen(line->line, MIME_MAX_ENC_LEN) / 4) * 4;
	if(mime_b64_decode(&dec_str, line->line, enc_len) == ERROR)
	    goto exit;
    
	dec_prev_ptr = dec_str;
	while((dec_ptr = strchr(dec_prev_ptr, '\n')) != NULL)
	{
	    dec_str_len = dec_ptr - dec_prev_ptr + 1;
	    if(dec_str_len <= left)
	    {
		strncat(out_str, dec_prev_ptr, dec_str_len);
		dec_prev_ptr = dec_ptr+1;
	    }
	    else
	    {
		strncat(out_str, dec_prev_ptr, left);
		dec_prev_ptr += left;
	    }

	    mime_debody_flush_str(dec_body, out_str);
	    left = max_len;
	}

	dec_str_len = strlen(dec_str) - (dec_prev_ptr - dec_str);
	do
	{
	    if(dec_str_len < left)
	    {
		strncat(out_str, dec_prev_ptr, dec_str_len);
		out_ptr += dec_str_len;
		left -= dec_str_len;
		dec_str_len = 0;
	    }
	    else
	    {
		strncat(out_str, dec_prev_ptr, left);
		dec_prev_ptr += left;
		dec_str_len -= left;

		mime_debody_flush_str(dec_body, out_str);
		left = max_len;

	    }
	} while(dec_str_len > 0);
	
    }
    if(left != max_len)
	mime_debody_flush_str(dec_body, out_str);

exit:
    xfree(out_str);
    return dec_body;
}

static Textlist* mime_debody_section(Textlist *body, Textlist *header);

static Textlist* mime_debody_multipart(Textlist *body, MIMEInfo *mime)
{
    Textlist header = { NULL, NULL };
    Textline *line;

    Textlist *dec_body, *ptr_body, tmp_body = { NULL, NULL };
    
    char *boundary = NULL;
    char *fin_boundary = NULL;

    if(mime->type_boundary == NULL)
	return NULL;

    dec_body = xmalloc(sizeof(*dec_body));
    dec_body->first = NULL;
    dec_body->last = NULL;

    /* --boundary\0 */
    boundary = xmalloc(strlen(mime->type_boundary) + 3);
    strcpy(boundary, "--");
    strcat(boundary, mime->type_boundary);

    /* --boundary--\0 */
    fin_boundary = xmalloc(strlen(boundary) + 3);
    strcpy(fin_boundary, boundary);
    strcat(fin_boundary, "--");
    
    for(line = body->first; line != NULL; line = line->next)
	if(strneq(line->line, boundary, strlen(boundary)))
	    break;
    
    for(line = line->next; line != NULL; line = line->next)
    {
	if(!strneq(line->line, boundary, strlen(boundary)))
	{
	    tl_append(&tmp_body, line->line);
	}
	else
	{
	    header_read_list(&tmp_body, &header);
	    header_delete_from_body(&tmp_body);

	    ptr_body = mime_debody_section(&tmp_body, &header);
	    if(ptr_body != NULL)
	    {
		tl_addtl(dec_body, ptr_body);
		if(ptr_body != &tmp_body)
		    xfree(ptr_body);
	    }
	    tl_clear(&tmp_body);

	    if(strneq(line->line, fin_boundary, strlen(fin_boundary)))
		break;
	}
    }
	    
    xfree(boundary);
    xfree(fin_boundary);
    return dec_body;
}




static int mime_decharset_section(Textlist *body, MIMEInfo *mime)
{

#ifdef HAVE_ICONV

    iconv_t desc;
    const char *src_ptr;
    char *res_str, *res_ptr;
    size_t res_len, src_len;
    size_t max_len;
    int rc;
    
    Textline *line;
#endif
    
    if(body == NULL || mime == NULL)
	return ERROR;

#ifdef HAVE_ICONV
    
    /* if no charset, us-ascii assumed */
    if(mime->type_charset == NULL || strieq(mime->type_charset, INTERNAL_CHARSET))
	return OK;
    
    desc = iconv_open(INTERNAL_CHARSET, mime->type_charset);
    if(desc == (iconv_t)-1)
    {
	fglog("WARNING: iconv cannot convert from %s to %s", mime->type_charset, INTERNAL_CHARSET);
	return ERROR;
    }

    /* Converting to 8bit charset, so result always less or equal */
    max_len = msg_get_line_length();
    res_str = xmalloc(max_len + 1);

    for(line = body->first; line != NULL; line = line->next)
    {
	res_ptr = res_str;
	src_ptr = line->line;
	src_len = strlen(line->line);
	res_len = max_len;

        while(src_len > 0)
	{
	    rc = iconv(desc, &src_ptr, &src_len, &res_ptr, &res_len);
	    if(rc == -1)
	    {
		/* Only if wrong symbol (or sequence), try to skip it */
		src_len--;
		src_ptr++;
		/* reset conversion state */
		/* iconv(desc, NULL, NULL, &res_ptr, &res_len); */
	    }
	}
	res_str[max_len - res_len] = '\0';
	strcpy(line->line, res_str);
	iconv(desc, NULL, NULL, &res_ptr, &res_len);
    }
    
    iconv_close(desc);
    xfree(res_str);

#endif

    return OK;
}
    

static Textlist* mime_debody_section(Textlist *body, Textlist *header)
{

    Textlist *dec_body;
    MIMEInfo *mime;

    if((mime = get_mime_from_header(header)) == NULL)
	return NULL;

    if(strieq(mime->type_type, "text/plain") || (mime->type == NULL))
    {
	if(strieq(mime->encoding, "8bit") || strieq(mime->encoding, "7bit"))
	{
	    dec_body = body;
	}
	else if(strieq(mime->encoding, "base64"))
	{
	    dec_body = mime_debody_base64(body);
	}
	else if(strieq(mime->encoding, "quoted-printable"))
	{
	    dec_body = mime_debody_qp(body);
	}
	else
	{
	    fglog("WARNING: Skipped unsupported transfer encoding %s", mime->encoding);
	    dec_body = NULL;
	    goto exit;
	}
	mime_decharset_section(dec_body, mime);
    }
    else if(strieq(mime->type_type, "multipart/mixed"))
    {
	dec_body = mime_debody_multipart(body, mime);
    }
    else
    {
	fglog("WARNING: Skipped unsupported mime type  %s", mime->type_type);
	dec_body = NULL;
	goto exit;
    }
exit:
    put_mime(mime);
    return dec_body;
    
}


int mime_debody(Textlist *body)
{
    Textlist *dec_body;
    Textlist *header;

    if((header = header_get_list()) == NULL)
	return ERROR;
    
    if((dec_body = mime_debody_section(body, header)) == NULL)
	return ERROR;

    if(dec_body->first == NULL)
    {
	fglog("ERROR: could not decode mime body");
	xfree(dec_body);
	return ERROR;
    }

    if(dec_body != body)
    {
	tl_clear(body);
	*body = *dec_body;
	xfree(dec_body);
    }

#ifdef HAVE_ICONV
    header_alter(header, "Content-Type", INTERNAL_TYPE);
#endif
    header_alter(header, "Content-Transfer-Encoding", INTERNAL_ENCODING);

    return OK;
}

