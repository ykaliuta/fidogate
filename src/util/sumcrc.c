/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * sumcrc32 --- CRC32 checksum computation
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
#include "getopt.h"

#define PROGRAM		"sumcrc32"

int s1_flag = FALSE;            /* -1 */
int crc32_flag = FALSE;         /* -3 */
int crc16_flag = FALSE;         /* -6 */
int z_flag = FALSE;             /* -z */
int x_flag = FALSE;             /* -x */
int v_flag = FALSE;             /* -v */

#define READ_SIZE	1024

/*
 * Prototypes
 */
int do_sumcrc32(char *);

/*
 * Process one file
 */
int do_sumcrc32(char *name)
{
    FILE *fp;
    int c;

    fp = fopen(name, R_MODE);
    if (fp == NULL) {
        fprintf(stderr, "%s: can't open %s", PROGRAM, name);
        perror("");
        return ERROR;
    }

    /*
     * -1 option: skip first line
     */
    if (s1_flag) {
        if (fgets(buffer, sizeof(buffer), fp) == NULL) {
            fprintf(stderr, "%s: can't read from %s\n", PROGRAM, name);
            return ERROR;
        }
    }

    crc32_init();
    crc16_init();
    while (TRUE) {
        c = getc(fp);
        if (c == EOF || (z_flag && c == 26))    /* EOF or ^Z */
            break;
        if (crc32_flag)
            crc32_update(c);
        else if (crc16_flag)
            crc16_update(c);
        else
            crc16_update_ccitt(c);
    }

    if (crc32_flag)
        printf(x_flag ? "%08lx" : "%010lu", crc32_value());
    else
        printf(x_flag ? "%04x" : "%05u",
               crc16_flag ? crc16_value() : crc16_value_ccitt());
    if (v_flag)
        printf(" %s", name);
    printf("\n");

    return OK;
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options] file ...\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options] file ...\n\n", PROGRAM);
    fprintf(stderr, "\
options:  -1 --skip-first-line         skip first line in text file\n\
          -3 --crc32                   use 32 bit CRC checksum\n\
          -6 --crc16                   use alternate CRC16 polynomial\n\
          -x --hex                     output CRC value in hex\n\
          -z --eof-at-ctrl-z           ^Z is EOF (MSDOS kludge)\n\
\n\
          -v --verbose                 verbose\n\
	  -h --help                    this help\n");
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c;
    int ret;

    int option_index;
    static struct option long_options[] = {
        {"skip-first-line", 0, 0, '1'},
        {"eof-at-ctrl-z", 0, 0, 'z'},
        {"hex", 0, 0, 'x'},
        {"crc32", 0, 0, '3'},
        {"crc16", 0, 0, '6'},

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "1zxvh36",
                            long_options, &option_index)) != EOF)
        switch (c) {
        case '1':
            s1_flag = TRUE;
            break;
        case 'z':
            z_flag = TRUE;
            break;
        case 'x':
            x_flag = TRUE;
            break;
        case '3':
            crc32_flag = TRUE;
            break;
        case '6':
            crc16_flag = TRUE;
            break;

    /***** Common options *****/
        case 'v':
            v_flag = TRUE;
            break;
        case 'h':
            usage();
            return 0;
            break;
        default:
            short_usage();
            return EX_USAGE;
            break;
        }

    if (optind == argc) {
        short_usage();
        return EX_USAGE;
    }

    /* Files */
    ret = 0;
    for (; optind < argc; optind++)
        if (do_sumcrc32(argv[optind]) == ERROR)
            ret = 1;

    return ret;
}
