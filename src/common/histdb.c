/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * MSGID history functions and dupe checking
 *
 *****************************************************************************
 * Copyright (C) 1990-2001
 *  _____ _____
 * |     |___  |   Martin Junius             FIDO:      2:2452/110
 * | | | |   | |   Radiumstr. 18             Internet:  mj@fido.de
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
#include "dbz.h"

/*
 * History file
 */
static FILE *hi_file = NULL;
short int hi_init(char *);

/*
 * Get base name
 */
#if defined(DBC_HISTORY) && defined(FIDO_STYLE_MSGID)
short int hi_init_dbc()
{
    char db_name[MAXPATH];

    BUF_EXPAND(db_name, cf_p_dbc_history());

    if (hi_init(db_name) == ERROR) {
        return ERROR;
    }
    return OK;
}
#endif                          /* DBC_HISTORY && FIDO_STYLE_MSGID */

#ifdef TIC_HISTORY
void hi_init_tic_history(void)
{
    char db_name[MAXPATH];

    BUF_EXPAND(db_name, cf_p_tic_history());

    if (hi_init(db_name) == ERROR) {
        return;
    }
    return;
}
#endif                          /* TIC_HISTORY */

void hi_init_history(void)
{
    char db_name[MAXPATH];

    BUF_EXPAND(db_name, cf_p_history());

    hi_init(db_name);
}

/*
 * Initialize MSGID history
 */
short int hi_init(char *his_file)
{
    FILE *fp;

    /* Test for history.dir, history.pag */
    BUF_EXPAND(buffer, his_file);
    BUF_APPEND(buffer, ".dir");
    if (check_access(buffer, CHECK_FILE) != TRUE) {
        /* Doesn't exist, create */
        if ((fp = fopen(buffer, W_MODE)) == NULL) {
            fglog("$ERROR: creating MSGID history %s failed", buffer);
            return ERROR;
        } else
            debug(9, "creating MSGID history %s", buffer);
    }
    BUF_EXPAND(buffer, his_file);
    BUF_APPEND(buffer, ".pag");
    if (check_access(buffer, CHECK_FILE) != TRUE) {
        /* Doesn't exist, create */
        if ((fp = fopen(buffer, W_MODE)) == NULL) {
            fglog("$ERROR: creating MSGID history %s failed", buffer);
            return ERROR;
        } else
            fglog("creating MSGID history %s", buffer);
    }
    /* Open the history text file */
    BUF_EXPAND(buffer, his_file);
    if ((hi_file = fopen(buffer, A_MODE)) == NULL) {
        fglog("$ERROR: open MSGID history %s failed", buffer);
        if (check_access(buffer, CHECK_FILE) == ERROR) {
            fglog("$ERROR: Premission denied %s", buffer);
            return ERROR;
        }
    }
    /* Open the DBZ file */
    dbzincore(1);
    /**dbzwritethrough(1);**/
    if (dbminit(buffer) == -1) {
        fglog("$ERROR: dbminit %s failed", buffer);
        return ERROR;
    }
    return OK;
}

/*
 * Close MSGID history
 */
void hi_close(void)
{

    if (hi_file) {
        if (fclose(hi_file) == ERROR)
            fglog("$ERROR: close MSGID history failed");
        if (dbzsync())
            fglog("$ERROR: dbzsync MSGID history failed");
        if (dbmclose() < 0)
            fglog("$ERROR: dbmclose MSGID history failed");

        hi_file = NULL;
    }
}

/*
 * Write record to DBC MSGID history
 */
#if defined(DBC_HISTORY) && defined(FIDO_STYLE_MSGID)
short int hi_write_dbc(char *rfc_msgid, char *fido_msgid, short int dont_flush)
{
    long offset;
    int ret;
    datum key, val;
    TIMEINFO ti;

    GetTimeInfo(&ti);
    if (hi_file) {
        /* Get offset in history text file */
        if ((offset = ftell(hi_file)) == ERROR) {
            fglog("$ERROR: ftell DBC MSGID history failed");
            return ERROR;
        }
    } else {
        fglog("$ERROR: can't open MSGID history file");
        return ERROR;
    }

    /* Write MSGID line to history text file */
    if (strchr(fido_msgid, ' '))
        fido_msgid = strrchr(fido_msgid, ' ') + 1;
    debug(7, "dbc history: offset=%ld: %s %s %ld", offset, fido_msgid,
          rfc_msgid, (long)ti.time);
    ret =
        fprintf(hi_file, "%s\t%s\t%ld\n", fido_msgid, rfc_msgid, (long)ti.time);
    if (ret == ERROR || (!dont_flush && fflush(hi_file) == ERROR)) {
        fglog("$ERROR: write to DBC MSGID history failed");
        return ERROR;
    }

    /* Write database record */
    key.dptr = fido_msgid;      /* Key */
    key.dsize = strlen(fido_msgid) + 1;
    val.dptr = (char *)&offset; /* Value */
    val.dsize = sizeof offset;
    if (dbzstore(key, val) < 0) {
        fglog("ERROR: dbzstore of record for DBC MSGID history failed");
        return ERROR;
    }

       /**FIXME: dbzsync() every n msgids */

    return OK;
}
#endif                          /* DBC_HISTORY && FIDO_STYLE_MSGID */

