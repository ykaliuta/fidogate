/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * MSGID history functions and dupe checking
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

/*****TEST********************************************************************/
#ifdef TEST
#include "getopt.h"

/*
 * history test
 */
int main(int argc, char *argv[])
{
    char *c_flag = NULL;
    char *S_flag = NULL, *L_flag = NULL;
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
            return EX_USAGE;
            break;
        }

    /*
     * Read config file
     */
    if (L_flag)                 /* Must set libexecdir beforehand */
        cf_s_libexecdir(L_flag);
    cf_read_config_file(c_flag ? c_flag : DEFAULT_CONFIG_MAIN);

    /*
     * Process config options
     */
    if (L_flag)
        cf_s_libexecdir(L_flag);
    if (S_flag)
        cf_s_spooldir(S_flag);

    cf_debug();

    hi_init_history();

    if (optind < argc) {
        for (i = optind; i < argc; i++) {
            m = argv[i];
            if (hi_test(m)) {
                debug(1, "dupe: %s", m);
            } else {
                if (t) {
                    debug(2, "new: %s (time=%ld)", m, t);
                    hi_write_t(t, 0, m);
                } else {
                    debug(2, "new: %s (time=now)", m);
                    hi_write(0, m);
                }
            }
        }
    } else {
        while (fgets(buffer, sizeof(buffer), stdin)) {
            strip_crlf(buffer);
            m = buffer;
            if (hi_test(m)) {
                debug(1, "dupe: %s", m);
            } else {
                if (t) {
                    debug(2, "new: %s (time=%ld)", m, t);
                    hi_write_t(t, 0, m);
                } else {
                    debug(2, "new: %s (time=now)", m);
                    hi_write(0, m);
                }
            }
        }

    }

    hi_close();

    return 0;
}
#endif /**TEST**/
/*****************************************************************************/
