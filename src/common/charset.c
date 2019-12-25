/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: charset.c,v 5.2 2004/11/23 00:50:40 anray Exp $
 *
 * NEW charset.c code using charset.bin mapping file
 *
 *****************************************************************************
 * Copyright (C) 1990-2001
 *  _____ _____
 * |     |___  |   Martin Junius             FIDO:      2:2452/110
 * | | | |   | |   Radiumstr. 18             Internet:  mj@fido.de
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
 * Alias linked list
 */
static CharsetAlias *charset_alias_list = NULL;
static CharsetAlias *charset_alias_last = NULL;

/*
 * Table linked list
 */
static CharsetTable *charset_table_list = NULL;
static CharsetTable *charset_table_last = NULL;

/*
 * Current charset mapping table
 */
static CharsetTable *charset_table_used = NULL;
static char *orig_in;
static char *orig_out;

/*
 * Alloc new CharsetTable and put into linked list
 */
CharsetTable *charset_table_new(void)
{
    CharsetTable *p;

    /* Alloc and clear */
    p = (CharsetTable *)xmalloc(sizeof(CharsetTable));
    memset(p, 0, sizeof(CharsetTable));
    p->next = NULL;			/* Just to be sure */
    
    /* Put into linked list */
    if(charset_table_list)
	charset_table_last->next = p;
    else
	charset_table_list       = p;
    charset_table_last       = p;

    return p;
}



/*
 * Alloc new CharsetAlias and put into linked list
 */
CharsetAlias *charset_alias_new(void)
{
    CharsetAlias *p;

    /* Alloc and clear */
    p = (CharsetAlias *)xmalloc(sizeof(CharsetAlias));
    memset(p, 0, sizeof(CharsetAlias));
    p->next = NULL;			/* Just to be sure */
    
    /* Put into linked list */
    if(charset_alias_list)
	charset_alias_last->next = p;
    else
	charset_alias_list       = p;
    charset_alias_last       = p;

    return p;
}



/*
 * Write binary mapping file
 */
int charset_write_bin(char *name)
{
    FILE *fp;
    CharsetTable *pt;
    CharsetAlias *pa;
    
    debug(14, "Writing charset.bin file %s", name);
    
    fp = fopen_expand_name(name, W_MODE, FALSE);
    if(!fp)
	return ERROR;

    /* Write aliases */
    for(pa = charset_alias_list; pa; pa=pa->next)
    {
	fputc(CHARSET_FILE_ALIAS, fp);
	fwrite(pa, sizeof(CharsetAlias), 1, fp);
	if(ferror(fp))
	{
	    fclose(fp);
	    return ERROR;
	}
    }
    /* Write tables */
    for(pt = charset_table_list; pt; pt=pt->next)
    {
	fputc(CHARSET_FILE_TABLE, fp);
	fwrite(pt, sizeof(CharsetTable), 1, fp);
	if(ferror(fp))
	{
	    fclose(fp);
	    return ERROR;
	}
    }

    fclose(fp);
    return OK;
}



/*
 * Read binary mapping file
 */
int charset_read_bin(char *name)
{
    FILE *fp;
    int c, n;
    CharsetTable *pt;
    CharsetAlias *pa;

    debug(14, "Reading charset.bin file %s", name);
    
    fp = fopen_expand_name(name, R_MODE, TRUE);

    while( (c = fgetc(fp)) != EOF )
    {
	switch(c)
	{
	case CHARSET_FILE_ALIAS:
	    pa = charset_alias_new();
	    n = fread((void *)pa, sizeof(CharsetAlias), 1, fp);
	    pa->next = NULL;			/* overwritten by fread() */
	    if(n != 1)
		return ERROR;
	    debug(15, "read charset alias: %s -> %s", pa->alias, pa->name);
	    break;
	case CHARSET_FILE_TABLE:
	    pt = charset_table_new();
	    n = fread((void *)pt, sizeof(CharsetTable), 1, fp);
	    pt->next = NULL;			/* overwritten by fread() */
	    if(n != 1)
		return ERROR;
	    debug(15, "read charset table: %s -> %s", pt->in, pt->out);
	    break;
	default:
	    return ERROR;
	    break;
	}
    }
    
    if(ferror(fp))
	return ERROR;
    fclose(fp);
    return OK;
}



/*
 * Convert to MIME quoted-printable =XX if qp==TRUE
 */
