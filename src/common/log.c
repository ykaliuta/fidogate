/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * Log and debug functions
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
 *  _____ _____
 * |     |___  |   Martin Junius             <mj@fidogate.org>
 * | | | |   | |   Radiumstr. 18
 * |_|_|_|@home|   D-51069 Koeln, Germany
 *
 * Copyright (C) 2015 Yauheni Kaliuta <y.kaliuta@gmail.com>
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

/* Debug level -v --verbose option */
int verbose = 0;

/* Syslog support */
#if defined(HAVE_SYSLOG) && defined(HAVE_SYSLOG_H)
#include <syslog.h>
#endif

#ifndef HAVE_STRERROR
/*
 * strerror()  ---  get string from sys_errlist[]
 */
char *strerror(int errnum)
{
#ifndef OS2
#ifndef __FreeBSD__
    extern int sys_nerr;
    extern char *sys_errlist[];
#endif
#endif

    if (errnum > 0 && errnum < sys_nerr)
        return sys_errlist[errnum];
    return "unknown error";
}
#endif /**HAVE_STRERROR**/

struct logger {
    int inited;
    int suppress_debug;
    int reopen_logfile;
    FILE *debugfile;
    FILE *logfile;
    char logfile_fullname[MAXPATH];
    char strid[MAXPATH];
    struct logger_ops *ops;
};

struct logger_ops {
    void (*debug)(struct logger *, const char *, va_list);
    void (*suppressed)(struct logger *, const char *, ...);
    void (*log)(struct logger *, const char *, va_list);
    void (*deinit)(struct logger *);
    void (*name_changed)(struct logger *);
};

static void file_debug(struct logger *, const char *, va_list);
static void file_suppressed(struct logger *, const char *, ...);
static void file_log(struct logger *, const char *, va_list);
static void file_deinit(struct logger *);

static void stream_log(struct logger *, const char *, va_list);

static void default_log(struct logger *, const char *, va_list);

#define DEFAULT_PROG "FIDOGATE"

static struct logger_ops file_ops = {
    .debug = file_debug,
    .suppressed = file_suppressed,
    .log = file_log,
    .deinit = file_deinit,
};

static struct logger_ops stream_ops = {
    .debug = file_debug,
    .suppressed = file_suppressed,
    .log = stream_log,
};

/* see syslog_ops below under syslog condition */

static struct logger_ops default_ops = {
    .debug = file_debug,
    .suppressed = file_suppressed,
    .log = default_log,
};

static struct logger the_logger = {
    .strid = DEFAULT_PROG,
    .ops = &default_ops,
};

static void log_finish_init(struct logger *l)
{
    /* copy from config.c:cf_initialize() */
    /*
     * Check for real uid != effective uid (setuid installed FIDOGATE
     * programs), disable debug() output (-v on command line) and
     * environment variables in this case.
     */
    if (getuid() != geteuid())
        l->suppress_debug = TRUE;
}

#if !defined(HAVE_SYSLOG) || !defined(HAVE_SYSLOG_H)
static int log_init_syslog(struct logger *l)
{
    return ERROR;
}
#else                           /* SYSLOG ENABLED */

static void syslog_debug(struct logger *l, const char *fmt, va_list args)
{
    vsyslog(LOG_DEBUG, fmt, args);
}

static void log_init_syslog(struct logger *l)
{
    l->logfile = NULL;
    l->debugfile = stderr;
    l->ops = &syslog_ops;
    BUF_COPY(l->logfile_fullname, "syslog");

    openlog(l->strid, LOG_PID, FACILITY);
}

/* SYSLOG variants*/

static void syslog_suppressed(struct logger *l, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsyslog(LOG_DEBUG, fmt, args);
    va_end(args);
}

static void syslog_log(struct logger *l, const char *fmt, va_list args)
{
    vsyslog(LOG_NOTICE, *fmt == '$' ? fmt + 1 : fmt, args);
    if (*fmt == '$')
        syslog(LOG_NOTICE, "(errno=%d: %m)", errno);
}

static void syslog_deinit(struct logger *l)
{
    closelog();
}

static void syslog_name_changed(struct logger *l)
{
    syslog_deinit(l);
    log_init_syslog(l);
}

static struct logger_ops syslog_ops = {
    .debug = syslog_debug,
    .suppressed = syslog_suppressed,
    .log = syslog_log,
    .deinit = syslog_deinit,
    .name_changed = syslog_name_changed,
};

#endif                          /* defined(HAVE_SYSLOG) || !defined(HAVE_SYSLOG_H) */

static void log_init_stream(struct logger *l, FILE * f)
{
    BUF_COPY(l->logfile_fullname, "-");
    l->logfile = f;
    l->debugfile = f;
    l->ops = &stream_ops;
}

