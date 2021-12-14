/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: history.c,v 4.14 2004/08/22 20:19:14 n0ll Exp $
 *
 * MSGID history functions and dupe checking
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
#include "dbz.h"



/*
 * history file
 */
static FILE *hi_file = NULL;



/*
 * Initialize MSGID history
 */
int hi_init(void)
{
    FILE *fp;
    
    /* Test for history.dir, history.pag */
    BUF_EXPAND(buffer, cf_p_history());
    BUF_APPEND(buffer, ".dir");
    if(check_access(buffer, CHECK_FILE) != TRUE)
    {
	/* Doesn't exist, create */
	if( (fp = fopen(buffer, W_MODE)) == NULL )
	{
	    logit("$ERROR: creating MSGID history %s failed", buffer);
	    return ERROR;
	}
	else
	    logit("creating MSGID history %s", buffer);
    }
    
    BUF_EXPAND(buffer, cf_p_history());
    BUF_APPEND(buffer, ".pag");
    if(check_access(buffer, CHECK_FILE) != TRUE)
    {
	/* Doesn't exist, create */
	if( (fp = fopen(buffer, W_MODE)) == NULL )
	{
	    logit("$ERROR: creating MSGID history %s failed", buffer);
	    return ERROR;
	}
	else
	    logit("creating MSGID history %s", buffer);
    }

    /* Open the history text file */
    BUF_EXPAND(buffer, cf_p_history());
    if( (hi_file = fopen(buffer, A_MODE)) == NULL ) 
    {
	logit("$ERROR: open MSGID history %s failed", buffer);
	return ERROR;
    }

    /* Open the DBZ file */
    dbzincore(1);
    /**dbzwritethrough(1);**/
    if (dbminit(buffer) < 0) {
	logit("$ERROR: dbminit %s failed", buffer);
	return ERROR;
    }


    return OK;
}



/*
 * Close MSGID history
 */
int hi_close(void)
{
    int ret = OK;
    
    if(hi_file)
    {
	if(fclose(hi_file) == ERROR) 
	{
	    logit("$ERROR: close MSGID history failed");
	    ret = ERROR;
	}

	if (dbzsync())
	{
	    logit("$ERROR: dbzsync MSGID history failed");
	    ret = ERROR;
	}
	if (dbmclose() < 0)
	{
	    logit("$ERROR: dbmclose MSGID history failed");
	    ret = ERROR;
	}

	hi_file = NULL;
    }

    return ret;
}



/*
 * Write record to MSGID history
 */
static int hi_write_t(time_t t, time_t msgdate, char *msgid)
{
    long offset;
    int ret;
    datum key, val;

    /* Get offset in history text file */
    if( (offset = ftell(hi_file)) == ERROR)
    {
	logit("$ERROR: ftell MSGID history failed");
	return ERROR;
    }

    /* Write MSGID line to history text file */
    debug(7, "history: offset=%ld: %s %ld", offset, msgid, t);
    ret = fprintf(hi_file, "%s\t%ld\n", msgid, t);
    if (ret == ERROR || fflush(hi_file) == ERROR)
    {
	logit("$ERROR: write to MSGID history failed");
	return ERROR;
    }

    /* Write database record */
    key.dptr  = msgid;				/* Key */
    key.dsize = strlen(msgid) + 1;
    val.dptr  = (char *)&offset;		/* Value */
    val.dsize = sizeof offset;
    if (dbzstore(key, val) < 0) {
	logit("ERROR: dbzstore of record for MSGID history failed");
	return ERROR;
    }

    /**FIXME: dbzsync() every n msgids */
    
    return OK;
}


int hi_write(time_t msgdate, char *msgid)
    /* msgdate currently not used */
{
    TIMEINFO ti;

    GetTimeInfo(&ti);

    return hi_write_t(ti.time, msgdate, msgid);
}



/*
 * Test if MSGID is already in database
 */
int hi_test(char *msgid)
{
    datum key, val;

    key.dptr  = msgid;				/* Key */
    key.dsize = strlen(msgid) + 1;
    val = dbzfetch(key);
    return val.dptr != NULL;
}




/*****TEST********************************************************************/
#ifdef TEST
#include "getopt.h"



/*
 * history test
 */
int main(int argc, char *argv[])
{
    char *c_flag=NULL;
    char *S_flag=NULL, *L_flag=NULL;
    int i, c;
    char *m;
    time_t t = 0;

    /* Init configuration */
    cf_initialize();

    while ((c = getopt(argc, argv, "t:vc:S:L:")) != EOF)
	switch (c) {
	case 't':
	    t = atol(optarg);
	    break;
	    
	/***** Common options *****/
	case 'v':
	    verbose++;
	    break;
	case 'c':
	    c_flag = optarg;
	    break;
	case 'S':
	    S_flag = optarg;
	    break;
	case 'L':
	    L_flag = optarg;
	    break;
	default:
	    exit(EX_USAGE);
	    break;
	}

    /*
     * Read config file
     */
    if(L_flag)				/* Must set libdir beforehand */
	cf_s_libdir(L_flag);
    cf_read_config_file(c_flag ? c_flag : DEFAULT_CONFIG_MAIN);

    /*
     * Process config options
     */
    if(L_flag)
	cf_s_libdir(L_flag);
    if(S_flag)
	cf_s_spooldir(S_flag);

    cf_debug();


    hi_init();

    if(optind < argc) 
    {
	for(i=optind; i<argc; i++)
	{
	    m = argv[i];
	    if(hi_test(m))
	    {
		debug(1, "dupe: %s", m);
	    }
	    else
	    {
		if(t)
		{
		    debug(2, "new: %s (time=%ld)", m, t);
		    hi_write_t(t, 0, m);
		}
		else
		{
		    debug(2, "new: %s (time=now)", m);
		    hi_write(0, m);
		}
	    }
	}
    }
    else 
    {
	while( fgets(buffer, sizeof(buffer), stdin) )
	{
	    strip_crlf(buffer);
	    m = buffer;
	    if(hi_test(m))
	    {
		debug(1, "dupe: %s", m);
	    }
	    else
	    {
		if(t)
		{
		    debug(2, "new: %s (time=%ld)", m, t);
		    hi_write_t(t, 0, m);
		}
		else
		{
		    debug(2, "new: %s (time=now)", m);
		    hi_write(0, m);
		}
	    }
	}
	
    }
    
    
    hi_close();
    
    exit(0);
}
#endif /**TEST**/
/*****************************************************************************/
