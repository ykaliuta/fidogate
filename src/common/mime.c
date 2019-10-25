/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: mime.c,v 5.11 2007/12/21 19:52:26 anray Exp $
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
void	   mime_free		(void);

static MIMEInfo *mime_list = NULL;



static char* mime_get_main_charset()
{
    MIMEInfo *mime;
    char *charset;
    mime = get_mime( s_header_getcomplete("MIME-Version"),
		     s_header_getcomplete("Content-Type"),
		     s_header_getcomplete("Content-Transfer-Encoding"));
    charset = strsave(mime->type_charset);
    mime_free();
    return charset;
}


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
	    else if(s[1]=='\n'           ||		/* =<LF> and */
		    (s[1]=='\r' && s[2]=='\n')  )	/* =<CR><LF> */
	    {
		break;
	    }
	}
	else if( (flags & MIME_US) && (s[0] == '_') ) /* Underscore */
	{
	    c = ' ';
	}
	else
	{					   	 /* Nothing special to do */
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
#define MIME_HEADER_STR_DELIM	"\n "
#define MIME_STRING_LIMIT 74
#define MIME_ENC_STRING_LIMIT 80
#define MIME_MAX_ENC_LEN 31

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
    if (('0' <= c) && ('9' >= c))	return (       c - '0');
    else if (('A' <= c) && ('F' >= c))	return (0x0a + c - 'A');
    else if (('a' <= c) && ('f' >= c))	return (0x0a + c - 'a');
    else                              	return ERROR;
}

#define B64_ENC_CHUNK 3
#define B64_NLET_PER_CHUNK 4

