/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: fopen.c,v 4.12 2004/08/22 20:19:11 n0ll Exp $
 *
 * Specialized fopen()-like functions
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
 * fopen_expand_name() --- expand file name and open file
 */
FILE *fopen_expand_name(char *name, char *mode, int err_abort)
{
    char xname[MAXPATH];
    FILE *fp;

    if(!name)
	return NULL;
    
    BUF_EXPAND(xname, name);
    
    fp = fopen(xname, mode);
    if(fp == NULL)
    {
	if(err_abort)
	{
	    logit("$ERROR: can't open %s", xname);
	    exit(EX_OSFILE);
	}
	else
	{
	    logit("$WARNING: can't open %s", xname);
	}
    }

    return fp;
}



/*
 * xfopen() --- expand file name, open file, check for error
 */
FILE *xfopen(char *name, char *mode)
{
    return fopen_expand_name(name, mode, TRUE);
}