static char *charset_qpen(int c, int qp)
{
    static char buf[4];

    c &= 0xff;
    
    if( qp && (c == '=' || c >= 0x80) )
	str_printf(buf, sizeof(buf), "=%2.2X", c & 0xff);
    else
    {
	buf[0] = c;
	buf[1] = 0;
    }
    
    return buf;
}



/*
 * Map single character
 */
char *charset_map_c(int c, int qp)
{
    static char buf[MAX_CHARSET_OUT * 4];
    char *s;
    
    c &= 0xff;
    buf[0] = 0;
    
    if(charset_table_used && c>=0x80)
    {
	s = charset_table_used->map[c - 0x80];
	while(*s)
	    BUF_APPEND(buf, charset_qpen(*s++, qp));
    }
    else
    {
	BUF_COPY(buf, charset_qpen(c, qp));
    }

    return buf;
}



/*
 * Translate string
 */
char *
xlat_s(char *s1, char *s2)
{
    char *dst;
    size_t src_len;
    size_t dst_len;
    int rc;

    if (s2 != NULL)
	free(s2);

    if (s1 == NULL)
	return NULL;

    src_len = strlen(s1);
    dst_len = src_len * MAX_CHARSET_OUT;
    dst = xmalloc(dst_len + 1);
    src_len++; /* recode also final \0 */

    rc = charset_recode_string(dst, &dst_len, s1, &src_len,
			       orig_in, orig_out);
    if (rc == OK)
	return dst;

    free(dst);
    return NULL;
}



/*
 * Search alias
 */
char *charset_alias_fsc(char *name)
{
    CharsetAlias *pa;

    /* Search for aliases */
    for(pa = charset_alias_list; pa; pa=pa->next)
    {
	if(strieq(pa->name, name))
	    return pa->alias;
    }

    return name;
}

char *charset_alias_rfc(char *name)
{
    CharsetAlias *pa;

    /* Search for aliases */
    for(pa = charset_alias_list; pa; pa=pa->next)
    {
	if(strieq(pa->alias, name))
	    return pa->name;
    }

    return name;
}



/*
 * Set character mapping table
 */
void charset_set_in_out(char *in, char *out)
{
    CharsetTable *pt;
    CharsetAlias *pa;

    if(!in || !out)
	return;

    debug(5, "charset: in=%s out=%s", in, out);

    orig_in = in;
    orig_out = out;
    
    /* Search for aliases */
    for(pa = charset_alias_list; pa; pa=pa->next)
    {
	if(strieq(pa->alias, in))
	    in = pa->name;
	if(strieq(pa->alias, out))
	    out = pa->name;
    }

    /* Search for matching table */
    for(pt = charset_table_list; pt; pt=pt->next)
    {
	if(strieq(pt->in, in) && strieq(pt->out, out))
	{
	    debug(5, "charset: table found in=%s out=%s", pt->in, pt->out);
	    charset_table_used = pt;
	    return;
	}
    }

    charset_table_used = NULL;
    return;
}



/*
 * Initialize charset mapping
 */
void charset_init(void)
{
    if(charset_read_bin( cf_p_charsetmap() ) == ERROR) 
    {
	fglog("ERROR: reading from %s", cf_p_charsetmap());
	exit(EX_SOFTWARE);
    }

    charset_table_used = NULL;
}


struct str_to_str {
    char *key;
    char *val;
};

static struct str_to_str level1_map[] = {
    { "ASCII", "ASCII" },
    { "DUTCH", "ISO646-DK" },
    { "FINNISH", "ISO646-FI" },
    { "FRENCH", "ISO646-FR" },
    { "CANADIAN", "ISO646-CA" },
    { "GERMAN", "ISO646-DE" },
    { "ITALIAN", "ISO646-IT" },
    { "NORWEIG", "ISO646-NO" },
    { "PORTU", "ISO646-PT" },
    { "SPANISH", "ISO646-ES" },
    { "SWEDISH", "ISO646-SE" },
    { "SWISS", "ISO646-CN" },
    { "UK", "ISO646-GB" },
};

static char *charset_level1_to_iconv(char *charset)
{
    int i;

    for (i = 0; i < sizeof(level1_map)/sizeof(level1_map[0]); i++)
	if (stricmp(level1_map[i].key, charset) == 0)
	    return level1_map[i].val;
    return NULL;
}


