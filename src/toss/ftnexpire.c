/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Expire MSGID history database
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
#include "dbz.h"

#define PROGRAM 	"ftnexpire"
#define CONFIG		DEFAULT_CONFIG_MAIN

/*
 * Prototypes
 */
int do_expire(int);
int do_line(FILE *, char *, int);

/*
 * Files
 */
static char history[MAXPATH];
static char history_dir[MAXPATH];
static char history_pag[MAXPATH];
static char history_n[MAXPATH];
static char history_n_dir[MAXPATH];
static char history_n_pag[MAXPATH];

static long history_line = 0;

/*
 * Options
 */
static double max_history = 14; /* Max. number of days entry stays
                                   in MSGID history database */

static time_t max_sec = 0;
static time_t now_sec = 0;
static time_t exp_sec = 0;

static long n_expired = 0;
static long n_processed = 0;

/*
 * Run expire
 */
int do_expire(int type)
{
    FILE *hi_o, *hi_n, *fp;
    TIMEINFO ti;

    /* Filenames */
    if (type == 1) {
        BUF_EXPAND(history, cf_p_dbc_history());
        BUF_EXPAND(history_dir, cf_p_dbc_history());
        BUF_EXPAND(history_pag, cf_p_dbc_history());
        BUF_EXPAND(history_n, cf_p_dbc_history());
        BUF_EXPAND(history_n_dir, cf_p_dbc_history());
        BUF_EXPAND(history_n_pag, cf_p_dbc_history());
    } else if (type == 2) {
        BUF_EXPAND(history, cf_p_tic_history());
        BUF_EXPAND(history_dir, cf_p_tic_history());
        BUF_EXPAND(history_pag, cf_p_tic_history());
        BUF_EXPAND(history_n, cf_p_tic_history());
        BUF_EXPAND(history_n_dir, cf_p_tic_history());
        BUF_EXPAND(history_n_pag, cf_p_tic_history());
    } else {
        BUF_EXPAND(history, cf_p_history());
        BUF_EXPAND(history_dir, cf_p_history());
        BUF_EXPAND(history_pag, cf_p_history());
        BUF_EXPAND(history_n, cf_p_history());
        BUF_EXPAND(history_n_dir, cf_p_history());
        BUF_EXPAND(history_n_pag, cf_p_history());
    }
    BUF_APPEND(history_dir, ".dir");
    BUF_APPEND(history_pag, ".pag");
    BUF_APPEND(history_n, ".n");
    BUF_APPEND(history_n_dir, ".n.dir");
    BUF_APPEND(history_n_pag, ".n.pag");

    debug(4, "old history: %s\n             %s\n             %s",
          history, history_dir, history_pag);
    debug(4, "new history: %s\n             %s\n             %s",
          history_n, history_n_dir, history_n_pag);

    /* Expire time (seconds) */
    GetTimeInfo(&ti);
    now_sec = ti.time;
    max_sec = 24L * 3600L * max_history;
    exp_sec = now_sec - max_sec;
    if (exp_sec < 0)
        exp_sec = 0;
    debug(4, "expire: now=%ld max=%ld, expire < %ld",
          (long)now_sec, (long)max_sec, (long)exp_sec);

    /* Open old history for reading */
    if ((hi_o = fopen(history, R_MODE)) == NULL) {
        fglog("$ERROR: open MSGID history %s failed", history);
        return ERROR;
    }

    /* Open new history for writing */
    if ((hi_n = fopen(history_n, W_MODE)) == NULL) {
        fglog("$ERROR: open MSGID history %s failed", history_n);
        fclose(hi_o);
        return ERROR;
    }

    /* Create empty new .dir and .pag */
    if ((fp = fopen(history_n_dir, W_MODE)) == NULL) {
        fglog("$ERROR: open MSGID history %s failed", history_n_dir);
        fclose(hi_o);
        fclose(hi_n);
        return ERROR;
    }
    fclose(fp);
    if ((fp = fopen(history_n_pag, W_MODE)) == NULL) {
        fglog("$ERROR: open MSGID history %s failed", history_n_pag);
        fclose(hi_o);
        fclose(hi_n);
        return ERROR;
    }
    fclose(fp);

    /* Initialize the new DBZ file */
    dbzincore(1);
    if (dbzagain(history_n, history) < 0) {
        fglog("$ERROR: dbzagain %s failed", history_n);
        fclose(hi_o);
        fclose(hi_n);
        return ERROR;
    }

    /* Process entries */
    while (fgets(buffer, sizeof(buffer), hi_o)) {
        do_line(hi_n, buffer, type);
    }

    /* Close everything */
    /**FIXME: error checking**/
    fclose(hi_o);
    fclose(hi_n);
    dbzsync();
    dbmclose();

    /* Rename */
    if (rename(history_n, history) == ERROR) {
        fglog("$ERROR: rename %s -> %s failed", history_n, history);
        return ERROR;
    }
    if (rename(history_n_dir, history_dir) == ERROR) {
        fglog("$ERROR: rename %s -> %s failed", history_n_dir, history_dir);
        return ERROR;
    }
    if (rename(history_n_pag, history_pag) == ERROR) {
        fglog("$ERROR: rename %s -> %s failed", history_n_pag, history_pag);
        return ERROR;
    }

    return OK;
}

/*
 * Process one line from history
 */
