/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 * $Id: file.c,v 4.6 2004/08/22 20:19:11 n0ll Exp $
 *
 * File-oriented functions
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
 * Rename file: NAME.pkt -> NAME.bad
 */
int rename_bad(char *name)
{
    char bad[MAXPATH];

    str_change_ext(bad, sizeof(bad), name, ".bad");
    
    logit("ERROR: bad packet renamed to %s", bad);
    if(rename(name, bad) == ERROR)
    {
	logit("$ERROR: can't rename %s -> %s", name, bad);
	return ERROR;
    }
    
    return OK;
}
