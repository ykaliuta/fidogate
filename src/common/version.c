/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway software UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * FIDOGATE version number handling stuff
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
#include "version.h"

/*
 * version_global() --- Get global FIDOGATE version string
 */
char *version_global(void)
{
    static char id[32];

    str_printf(id, sizeof(id), "%d.%d%s",
               VERSION_MAJOR, VERSION_MINOR, EXTRAVERSION);
    return id;
}

/*
 * get_keyword_arg()
 */
static char *get_keyword_arg(char *s)
{
    char *p;

    while (*s && *s != ':')
        s++;
    if (*s == ':')
        s++;
    while (*s && *s == ' ')
        s++;

    for (p = s; *p && *p != ' '; p++) ;
    *p = 0;

    return s;
}

/*
 * version_local() --- Get local version from passed RCS/CVS Revision string
 */
char *version_local(char *rev)
{
    static char id[128];

    BUF_COPY(id, rev);

    return get_keyword_arg(id);
}

/*
 * Get major/minor version number
 */
int version_major(void)
{
    return VERSION_MAJOR;
}

int version_minor(void)
{
    return VERSION_MINOR;
}