/*
 * Get charset name from ^ACHRS kludge line
 */
char *charset_chrs_name(char *s)
{
    static char name[MAXPATH];
    char *p;
    int level;
    
    while(is_space(*s))
	s++;
    debug(5, "FSC-0054 ^ACHRS/CHARSET: %s", s);

    BUF_COPY(name, s);
    p = strtok(name, " \t");
    if(!p)
	return NULL;

    p = strtok(NULL, " \t");
    if(!p)
	/* In this case it's an FSC-0050 kludge without the class code.
	 * Treat it like FSC-0054 level 2. */
	level = 2;
    else
	level = atoi(p);

    switch (level) {
    case 1:
	p = charset_level1_to_iconv(name);
	debug(5, "FSC-0054 level 1 charset=%s (level 2: %s)",
	      name, p);
	return p;

    case 2:
	debug(5, "FSC-0054 level 2 charset=%s", name);
	return name;
    }

    return NULL;
}




#ifdef TEST
/*
 * Charset mapping test
 */
int main(int argc, char *argv[])
{
    char *in  = "ibmpc";
    char *out = "iso-8859-1";
    
    if(argc < 2) 
    {
	fprintf(stderr, "usage: testcharset CHARSET.BIN [IN] [OUT]\n");
	exit(EXIT_ERROR);
    }
    
    verbose = 15;

/*    charset_init(); */
    
    if(charset_read_bin(argv[1]) == ERROR) 
    {
	fprintf(stderr, "testcharset: can't read %s\n", argv[1]);
	exit(EXIT_ERROR);
    }
    
    if(argc > 2)
	in = argv[2];
    if(argc > 3)
	out = argv[3];

    charset_set_in_out(in, out);

    while(TRUE)
    {
	printf("Enter char: ");
	fflush(stdout);
	fgets(buffer, sizeof(buffer), stdin);

	if(buffer[0] == '\n')
	    break;
	
	printf("qp=FALSE  : %s\n", charset_map_c(buffer[0], FALSE));
	printf("qp=TRUE   : %s\n", charset_map_c(buffer[0], TRUE ));
    }
    
    exit(EXIT_OK);}
#endif /**TEST**/

void charset_free(void)
{
    CharsetAlias *pa, *pa1;
    CharsetTable *pt, *pt1;

    for(pa = charset_alias_list; pa; pa=pa1)
    {
	pa1=pa->next;
	xfree(pa);
    }
    for(pt = charset_table_list; pt; pt=pt1)
    {
	pt1=pt->next;
	xfree(pt);
    }
}

#ifdef HAVE_ICONV
static int _charset_recode_iconv(char *dst, size_t *dstlen,
				 char *src, size_t *srclen,
				 char *from, char *_to)
{
    int rc;
    iconv_t desc;
    size_t size;
    char *to;

    debug(6, "Using ICONV");

    size = strlen(_to) + sizeof("//TRANSLIT");
    to = xmalloc(size);
    sprintf(to, "%s//TRANSLIT", _to);

    desc = iconv_open(to, from);
    if(desc == (iconv_t)-1)
    {
	debug(6, "WARNING: iconv cannot convert from %s to %s", from, to);
	return ERROR;
    }

    while(*srclen > 0)
    {
	rc = iconv(desc, &src, srclen, &dst, dstlen);
	if(rc != -1)
	    continue;

	if((errno == E2BIG) || (*dstlen == 0))
	{
	    rc = ERROR;
	    goto exit;
	}

	/* Only if wrong symbol (or sequence), try to skip it */
	(*srclen)--;
	src++;

	*dst++ = '?';
	(*dstlen)--;
    }

    /*
     * write sequence to get to the initial state if needed
     * https://www.gnu.org/software/libc/manual/html_node/iconv-Examples.html
     */
    iconv(desc, NULL, NULL, &dst, dstlen);
    rc = OK;

exit:
    *dst = '\0';
    iconv_close(desc);
    free(to);

    return rc;
}