static void log_init_file(struct logger *l, char *fname)
{
    BUF_EXPAND(l->logfile_fullname, fname);
    l->reopen_logfile = TRUE;
    l->logfile = NULL;
    l->debugfile = stderr;
    l->ops = &file_ops;
}

static void log_init_file_or_stream(struct logger *l, char *fname)
{
    FILE *f = NULL;

    if (streq(fname, "stdout"))
        f = stdout;
    if (streq(fname, "stderr"))
        f = stderr;

    if (f == NULL)
        log_init_file(l, fname);
    else
        log_init_stream(l, f);
}

static int log_init(struct logger *l, char *fname)
{
    if (l->inited) {
        fprintf(stderr, "ERROR: log subsystem already inited\n");
        return ERROR;
    }

    if (fname == NULL)
        fname = cf_p_logfile();

    if (streq(fname, "syslog"))
        log_init_syslog(l);
    else
        log_init_file_or_stream(l, fname);

    log_finish_init(l);
    l->inited = TRUE;
    return OK;
}

/* it will keep the "program" name to support old way */
static void log_deinit(struct logger *l)
{
    if (l->ops->deinit)
        l->ops->deinit(l);
    l->inited = FALSE;
}

static int open_logfile(struct logger *l)
{
    FILE *f;

    if (l->logfile) {
        fprintf(stderr, "ERROR: Logfile (%s) already openned\n",
                l->logfile_fullname);
        return ERROR;
    }

    f = fopen(l->logfile_fullname, A_MODE);
    if (f == NULL) {
        fprintf(stderr,
                "%s WARNING: can't open log file %s (errno=%d: %s)\n",
                l->strid, l->logfile_fullname, errno, strerror(errno));
        if (!verbose)
            verbose = -1;
        return ERROR;
    }
    l->logfile = f;
    return OK;
}

static void close_logfile(struct logger *l)
{
    if (l->logfile)
        fclose(l->logfile);
    l->logfile = NULL;
}

static void log_print(FILE * stream, char *name, int err,
                      const char *fmt, va_list args)
{
    char buf[32];

    fprintf(stream, "%s %s ",
            date_buf(buf, sizeof(buf), DATE_LOG, NULL, -1), name);
    vfprintf(stream, *fmt == '$' ? fmt + 1 : fmt, args);
    if (*fmt == '$')
        fprintf(stream, " (errno=%d: %s)", err, strerror(err));
    fprintf(stream, "\n");
}

static void file_log(struct logger *l, const char *fmt, va_list args)
{
    int r = OK;
    int save_errno;

    save_errno = errno;

    r = open_logfile(l);
    if (r != OK)
        return;

    log_print(l->logfile, l->strid, save_errno, fmt, args);

    close_logfile(l);
}

static void file_debug(struct logger *l, const char *fmt, va_list args)
{
    vfprintf(l->debugfile, fmt, args);
    fprintf(l->debugfile, "\n");
    fflush(l->debugfile);
}

static void file_suppressed(struct logger *l, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(l->debugfile, fmt, args);
    va_end(args);
}

static void file_deinit(struct logger *l)
{
    close_logfile(l);
}

/* STREAM variants */

static void stream_log(struct logger *l, const char *fmt, va_list args)
{
    log_print(l->logfile, l->strid, errno, fmt, args);
}

/* DEFAULT */

static void default_log(struct logger *l, const char *fmt, va_list args)
{
    if (!l->inited)
        log_init(l, NULL);
    l->ops->log(l, fmt, args);
}

/*
  Keep the old API
*/
void fglog(const char *fmt, ...)
{
    struct logger *l = &the_logger;
    va_list args;

    va_start(args, fmt);
    l->ops->log(l, fmt, args);
    va_end(args);
}

void debug(int lvl, const char *fmt, ...)
{
    struct logger *l = &the_logger;
    va_list args;

    if (verbose < lvl)
        return;

    if (!l->inited) {
        l->debugfile = stderr;
        log_finish_init(l);
    }

    va_start(args, fmt);

    if (l->suppress_debug) {
        l->ops->suppressed(l,
                           "debug called for uid=%d euid=%d, "
                           "output disabled\n", (int)getuid(), (int)geteuid());
        goto out;
    }

    l->ops->debug(l, fmt, args);
 out:
    va_end(args);
}

void log_file(char *name)
{
    struct logger *l = &the_logger;

    log_deinit(l);
    log_init(l, name);
    setenv("FIDOGATE_LOGFILE", name, 1);
}

void log_program(char *name)
{
    struct logger *l = &the_logger;

    if (name == NULL)           /* shouldn't happen */
        return;

    BUF_COPY(l->strid, name);
    if (l->ops->name_changed)
        l->ops->name_changed(l);
}
