/*
 * (C) Maint Laboratory 2003
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include "../common.h"

extern int verbose;
extern char *prgname;
char logname[PATH_MAX];
static FILE *logfd;

void loginit(char *service, char *dir)
{
    logname[0] = '\0';
    if (dir != NULL) {
	sprintf(logname, "%s/%s", dir, service);
	if ((logfd = fopen(logname, "w+")) == NULL) {
	    fprintf(stderr, "Warning: Cannot open %s (%s). Use syslog\n",
		    logname, sys_errlist[errno]);
	    logname[0] = '\0';
	    service = "news";
	} else
	    return;
    }
    logname[0] = '\0';
    openlog(service, LOG_CONS | LOG_NDELAY, LOG_NEWS);
}

void logclose()
{
    if (logname[0] != '\0') {
        fflush(logfd);
	fclose(logfd);
	logname[0] = '\0';
    } else {
	closelog();
    }
}
void error(const char *fmt, ...)
{
    va_list ap;
    char out[MSGBUF];

    va_start(ap, fmt);
    vsnprintf(out, MSGBUF, fmt, ap);
    va_end(ap);

    if (verbose == 1) {
	fprintf(stderr, "%s\n", out);
	fflush(stderr);
    }
    if (logname[0] != '\0'){
	fprintf(logfd, "%s %s: %s\n", diagtime(), prgname, out);
        fflush(logfd);
    } else
	syslog(LOG_ERR, "%s: %s", prgname, out);
}
void message(const char *fmt, ...)
{
    va_list ap;
    char out[MSGBUF];

    va_start(ap, fmt);
    vsnprintf(out, MSGBUF, fmt, ap);
    va_end(ap);

    if (verbose == 1) {
	fprintf(stderr, "%s\n", out);
	fflush(stderr);
    }
    if (logname[0] != '\0'){
	fprintf(logfd, "%s %s: %s\n", diagtime(), prgname, out);
        fflush(logfd);
    } else
	syslog(LOG_INFO, "%s: %s", prgname, out);
}
void notice(const char *fmt, ...)
{
    va_list ap;
    char out[MSGBUF];

    va_start(ap, fmt);
    vsnprintf(out, MSGBUF, fmt, ap);
    va_end(ap);

    if (verbose == 1) {
	fprintf(stderr, "%s\n", out);
	fflush(stderr);
    }
    if (logname[0] != '\0'){
	fprintf(logfd, "%s %s: %s\n", diagtime(), prgname, out);
        fflush(logfd);
    } else
	syslog(LOG_NOTICE, "%s: %s", prgname, out);
}
void dolog(int flag, const char *fmt, ...)
{
    va_list ap;
    char out[MSGBUF];

    va_start(ap, fmt);
    vsnprintf(out, MSGBUF, fmt, ap);
    va_end(ap);

    if (verbose == 1) {
	fprintf(stderr, "%s\n", out);
	fflush(stderr);
    }
    if (logname[0] != '\0'){
	fprintf(logfd, "%s %s: %s\n", diagtime(), prgname, out);
        fflush(logfd);
    } else
	syslog(flag, "%s: %s", prgname, out);
}
