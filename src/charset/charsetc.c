/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: charsetc.c,v 1.13 2004/08/22 20:19:10 n0ll Exp $
 *
 * Charset mapping table compiler
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
#include "getopt.h"



#define PROGRAM		"charsetc"
#define VERSION		"$Revision: 1.13 $"



/*prototypes.h*/
CharsetTable *charset_table_new	(void);
CharsetAlias *charset_alias_new	(void);
int	charset_write_bin	(char *);
/**************/



/*
 * Prototypes
 */
int	charset_parse_c		(char *);
int	charset_do_line		(char *);
int	charset_do_file		(char *);
int	charset_write_bin	(char *);
int	compile_map		(char *, char *);

void	short_usage		(void);
void	usage			(void);



/*
 * Parse character
 */
int charset_parse_c(char *s)
{
    int val, n;
    
    if(s[0] == '\\')			/* Special: \NNN or \xNN */
    {
	s++;
	val = 0;
	n = 0;
	if(s[0]=='x' || s[0]=='X')	/* Hex */
	{
	    s++;
	    while(is_xdigit(s[0]) && n<2)
	    {
		s[0] = toupper(s[0]);
		val = val * 16 + s[0] - (s[0]>'9' ? 'A'-10 : '0');
		s++;
		n++;
	    }
	    if(*s)
		return ERROR;
	}
	else				/* Octal */
	{
	    while(is_odigit(s[0]) && n<3)
	    {
		val = val * 8 +  s[0] - '0';
		s++;
		n++;
	    }
	    if(*s)
		return ERROR;
	}
    }
    else
    {
	if(s[1] == 0)			/* Single char */
	    val = s[0] & 0xff;
	else
	    return ERROR;
    }

    return val & 0xff;
}



/*
 * Process one line from charset.map file
 */
int charset_do_line(char *line)
{
    static CharsetTable *pt = NULL;
    char *key, *w1, *w2;
    CharsetAlias *pa;
    int i, c1, c2;
    
    debug(16, "charset.map line: %s", line);

    key = strtok(line, " \t");
    if(!key)
	return OK;

    /* Include map file */
    if(      strieq(key, "include") ) 
    {
	w1 = strtok(NULL, " \t");
	if( charset_do_file(w1) == ERROR)
	    return ERROR;
    }

    /* Define alias */
    else if( strieq(key, "alias") ) 
    {
	w1 = strtok(NULL, " \t");
	w2 = strtok(NULL, " \t");
	if(!w1 || !w2) 
	{
	    fprintf(stderr, "%s:%ld: argument(s) for alias missing\n",
		    PROGRAM, cf_lineno_get());
	    return ERROR;
	}
	
	pa = charset_alias_new();
	BUF_COPY(pa->alias, w1);
	BUF_COPY(pa->name, w2);
	debug(15, "new alias: alias=%s name=%s", pa->alias, pa->name);
    }

    /* Define table */
    else if( strieq(key, "table") )
    {
	w1 = strtok(NULL, " \t");
	w2 = strtok(NULL, " \t");
	if(!w1 || !w2) 
	{
	    fprintf(stderr, "%s:%ld: argument(s) for table missing\n",
		    PROGRAM, cf_lineno_get());
	    return ERROR;
	}

	pt = charset_table_new();
	BUF_COPY(pt->in, w1);
	BUF_COPY(pt->out, w2);
	debug(15, "new table: in=%s out=%s", pt->in, pt->out);
    }

    /* Define mapping for character(s) in table */
    else if( strieq(key, "map") )
    {
	w1 = strtok(NULL, " \t");
	if(!w1) 
	{
	    fprintf(stderr, "%s:%ld: argument for map missing\n",
		    PROGRAM, cf_lineno_get());
	    return ERROR;
	}

	/* 1:1 mapping */
	if(strieq(w1, "1:1"))
	{
	    for(i=0; i<MAX_CHARSET_IN; i++)
	    {
		if(pt->map[i][0] == 0)
		{
		    pt->map[i][0] = 0x80 + i;
		    pt->map[i][1] = 0;
		}
	    }
	}
	/* 1:1 mapping, but not for 0x80...0x9f */
	if(strieq(w1, "1:1-noctrl"))
	{
	    for(i=0x20; i<MAX_CHARSET_IN; i++)
	    {
		if(pt->map[i][0] == 0)
		{
		    pt->map[i][0] = 0x80 + i;
		    pt->map[i][1] = 0;
		}
	    }
	}
	/* Mapping for undefined characters */
	else if(strieq(w1, "default"))
	{
	    /**FIXME: not yet implemented**/
	}
	/* Normal mapping */
	else
	{
	    if( (c1 = charset_parse_c(w1)) == ERROR)
	    {
		fprintf(stderr, "%s:%ld: illegal char %s\n",
			PROGRAM, cf_lineno_get(), w1);
		return ERROR;
	    }
	    if( c1 < 0x80 )
	    {
		fprintf(stderr, "%s:%ld: illegal char %s, must be >= 0x80\n",
			PROGRAM, cf_lineno_get(), w1);
		return ERROR;
	    }

	    for( i=0; i<MAX_CHARSET_OUT-1 && (w2 = strtok(NULL, " \t")); i++ )
	    {
		if( (c2 = charset_parse_c(w2)) == ERROR)
		{
		    fprintf(stderr, "%s:%ld: illegal char definition %s\n",
			    PROGRAM, cf_lineno_get(), w2);
		    return ERROR;
		}
		pt->map[c1 & 0x7f][i] = c2;
	    }
	    for( ; i<MAX_CHARSET_OUT; i++ )
		pt->map[c1 & 0x7f][i] = 0;

	    debug(15, "map: %u -> %u %u %u %u",
		  c1,
		  pt->map[c1 & 0x7f][0], pt->map[c1 & 0x7f][1],
		  pt->map[c1 & 0x7f][2], pt->map[c1 & 0x7f][3] );
	}
    }
    /* Error */
    else 
    {
	fprintf(stderr, "%s:%ld: illegal key word %s\n",
		PROGRAM, cf_lineno_get(), key);
	return ERROR;
    }
    
    return OK;
}



