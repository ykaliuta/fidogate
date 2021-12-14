/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: charset.c,v 1.13 2004/08/22 20:19:10 n0ll Exp $
 *
 * NEW charset.c code using charset.bin mapping file
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
char *charset_qpen(int c, int qp)
{
    static char buf[4];

    c &= 0xff;
    
    if( qp && (c == '=' || c >= 0x80) )
	str_printf(buf, sizeof(buf), "=%02.2X", c & 0xff);
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
int charset_set_in_out(char *in, char *out)
{
    CharsetTable *pt;
    CharsetAlias *pa;

    if(!in || !out)
	return ERROR;

    debug(5, "charset: in=%s out=%s", in, out);
    
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
	    return OK;
	}
    }

    charset_table_used = NULL;
    return ERROR;
}



/*
 * Initialize charset mapping
 */
void charset_init(void)
{
    if(charset_read_bin( cf_p_charsetmap() ) == ERROR) 
    {
	logit("ERROR: reading from %s", cf_p_charsetmap());
	exit(EX_SOFTWARE);
    }

    charset_table_used = NULL;
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
    
    if(level == 2)
    {
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
