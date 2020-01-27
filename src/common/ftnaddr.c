/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Function for handling FTN addresses
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

/*
 * Initialize FTNAddr
 */
void ftnaddr_init(FTNAddr * ftn)
{
    ftn->name[0] = 0;
    node_clear(&ftn->node);
}

/*
 * Invalidate FTNAddr
 */
void ftnaddr_invalid(FTNAddr * ftn)
{
    ftn->name[0] = 0;
    node_invalid(&ftn->node);
}

/*
 * Parse string, return FTNAddr
 *
 * Formats:
 *		User Name @ Z:N/F.P
 *		User Name			(address = 0:0/0.0)
 *		User Name @ Z:N/F.P@DOMAIN
 *		(User Name @ DOMAIN#Z:N/F.P	NOT YET IMPLEMENTED)
 */
FTNAddr ftnaddr_parse(char *s)
{
    FTNAddr ftn;
    char *d;

    /* Delimiter is first '@' */
    d = strchr(s, '@');
    if (!d)
        d = s + strlen(s);

    /* Copy user name */
    str_copy_range(ftn.name, sizeof(ftn.name), s, d);
    strip_space(ftn.name);

    /* Parse address */
    if (*d == '@')
        d++;
    while (*d && is_space(*d))
        d++;
    if (*d) {
        if (asc_to_node(d, &ftn.node, FALSE) == ERROR)
            node_invalid(&ftn.node);
    } else
        node_clear(&ftn.node);

    return ftn;
}

/*
 * Output FTNAddr
 */
char *s_ftnaddr_print(FTNAddr * ftn)
{
    return s_printf("%s @ %s", ftn->name, s_znfp_print(&ftn->node, FALSE));
}

#ifdef TEST /****************************************************************/

int main(int argc, char *argv[])
{
    FTNAddr ftn;

    /* Init configuration */
    cf_initialize();
    cf_read_config_file(DEFAULT_CONFIG_MAIN);

    if (argc != 2) {
        fprintf(stderr, "usage: ftnaddr 'user name @ Z:N/F.P'\n");
        exit(1);
    }

    ftn = ftnaddr_parse(argv[1]);

    printf("FTNAddr: %s\n", s_ftnaddr_print(&ftn));

    tmps_freeall();

    exit(0);
    return 0;
}

#endif /**TEST***************************************************************/