/*
 * Process charset.map file
 */
int charset_do_file(char *name)
{
    FILE *fp;
    char *p;
    long oldn;

    if(!name)
	return ERROR;
    debug(1, "Reading charset.map file %s", name);
    
    oldn = cf_lineno_set(0);
    fp = fopen_expand_name(name, R_MODE_T, FALSE);
    if(!fp)
	return ERROR;
    
    while( (p = cf_getline(buffer, BUFFERSIZE, fp)) )
	charset_do_line(p);

    fclose(fp);
    cf_lineno_set(oldn);
    
    return OK;
}



/*
 * Compile charset.map
 */
int compile_map(char *in, char *out)
{
    /* Read charset.map and compile */
    if(charset_do_file(in) == ERROR)
    {
	fprintf(stderr, "%s: compiling map file %s failed", PROGRAM, in);
	return EXIT_ERROR;
    }

    /* Write binary output file */
    if(charset_write_bin(out) == ERROR)
    {
	fprintf(stderr, "%s: writing binary map file %s failed", PROGRAM, out);
	return EXIT_ERROR;
    }

    return EXIT_OK;
}



/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] charset.map charset.bin\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
    exit(EX_USAGE);
}


void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
	    version_global(), PROGRAM, version_local(VERSION) );
    
    fprintf(stderr, "usage:   %s [-options] charset.map charset.bin\n\n",
	    PROGRAM);
    fprintf(stderr, "\
options:  \n\
\n\
          -v --verbose                 verbose\n\
	  -h --help                    this help\n"                     );

    exit(0);
}



/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    int ret = EXIT_OK;
    char *name_in, *name_out;
    
    int option_index;
    static struct option long_options[] =
    {

	{ "verbose",      0, 0, 'v'},	/* More verbose */
	{ "help",         0, 0, 'h'},	/* Help */
	{ 0,              0, 0, 0  }
    };

    
    log_file("stderr");
    log_program(PROGRAM);
    
    while ((c = getopt_long(argc, argv, "vh",
			    long_options, &option_index     )) != EOF)
	switch (c) {
	    
	/***** Common options *****/
	case 'v':
	    verbose++;
	    break;
	case 'h':
	    usage();
	    break;
	default:
	    short_usage();
	    break;
	}

    if(optind+2 != argc)
	short_usage();

    name_in  = argv[optind++];
    name_out = argv[optind++];

    ret = compile_map(name_in, name_out);
    
    exit(ret);
}