/*
 * Write record to MSGID history
 */
short int hi_write_t(time_t t, time_t msgdate, char *msgid)
{
    long offset;
    int ret;
    datum key, val;

    if (hi_file) {
        /* Get offset in history text file */
        if ((offset = ftell(hi_file)) == ERROR) {
            fglog("$ERROR: ftell MSGID history failed");
            return ERROR;
        }
    } else {
        fglog("$ERROR: can't open MSGID history file");
        return ERROR;
    }

    /* Write MSGID line to history text file */
    debug(7, "history: offset=%ld: %s %ld", offset, msgid, (long)t);
    ret = fprintf(hi_file, "%s\t%ld\n", msgid, (long)t);
    if (ret == ERROR || fflush(hi_file) == ERROR) {
        fglog("$ERROR: write to MSGID history failed");
        return ERROR;
    }

    /* Write database record */
    key.dptr = msgid;           /* Key */
    key.dsize = strlen(msgid) + 1;
    val.dptr = (char *)&offset; /* Value */
    val.dsize = sizeof offset;
    if (dbzstore(key, val) < 0) {
        fglog("ERROR: dbzstore of record for MSGID history failed");
        return ERROR;
    }

       /**FIXME: dbzsync() every n msgids */

    return OK;
}

short int hi_write(time_t msgdate, char *msgid)
    /* msgdate currently not used */
{
    TIMEINFO ti;

    GetTimeInfo(&ti);

    return hi_write_t(ti.time, msgdate, msgid);
}

/*
 * Write record to DB
 */
short int hi_write_avail(char *area, char *desc)
{
    long offset;
    int ret;
    datum key, val;

    if (hi_file) {
        /* Get offset in history text file */
        if ((offset = ftell(hi_file)) == ERROR) {
            fglog("$ERROR: ftell MSGID history failed");
            return ERROR;
        }
    } else {
        fglog("$ERROR: can't open MSGID history file");
        return ERROR;
    }

    /* Write MSGID line to history text file */
    debug(7, "history: offset=%ld: %s %s", offset, area, desc);
    ret = fprintf(hi_file, "%s\t%s\n", area, desc);
    if (ret == ERROR || fflush(hi_file) == ERROR) {
        fglog("$ERROR: write to MSGID history failed");
        return ERROR;
    }

    /* Write database record */
    key.dptr = area;            /* Key */
    key.dsize = strlen(area) + 1;
    val.dptr = (char *)&offset; /* Value */
    val.dsize = sizeof offset;
    if (dbzstore(key, val) < 0) {
        fglog("ERROR: dbzstore of record for MSGID history failed");
        return ERROR;
    }

       /**FIXME: dbzsync() every n msgids */

    return OK;
}

/*
 * Test if MSGID is already in database
 */
short int hi_test(char *key_string)
{
    datum key, val;

    key.dptr = key_string;      /* Key */
    key.dsize = strlen(key_string) + 1;
    val = dbzfetch(key);
#ifdef DEBUG
    debug(1, "hi_test() key = %s, returned %s", key_string, val.dptr);
#endif
    return val.dptr != NULL;
}

/*
 * Test if DB key is already in database
 */
char *hi_fetch(char *key_string, int flag)
{
    datum key, val;
    static char out[MAXPATH];

    if (flag == 0)
        key_string = strchr(key_string, ' ') + 1;
    debug(7, "search key %s", key_string);
    key.dptr = key_string;      /* Key */
    key.dsize = strlen(key_string) + 1;
    val = dbcfetch(key);
    if (val.dptr) {
        BUF_COPY(out, xstrtok(val.dptr, " \t"));
        debug(7, "found: %s", out);
        return out;
    } else {
        debug(7, "not found");
        return NULL;
    }
}
