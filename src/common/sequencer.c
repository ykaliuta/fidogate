/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: sequencer.c,v 4.10 2004/08/22 20:19:11 n0ll Exp $
 *
 * Number sequencer using sequence file in LIBDIR
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
 * Sequencer: read number from file and increment it
 */
long sequencer(char *seqname)
{
    return sequencer_nx(seqname, TRUE);
}


long sequencer_nx(char *seqname, int err_abort)
{
    char filename[MAXPATH];
    FILE *fp;
    long seqn;

    /* Open file, create if necessary */
    BUF_EXPAND(filename, seqname);
    if( (fp = fopen(filename, RP_MODE)) == NULL )
	if(errno == ENOENT)
	    fp = fopen(filename, WP_MODE);
	    
    if(fp == NULL)
    {
	if(err_abort) 
	{
	    logit("$ERROR: can't access sequencer file %s", filename);
	    exit(EX_OSFILE);
	}
	else
	    return ERROR;
    }
    
    /* Lock file, get number and increment it */
    lock_file(fp);

    /* filename[] is also used as a buffer for reading the seq value */
    if(fgets(filename, sizeof(filename), fp))
	seqn = atol(filename);
    else
	seqn = 0;
    seqn++;
    if(seqn < 0)
	seqn = 0;
    
    rewind(fp);
    fprintf(fp, "%ld\n", seqn);
    rewind(fp);

    fclose(fp);

    return seqn;
}		
