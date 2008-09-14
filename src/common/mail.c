/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: mail.c,v 4.14 2004/08/22 20:19:11 n0ll Exp $
 *
 * Create RFC messages in mail/news dir
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



char mail_dir[MAXPATH];
char news_dir[MAXPATH];

static char m_name[MAXPATH];
static char m_tmp [MAXPATH];
static FILE *m_file = NULL;

static char n_name[MAXPATH];
static char n_tmp [MAXPATH];
static FILE *n_file = NULL;



/*
 * Open RFC mail file
 */
int mail_open(int sel)
{
    long n;
    
    switch(sel)
    {
    case 'm':
    case 'M':
	n = sequencer(DEFAULT_SEQ_MAIL);
	str_printf(m_tmp,  sizeof(m_tmp),  "%s/%08ld.tmp", mail_dir, n);
	str_printf(m_name, sizeof(m_name), "%s/%08ld.rfc", mail_dir, n);
	m_file = fopen(m_tmp, W_MODE);
	if(!m_file) {
	    logit("$Can't create mail file %s", m_tmp);
	    return ERROR;
	}
	break;
	
    case 'n':
    case 'N':
	n = sequencer(DEFAULT_SEQ_NEWS);
	str_printf(n_tmp,  sizeof(n_tmp),  "%s/%08ld.tmp", news_dir, n);
	str_printf(n_name, sizeof(n_name), "%s/%08ld.rfc", news_dir, n);
	n_file = fopen(n_tmp, W_MODE);
	if(!n_file) {
	    logit("$Can't create mail file %s", n_tmp);
	    return ERROR;
	}
	break;

    default:
	logit("mail_open(%d): illegal value", sel);
	return ERROR;
	break;
    }

    return OK;
}



/*
 * Return mail tmp name
 */
char *mail_name(int sel)
{
    switch(sel)
    {
    case 'm':
    case 'M':
	return m_tmp;
	break;
    case 'n':
    case 'N':
	return n_tmp;
	break;
    }

    return NULL;
}



/*
 * Return mail file pointer
 */
FILE *mail_file(int sel)
{
    switch(sel)
    {
    case 'm':
    case 'M':
	return m_file;
	break;
    case 'n':
    case 'N':
	return n_file;
	break;
    }

    return NULL;
}



/*
 * Close mail file
 */
int mail_close(int sel)
{
    int err=ERROR;
    
    switch(sel)
    {
    case 'm':
    case 'M':
	fclose(m_file);
	err = rename(m_tmp, m_name);
	if(err == ERROR)
	    logit("$Can't rename mail file %s to %s", m_tmp, m_name);

	m_tmp[0]  = 0;
	m_name[0] = 0;
	m_file    = NULL;
	break;

    case 'n':
    case 'N':
	fclose(n_file);
	err = rename(n_tmp, n_name);
	if(err == ERROR)
	    logit("$Can't rename mail file %s to %s", n_tmp, n_name);

	n_tmp[0]  = 0;
	n_name[0] = 0;
	n_file    = NULL;
	break;
    }

    return err;
}