int mime_enheader(char **dst, unsigned char *src, size_t len, char *encoding)
{
    int buflen, delimlen = 0;
    char *buf = NULL;
    int padding;
    int i;
    int outpos = 0;
    char *delim = NULL;

    debug(6, "MIME: %s: %zd chars to encode (%s): %s",
	  __FUNCTION__, len, encoding, src);
    
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
            + xstrnlen(encoding, MIME_MAX_ENC_LEN)
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
        strncat(buf, encoding, MIME_MAX_ENC_LEN);
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
	 * Use this set of shifts, because loop and pointer is
	 * endian-dependend way
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

    debug(6, "MIME: %s: encoded: %s", __FUNCTION__, buf);

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
/*
 * mime word is a mime encoded string, separated by space
 * from other mime words.
 * The function takes non-whitespace starting string
 * and decodes it from mime (quoted-printable or base64),
 * if it is properly encoded.
 * Leaves the string as is, if cannot recode.
 * Allocates buffer,
 * return the buffer,
 *        the byte length of the decoded string (no final '\0'),
 *        and the charset (from the mime info)
 */
static int mime_handle_mimed_word(char *s, char **out, size_t *out_len,
				  bool *is_mime,
				  char *charset, size_t ch_size)

{
    char *p;
    size_t len;
    size_t tmp_len;
    char *buf;
    char *beg;
    char *end;
    int (*decoder)(char **dst, char *src, size_t len) = NULL;

    /* May be MIME (b64 or qp) */
    p = strchr(s + strlen(MIME_HEADER_CODE_START), '?');
    if (p == NULL)
	goto fallback;

    tmp_len = p - (s + strlen(MIME_HEADER_CODE_START));
    if (tmp_len > MIME_MAX_ENC_LEN) {
	fglog("ERROR: charset name's length too long, %zd. Do not recode", tmp_len);
	goto fallback;
    }

    tmp_len = MIN(tmp_len + 1, ch_size);
    snprintf(charset, tmp_len, "%s", s + strlen(MIME_HEADER_CODE_START));

    /* Check if b64 or qp */
    if (strnieq(p,
		MIME_HEADER_CODE_MIDDLE_QP,
		strlen(MIME_HEADER_CODE_MIDDLE_QP))) {
	/* May be qp */
	decoder = mime_qp_decode;
	beg = p + strlen(MIME_HEADER_CODE_MIDDLE_QP);

    } else if (strnieq(p,
		       MIME_HEADER_CODE_MIDDLE_B64,
		       strlen(MIME_HEADER_CODE_MIDDLE_B64))) {
	/* May be b64 */
	decoder = mime_b64_decode;
	beg = p + strlen(MIME_HEADER_CODE_MIDDLE_B64);
    } else {
	fglog("ERROR: subject looks like mime, but does not have proper header");
	goto fallback;
    }

    end = strchr(beg, '?');

    if (end == NULL ||
	!strnieq(end,
		 MIME_HEADER_CODE_END,
		 strlen(MIME_HEADER_CODE_END))) {
	fglog("ERROR: subject looks like mime, but does not have proper ending");
	goto fallback;
    }
    if (decoder(&buf, beg, end - beg) == ERROR) {
	fglog("ERROR: subject mime deconding failed");
	goto fallback;
    }

    *out_len = strlen(buf);
    *out = buf;
    *is_mime = true;
    return end + strlen(MIME_HEADER_CODE_END) - s;

fallback:
    /* keep it as is */
    debug(6, "Could not unmime subject '%s', leaving as is", s);
    len = strlen(s);
    *out = xmalloc(len);
    memcpy(*out, s, len); /* no '\0' */
    snprintf(charset, ch_size, "%s", INTERNAL_CHARSET);
    *is_mime = false;
    *out_len = len;

    return len;
}

/*
 * Fetches the mime word from the string. It can be mime-encoded
 * or plain part, separated with "space".
 * _Must_ start with non-space.
 * For mime-encoded parts decode it and fetch the charset.
 * For plain parts copy them as is, take the global charset.
 * Allocates the final buffer.
 * @returns the number of bytes, handled in the source string.
 */
static int mime_handle_word(char *s, char **out, size_t *out_len,
			    bool *is_mime, char *charset, size_t ch_size)
{
    char *plain_charset;
    bool free_charset = true;
    char *p;
    size_t len;
    char *res;

    if (strnieq(s,
		MIME_HEADER_CODE_START,
		strlen(MIME_HEADER_CODE_START))) {
	return mime_handle_mimed_word(s,
				      out, out_len,
				      is_mime,
				      charset, ch_size);
    }

    plain_charset = mime_get_main_charset();
    if (plain_charset == NULL) {
	plain_charset = INTERNAL_CHARSET;
	free_charset = false;
    }

    strncpy(charset, plain_charset, ch_size - 1);
    charset[ch_size - 1] = '\0';

    if (free_charset)
	free(plain_charset);

    *is_mime = false;

    /* if there are several words, handle only one. */
    p = strchr(s, ' ');
    if (p != NULL)
	    len = p - s;
    else
	    len = strlen(s);

    res = xmalloc(len + 1);
    str_copy(res, len + 1, s);

    *out_len = len;
    *out = res;

    return *out_len;
}

/* source @s must be '\0'-terminated */
char *mime_deheader(char *d, size_t d_max, char *s)
{
    char *save_d = d;
    bool is_mime = false;
    bool is_prev_mime = false;
    char *buf;
    size_t len;
    char charset[MIME_MAX_ENC_LEN + 1];
    int rc;
    size_t d_left = d_max - 1;
    size_t d_size;
    size_t s_handled;

    while (d_left != 0 && *s != '\0') {
	if (isspace(*s)) {
	    /* just skip spaces between mime words */
	    if (!is_mime) {
		*d++ = *s;
		d_left--;
	    }
	    s++;
	    continue;
	}

	is_prev_mime = is_mime;

        /* it means mime "word" -- encoded or plain part */
	s_handled = mime_handle_word(s, &buf, &len, &is_mime,
				     charset, sizeof(charset));

	/* keep at least one space between mime and non-mime words */
	if (is_prev_mime && !is_mime) {
	    *d++ = ' ';
	    d_left--;

	    if (d_left == 0)
		break;
	}

	debug(6, "subject charset: %s", charset);
	d_size = d_left;
	rc = charset_recode_string(d, &d_size, buf, &len,
				   charset, INTERNAL_CHARSET);
	free(buf);

	/* unused space was returned in d_size */
	d += d_left - d_size;

	if (rc != OK) {
	    debug(6, "Could not recode subject %s", buf);
	    goto out;
	}

	d_left = d_size;
	s += s_handled;
    }

out:
    *d = '\0';
    return save_d;
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
    if(p != NULL)
    {
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
	
	tmp_line = tl_get(&header_line, "charset", strlen("charset"));
	if(tmp_line != NULL)
	{
	    tmp_str = mime_attr_value(tmp_line->line);
	    mime->type_charset = s_copy(tmp_str);
	    xfree(tmp_str);
	}
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
	tmp_line = tl_get(&header_line, "filename", strlen("filename"));
	if(tmp_line != NULL)
	    tmp_str = mime_attr_value(tmp_line->line);
	mime->disposition_filename = s_copy(tmp_str);
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

    if(str[len - 2] == '\r')
    {
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


static Textlist* mime_debody_base64(Textlist *body)
{
    char *out_str;
    char *dec_str;
    char *out_ptr;
    char *dec_ptr, *dec_prev_ptr;

    Textlist *dec_body = NULL;
    Textline *line;

    int max_len = MAX_LINE_LENGTH;
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

	enc_len = (xstrnlen(line->line, MIME_ENC_STRING_LIMIT) / 4) * 4;
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

    char *res_str = NULL;
    size_t res_len, src_len;
    size_t max_len;
    int rc;
    
    Textline *line;
    
    if(body == NULL || mime == NULL)
	return ERROR;

    /* if no charset, us-ascii assumed */
    if(mime->type_charset == NULL || strieq(mime->type_charset, INTERNAL_CHARSET))
	return OK;

   
    max_len = MAX_LINE_LENGTH;
    res_str = xmalloc(max_len + 1);

    for(line = body->first; line != NULL; line = line->next)
    {
	src_len = strlen(line->line);
	res_len = max_len;

	rc = charset_recode_string(res_str, &res_len, line->line, &src_len,
				   mime->type_charset, INTERNAL_CHARSET);

	if(rc == ERROR)
	{
	    if(src_len != 0 && res_len == 0)
		fglog("WARNING: no space in the destination buffer for iconv");
	    else
		goto exit; /* something really wrong */
	}
	
	/* Converting to 8bit charset, so result always less or equal */
	strcpy(line->line, res_str);
    }

    rc = OK;
exit:
    xfree(res_str);
    return rc;
}
    

static Textlist* mime_debody_section(Textlist *body, Textlist *header)
{

    Textlist *dec_body;
    MIMEInfo *mime;

    if((mime = get_mime_from_header(header)) == NULL)
	return NULL;

    if((mime->type_type == NULL) || strieq(mime->type_type, "text/plain")
       || (mime->type == NULL))
    {
	if((mime->encoding == NULL) || strieq(mime->encoding, "8bit")
	   || strieq(mime->encoding, "7bit"))
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
    else if(strieq(mime->type_type, "multipart/mixed") ||
	    strieq(mime->type_type, "multipart/alternative"))
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
    mime_free();
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

    header_alter(header, "Content-Type", INTERNAL_TYPE);
    header_alter(header, "Content-Transfer-Encoding", INTERNAL_ENCODING);

    return OK;
}

void mime_free(void)
{
    MIMEInfo *mime, *n;
    
    for(mime=mime_list; mime; mime=n)
    {
	n = mime->next;

	xfree(mime->version);
	xfree(mime->type);
	xfree(mime->type_type);
	xfree(mime->type_charset);
	xfree(mime->type_boundary);
	xfree(mime->encoding);
	xfree(mime->disposition);
	xfree(mime->disposition_filename);
	xfree(mime);
    }

}
