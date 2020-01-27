/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Functions for handling BinkleyTerm-style FLO files
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
 * Static vars holding data for flo_xxxx() functions
 */
static char flo_name[MAXPATH];  /* File name */
static FILE *flo_fp = NULL;     /* File stream */
static long flo_off_cur = -1;   /* Offset beginning of current line
                                 * (last one read by flo_gets()) */
static long flo_off_next = -1;  /* Offset beginning of next line */

/*
 * Return FLO file pointer
 */
FILE *flo_file(void)
{
    return flo_fp;
}

/*
 * Open FLO file for read/write, complete with BSY files and locking
 */
int flo_open(Node * node, int bsy)
{
    return flo_openx(node, bsy, NULL, FALSE);
}

int flo_openx(Node * node, int bsy, char *flav, int apmode)
{
    char *flo;
    char *mode;

    mode = apmode ? AP_MODE : RP_MODE;

    /* Find existing or new FLO file */
    flo = bink_find_flo(node, flav);
    if (!flo)
        return ERROR;
    BUF_COPY(flo_name, flo);

    /* Create directory if necessary */
    if (bink_mkdir(node) == ERROR)
        return ERROR;

    /* Create BSY file */
    if (bsy)
        if (bink_bsy_create(node, WAIT) == ERROR)
            return ERROR;

 again:
    /* Open FLO file for read/write */
    debug(4, "Opening FLO file, mode=%s", mode);
    flo_fp = fopen(flo_name, mode);
    if (flo_fp == NULL) {
        fglog("$opening FLO file %s mode %s failed", flo_name, mode);
        if (bsy)
            bink_bsy_delete(node);
        return ERROR;
    }
    chmod(flo_name, FLO_MODE);

    /* Lock it, waiting for lock to be granted */
    debug(4, "Locking FLO file");
    if (lock_file(flo_fp)) {
        /* Lock error ... */
        fglog("$locking FLO file %s failed", flo_name);
        if (bsy)
            bink_bsy_delete(node);
        fclose(flo_fp);
        return ERROR;
    }

    /* Lock succeeded, but the FLO file may have been deleted */
    if (access(flo_name, F_OK) == ERROR) {
        debug(4, "FLO file %s deleted after locking", flo_name);
        fclose(flo_fp);
        if (apmode) {
            if (bsy)
                bink_bsy_delete(node);
            goto again;
        } else
            return ERROR;
    }

    debug(4, "FLO file %s open and locking succeeded", flo_name);

    flo_off_cur = -1;           /* Nothing read yet */
    flo_off_next = 0;           /* Next line will be the 1st one */

    return OK;
}

/*
 * Read entry line from FLO file
 */
char *flo_gets(char *s, size_t len)
{
    long off;
    char *ret;

    /* Current offset */
    if ((off = ftell(flo_fp)) == -1) {
        fglog("$ftell FLO file %s failed", flo_name);
        return NULL;
    }
    flo_off_cur = off;

    /* Read next line */
    if (!(ret = fgets(s, len, flo_fp))) {
        /* EOF or error */
        if (ferror(flo_fp)) {
            fglog("$reading FLO file %s failed", flo_name);
            return NULL;
        }
    }

    /* New offset == offset of next entry line */
    if ((off = ftell(flo_fp)) == -1) {
        fglog("$ftell FLO file %s failed", flo_name);
        return NULL;
    }
    flo_off_next = off;

    /* Remove CR/LF */
    strip_crlf(s);

    return ret;
}

/*
 * Close FLO file
 */
int flo_close(Node * node, int bsy, int del)
{
    int ret = OK;

    if (flo_fp) {
        if (del)
            if (unlink(flo_name) == -1) {
                fglog("$removing FLO file %s failed", flo_name);
                ret = ERROR;
            }
        fclose(flo_fp);
        flo_fp = NULL;
    }

    if (bsy)
        bink_bsy_delete(node);

    return ret;
}

/*
 * Mark entry line in FLO file as sent
 */
void flo_mark(void)
{
    char tilde = '~';

    if (flo_fp == NULL || flo_off_cur == -1 || flo_off_next == -1)
        return;

    /* Seek to beginning of line last read */
    if (fseek(flo_fp, flo_off_cur, SEEK_SET) == -1) {
        fglog("$seeking to offset %ld in FLO file %s failed",
              flo_off_cur, flo_name);
        return;
    }

    /* Write ~ char */
    if (fwrite(&tilde, 1, 1, flo_fp) != 1) {
        fglog("$writing ~ to FLO file %s failed", flo_name);
        return;
    }

    /* Seek to beginning of next line */
    if (fseek(flo_fp, flo_off_next, SEEK_SET) == -1) {
        fglog("$seeking to offset %ld in FLO file %s failed",
              flo_off_next, flo_name);
        return;
    }

    return;
}

/*****************************************************************************/
#ifdef TEST

#include "getopt.h"

#define CONFIG		DEFAULT_CONFIG_MAIN

int main(int argc, char *argv[])
{
    Node node;
    int c;
    char *c_flag = NULL;
    char *S_flag = NULL, *L_flag = NULL;
    char mode[] = "-";
    char *line;

    int option_index;
    static struct option long_options[] = {
        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"config", 1, 0, 'c'},  /* Config file */
        {"spool-dir", 1, 0, 'S'},   /* Set FIDOGATE spool directory */
        {"lib-dir", 1, 0, 'L'}, /* Set FIDOGATE lib directory */
        {0, 0, 0, 0}
    };

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "vc:S:L:",
                            long_options, &option_index)) != EOF)
        switch (c) {
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
        }

    /*
     * Read config file
     */
    if (L_flag)                 /* Must set libexecdir beforehand */
        cf_s_libexecdir(L_flag);
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /*
     * Process config options
     */
    if (L_flag)
        cf_s_libexecdir(L_flag);
    if (S_flag)
        cf_s_spooldir(S_flag);

    /***** Test **************************************************************/
    if (optind >= argc) {
        fprintf(stderr,
                "usage: testflo [-v] [-c CONFIG] "
                "[-L LIBEXECDIR] [-S SPOOLDIR] Z:N/F.P\n");
        exit(1);
    }

    if (asc_to_node(argv[optind], &node, FALSE) == ERROR) {
        fprintf(stderr, "testflo: %s is not an FTN address\n", argv[optind]);
        exit(1);
    }

    if (flo_open(&node, TRUE) == ERROR) {
        printf("No FLO file for %s\n", znfp1(&node));
        exit(0);
    }

    printf("Contents of FLO file:\n");
    str_printf(buffer, sizeof(buffer), "cat -v %s", flo_name);
    system(buffer);

    while ((line = flo_gets(buffer, sizeof(buffer)))) {
        mode[0] = '-';
        printf("Line  :   %s\n", line);
        if (*line == '^' || *line == '#')
            mode[0] = *line++;
        printf("  DOS : %s %s\n", mode, line);
        if (cf_dos())
            printf("  UNIX: %s %s\n", mode, cf_unix_xlate(line));
        flo_mark();
    }

    printf("Modified contents of FLO file:\n");
    str_printf(buffer, sizeof(buffer), "cat -v %s", flo_name);
    system(buffer);

    flo_close(&node, TRUE, TRUE);

    exit(0);

    /**NOT REACHED**/
    return 0;
}

#endif /**TEST**/