int do_line(FILE * hi_n, char *line, int type)
{
    char *msgid, *p, *rfc_msgid = NULL;
    time_t t;
    int expired;
    long offset;
    int ret;
    datum key, val;

    /* Parse old entry */
    strip_crlf(line);
    history_line++;

    if (type == 1) {
        msgid = strtok(line, "\t");
        rfc_msgid = strtok(NULL, "\t");
        p = strtok(NULL, "\t");

        if (!msgid || !p || !rfc_msgid) {
            fglog("$WARNING: illegal entry in %s, line %ld", history,
                  history_line);
            return ERROR;
        }
    }

    else {
        msgid = strtok(line, "\t");
        p = strtok(NULL, "\t");
        if (!msgid || !p) {
            fglog("$WARNING: illegal entry in %s, line %ld", history,
                  history_line);
            return ERROR;
        }
    }

    /* Check expire */
    t = atol(p);
    expired = t < exp_sec;
    debug(7, "msgid=%s  time=%ld  expired=%s",
          msgid, (long)t, expired ? "YES" : "NO");

    /* Write if not expired */
    if (!expired) {
        /* Get offset in history text file */
        if ((offset = ftell(hi_n)) == ERROR) {
            fglog("$ERROR: ftell MSGID history failed");
            return ERROR;
        }
        /* Write MSGID line to history text file */
        if (type == 1)
            ret = fprintf(hi_n, "%s\t%s\t%ld\n", msgid, rfc_msgid, (long)t);
        else
            ret = fprintf(hi_n, "%s\t%ld\n", msgid, (long)t);

        if (ret == ERROR || fflush(hi_n) == ERROR) {
            fglog("$ERROR: write to MSGID history failed");
            return ERROR;
        }

        /* Write database record */
        key.dptr = msgid;       /* Key */
        key.dsize = strlen(msgid) + 1;
        val.dptr = (char *)&offset; /* Value */
        val.dsize = sizeof offset;
        if (dbzstore(key, val) < 0) {
            fglog("ERROR: dbzstore of record for MSGID history failed");
            return ERROR;
        }
    } else
        n_expired++;

    n_processed++;

    return OK;
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options]\n\n", PROGRAM);
    fprintf(stderr, "\
options: -m --maxhistory DAYS         set max number of days in history\n\
         -w --wait [TIME]             wait for history lock to be released\n\
\n\
         -v --verbose                 more verbose\n\
	 -h --help                    this help\n\
         -c --config name             read config file (\"\" = none)\n\
	 -d --dbc                     set history file correspond db\n\
	 -t --tic                     set history file tick db\n");
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    int c, ret = EXIT_OK;
    char *p;
    char *m_flag = NULL;
    int w_flag = NOWAIT;
    char *c_flag = NULL;
    int type = 0;

    time_t expire_start, expire_delta;

    int option_index;
    static struct option long_options[] = {
        {"maxhistory", 1, 0, 'm'},  /* MaxHistory */
        {"wait", 1, 0, 'w'},    /* Wait for history DB lock */

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"dbc", 0, 0, 'd'},     /* Correspond db file */
        {"tic", 0, 0, 't'},     /* Tic db file */
        {0, 0, 0, 0}
    };

    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();

    while ((c = getopt_long(argc, argv, "m:wdtvhc:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftnexpire options *****/
        case 'm':
            m_flag = optarg;
            break;
        case 'w':
            if (optarg)
                w_flag = atoi(optarg);
            else
                w_flag = WAIT;
            break;

    /***** Common options *****/
        case 'v':
            verbose++;
            break;
        case 'h':
            usage();
            return 0;
            break;
        case 'c':
            c_flag = optarg;
            break;
        case 'd':
            type = 1;
            break;
        case 't':
            type = 2;
            break;
        default:
            short_usage();
            return EX_USAGE;
            break;
        }

    /*
     * Read config file
     */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    cf_debug();

    /*
     * Process optional config statements
     *
     * -m overrides config MaxHistory
     */
    if (type == 1
        && ((p = m_flag) || (p = cf_get_string("MaxHistoryDBC", TRUE)))) {
        max_history = atof(p);
        if (max_history < 0)
            max_history = 0;
        debug(8, "actual MaxHistoryDBC %g", max_history);
    } else if (type == 2
               && ((p = m_flag)
                   || (p = cf_get_string("MaxTicHistory", TRUE)))) {
        max_history = atof(p);
        if (max_history < 0)
            max_history = 0;
        debug(8, "actual MaxTicHistory %g", max_history);
    } else if ((p = m_flag) || (p = cf_get_string("MaxHistory", TRUE))) {
        max_history = atof(p);
        if (max_history < 0)
            max_history = 0;
        debug(8, "actual MaxHistory %g", max_history);
    }

    /*
     * Run expire, locking MSGID history database
     */
    if (lock_program(cf_p_lock_history(), w_flag ? w_flag : WAIT) == ERROR) {   /* Already busy */
        fglog("MSGID history database is busy");
        exit_free();
        return EXIT_BUSY;
    }

    /* Start time */
    expire_start = time(NULL);
    /* Expire */
    if (do_expire(type) == ERROR)
        ret = EXIT_ERROR;
    /* Stop time */
    expire_delta = time(NULL) - expire_start;
    if (expire_delta <= 0)
        expire_delta = 1;

    /* Statistics */
    fglog("ids processed: %ld total, %ld expired in %ld s, %.2f ids/s",
          n_processed, n_expired,
          (long)expire_delta, (double)n_processed / expire_delta);

    unlock_program(cf_p_lock_history());

    exit_free();
    return ret;
}
