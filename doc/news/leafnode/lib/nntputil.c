/*
 * Все общение с leafnode выдрано из leafnode 2.0 beta 8 и
 * поправлено для своих нужд.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "../common.h"

#ifdef _BSD
#include <sys/syslimits.h> 
#endif 

extern FILE *inp_nntp;
extern FILE *out_nntp;
extern int verbose;
char *get_line(char *);

#define SKIPLWS(p) while (*(p) && isspace((unsigned char) *(p))) { (p)++; }

/*
 *  This file wraps strtoul and strtol to simplify error checking.
 *  get_long
 *  year 2000
 *  author Matthias Andree
 */

/*  Reads a signed long integer from \p in and stores it into the long
 *  that \p var points to. 
 *  \return 
 *  1 for success, 
 *  0 if problem 
 */
int get_long(const char *in, long *var)
{
    char *e;

    *var = strtol(in, &e, 10);
    if (e == in)
	return 0;
    if ((errno == ERANGE)
	&& ((*var == LONG_MIN) || (*var == LONG_MAX)))
	return 0;
    return 1;
}
static void _ignore_answer(FILE * f)
{
    char *l;
    char buf[BUFSIZE];

    while (((l = get_line(buf)))) {
	if (!strcmp(l, "."))
	    break;
    }
}
/*
 * check whether any of the newsgroups is on server
 * return TRUE if yes, FALSE otherwise
 */
int isgrouponserver(char *newsgroups)
{
    char *p, *q;
    int retval;

    if (!newsgroups)
	return FALSE;

    retval = FALSE;
    p = newsgroups;
    do {
	SKIPLWS(p);
	q = strchr(p, ',');
	if (q)
	    *q++ = '\0';
	putaline("GROUP %s", p);
	if (nntpreply() == 211)
	    retval = TRUE;
	p = q;
    } while (q && !retval);
    return retval;
}
/*
 * check whether message-id is on server
 * return TRUE if yes, FALSE otherwise
 *
 * Since the STAT implementation is buggy in some news servers
 * (NewsCache), we use HEAD instead, although this causes more traffic.
 */
int ismsgidonserver(char *msgid)
{
    char *l;
    long a;
    char buf[BUFSIZE];

    if (!msgid)
	return FALSE;

    putaline("STAT %s", msgid);
    l = get_line(buf);
    if (get_long(l, &a) == 1) {
	switch (a) {
	case 223:
	    return TRUE;
	case 430:
	    return FALSE;
	default:		/* the server is undecisive,
				   fall back to HEAD */
	    /* FIXME: should log protocol error and pass error
	       to upper layers */
	    break;
	}
    }

    putaline("HEAD %s", msgid);
    l = get_line(buf);
    if (get_long(l, &a) == 1 && a == 221) {
	_ignore_answer(inp_nntp);
	return TRUE;
    } else {
	return FALSE;
    }
}
int newnntpreply( /*@out@ */ char **resline
	     /** If non-NULL, stores pointer to line here. */ )
{
    char *response;
    int r = 0;
    int c;
    char buf[BUFSIZE];

    do {
	response = get_line(buf);
	if (!response) {
	    error("NNTP server went away while waiting for response");
	    return -1;
	}
	if (strlen(response) >= 3 && isdigit((unsigned char) response[0])
	    && isdigit((unsigned char) response[1])
	    && isdigit((unsigned char) response[2])
	    && ((response[3] == ' ')
		|| (response[3] == '\0')
		|| (response[3] == '-'))) {
	    long rli;
	    int rl;

	    if (0 == get_long(response, &rli) ||
		rli > INT_MAX || rli < INT_MIN)
		r = -1;
	    rl = (int) rli;
	    if (r > 0 && r != rl)
		r = -1;		/* protocol error */
	    else
		r = rl;
	    c = (response[3] == '-');
	} else {
	    c = 0;
	    r = -1;		/* protocol error */
	}
    } while (c);

    if (resline)
	*resline = response;

    return r;
}
/*
 * Reads a line from the server, parses the status code, returns it and
 * discards the rest of the status line.
 */
int nntpreply(void)
{
    return newnntpreply(0);
}
void putaline(const char *fmt, ...)
{
    char lineout[BUFSIZE * 2];
    va_list args;

    va_start(args, fmt);
    vsnprintf(lineout, sizeof(lineout), fmt, args);
    fprintf(out_nntp, "%s\r\n", lineout);
    fflush(out_nntp);
    va_end(args);
}
/* post article in open file f, return FALSE if problem, return TRUE if ok */
/* FIXME: add IHAVE */
int post_FILE(FILE *f, fpos_t *pos)
{
    int r;
    static char line[BUFSIZE];
    char *s;
    fpos_t s_pos;
    unsigned char flagNEXT, flagONE;

    fsetpos(f, pos);
    putaline("POST");
    r = nntpreply();
    if (r != 340) {
	error("nntpreply rc = %d", r);
	return FALSE;
    }
    flagNEXT = 0;
    flagONE = 0;
    s_pos = *pos;
    while (fgets(line, BUFSIZE - 1, f) != NULL) {
        line[BUFSIZE - 1] = '\0';
	if((s = strstr(line, "\r\n")) != NULL)
	    *s = '\0';
	if((s = strrchr(line, '\n')) != NULL)
	    *s = '\0';
	if (strncmp(line, "#! rnews", 8) == 0) {
	    if(flagONE){
		flagNEXT = 1;
#ifdef DEBUG
		putaline("выход по #! rnews");
#endif
		break;
	    }
	    continue;
	}
	putaline("%s", line);
	flagONE = 1;
	fgetpos(f, &s_pos);
    }
    putaline(" + ");
    putaline(".");
    fflush(out_nntp);
    s = get_line(line);
    if (s == NULL){
#ifdef DEBUG
	notice("По чтению NULL с сервера");
#endif
	return FALSE;
    }
    if (strncmp(line, "240", 3) == 0) {
	if (verbose)
	    notice("Ok rc = %s", line);
	*pos = s_pos;
	fsetpos(f, &s_pos);
	return TRUE;
    }
    notice("%s", line);
    return FALSE;
}
/* post article in open file f, return FALSE if problem, return TRUE if ok */
/* FIXME: add IHAVE */
int post_MAIL(FILE *f, char *newsgroups, char *subject)
{
    int r, l;
    static char line[BUFSIZE];
    char *s;

    rewind(f);
    putaline("POST");
    r = nntpreply();
    if (r != 340) {
	error("nntpreply rc = %d", r);
	return FALSE;
    }
    while (fgets(line, BUFSIZE - 1, f) != NULL) {
	if((s = strstr(line, "\r\n")) != NULL)
	    *s = '\0';
	if((s = strrchr(line, '\n')) != NULL)
	    *s = '\0';
	if (strncmp(line, "From ", 5) == 0) {
            bzero(line, BUFSIZE);
	    sprintf(line, "Newsgroups: %s", newsgroups);
	} else if (subject[0] != '\0' && strncmp(line, "Subject: ", 9) == 0) {
	    sprintf(line, "Subject: %s", subject);
            l = strlen(line);
	    memset(&line[l], ' ', BUFSIZE - l);
            line[BUFSIZE - 1] = '\0';
	}
        putaline("%s", line);
    }
    putaline(".");
    fflush(out_nntp);
    s = get_line(line);
    if (s == NULL){
	return FALSE;
    }
    if (strncmp(line, "240", 3) == 0) {
	if (verbose)
	    notice("Ok rc = %s", line);
	return TRUE;
    }
    notice("%s", line);
    return FALSE;
}