static int charset_recode_iconv(char *dst, size_t *dstlen,
				char *src, size_t *srclen,
				char *from, char *to)
{
    int rc;
    char *p;
    char *buf;
    size_t len;
    size_t off;

    rc = _charset_recode_iconv(dst, dstlen, src, srclen, from, to);
    if (rc == OK)
	return OK;

    /* Heuristic, LATIN-1 -> LATIN1 */
    p = strchr(from, '-');
    if (p == NULL)
	return ERROR;

    off = p - from;
    len = strlen(from);

    buf = xmalloc(len + 1);
    memcpy(buf, from, off);
    memcpy(buf + off, p + 1, len - off - 1);
    buf[len - 1] = '\0';

    rc = _charset_recode_iconv(dst, dstlen, src, srclen, buf, to);
    free(buf);

    return rc;
}

#else
static int charset_recode_iconv(char *dst, size_t *dstlen,
				char *src, size_t *srclen,
				char *from, char *to)
{
	return ERROR;
}
#endif

/* uses internal recode */
static int charset_recode_int(char *dst, size_t *dstlen,
			      char *src, size_t *srclen,
			      char *from, char *to)
{

    charset_set_in_out(from, to);
    *dst = '\0';

    for(; (*srclen > 0 ) && (*dstlen > 0); (*srclen)--, (*dstlen)--)
	strcat(dst, charset_map_c(*(src++), 0));

    return OK;
}

/*
 * get source string, lenght of it, buffer for the destination string
 * and its length.
 * Return in srclen -- rest of undecoded characters (0 if ok)
 * in dstlen -- number of unused bytes in the dst buffer
 * The argument's order is like in str/mem functions
 *
 * Adjust given length to string's length
 */
int charset_recode_string(char *dst, size_t *dstlen,
			  char *src, size_t *srclen,
			  char *from, char *to)
{
    int rc;
    size_t len;

    if(src == NULL || dst == NULL || srclen == NULL || dstlen == NULL)
	return ERROR;

    if(*srclen == 0 || *dstlen == 0)
	return ERROR;

    debug(6, "mime charset: recoding from %s to %s", from, to);

    if (strieq(from, to)) {
	len = MIN(*dstlen, *srclen);
	memcpy(dst, src, len);
	*dstlen -= len;
	*srclen -= len;
	return OK;
    }

    rc = charset_recode_iconv(dst, dstlen, src, srclen, from, to);
    if (rc == OK)
	    return OK;

    rc = charset_recode_int(dst, dstlen, src, srclen, from, to);
    return rc;
}

int charset_is_7bit(char *buffer, size_t len)
{
     int i;

     if(buffer == NULL)
	  return TRUE;

     for(i = 0; i < len; i++)
	  if(buffer[i] & 0x80)
	       return FALSE;
     return TRUE;
}

enum utf8_state {
    START_SEQ,
    PROCESS_SEQ,
    FINISH,
    ERR,
};

static enum utf8_state utf8_check_start(unsigned char c, size_t *n)
{
    size_t num;

    if ((c & 0x80) == 0)
	num = 1;
    else if (((c & 0xc0) == 0xc0) && ((c & 0x20) == 0))
	num = 2;
    else if (((c & 0xe0) == 0xe0) && ((c & 0x10) == 0))
	num = 3;
    else if (((c & 0xf0) == 0xf0) && ((c & 0x08) == 0))
	num = 4;
    else
	return ERR;

    *n = num;
    return PROCESS_SEQ;
}

static bool utf8_check_rest_byte(unsigned char c)
{
    return ((c & 0x80) == 0x80) && ((c & 0x40) == 0);
}

static bool utf8_check_rest_bytes(char *s, size_t len, size_t i, size_t num)
{
    while (num--) {
	if (s[i] == '\0' || i == len)
	    return false;
	if (!utf8_check_rest_byte(s[i]))
	    return false;
	i++;
    }
    return true;
}

bool charset_is_valid_utf8(char *s, size_t len)
{
    enum utf8_state state = START_SEQ;
    size_t i;
    size_t num;
    static const void *const states[] = {
	[START_SEQ] = &&START_SEQ,
	[PROCESS_SEQ] = &&PROCESS_SEQ,
	[FINISH] = &&FINISH,
	[ERR] = &&ERR,
    };

    i = 0;
    goto START_SEQ;

START_SEQ:
    if (s[i] == '\0' || i == len)
	goto FINISH;
    state = utf8_check_start(s[i], &num);
    goto *states[state];

PROCESS_SEQ:
    i++;
    num--;
    if (!utf8_check_rest_bytes(s, len, i, num))
	goto ERR;
    i += num;
    goto START_SEQ;

FINISH:
    return true;
ERR:
    return false;
}
