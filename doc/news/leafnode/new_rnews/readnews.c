/*
 * (C) Maint Laboratory 2003
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <dirent.h>
#include "../common.h"

extern unsigned char flagREMOVE;
extern int verbose;

int readnews(int argc, char *fname)
{
    char buf[BUFSIZE];
    char newsgroups[128];
    char msgid[128];
    char *b;
    unsigned char flagONE;
    FILE *in;
    fpos_t pos, bpos;
    int rc;
    int k, number;

    if (fname == NULL) {
	in = stdin;
    } else {
	if ((in = fopen(fname, "r")) == NULL) {
	    myerror("cant open input file %s", fname);
	    return -1;
	}
    }
    flagONE = 0;
    rc = 0;
    fgetpos(in, &pos);
    number = 0;
    while (fgets(buf, BUFSIZE, in) != NULL) {
	if ((b = strchr(buf, '\n')))
	    *b = '\0';
	if (strncmp(buf, "#! rnews", 8) == 0) {
	    newsgroups[0] = '\0';
	    msgid[0] = '\0';
	    flagONE = 1;
	    bpos = pos;
            number++;
	} else if (flagONE) {
	    if (strncmp(buf, "Newsgroups: ", 12) == 0) {
		strncpy(newsgroups, &buf[12], 128);
		b = strrchr(newsgroups, ' ');
		if (b != NULL)
		    *b = '\0';
		if (!isgrouponserver(newsgroups)) {
		    tofailed(in, bpos, FAILED_POSTING);
		    myerror("Group not found: %s", newsgroups);
		    newsgroups[0] = '\0';
		    msgid[0] = '\0';
		    flagONE = 0;
		    rc = 1;
		}
	    } else if (strncmp(buf, "Message-ID: ", 12) == 0) {
		strncpy(msgid, &buf[12], 128);
		if (ismsgidonserver(msgid)) {
		    tofailed(in, bpos, DUPE_POST);
		    myerror("Message-ID of %s already in use upstream",
			  msgid);
		    newsgroups[0] = '\0';
		    msgid[0] = '\0';
		    flagONE = 0;
		    rc = 1;
		}
	    }
	    if (newsgroups[0] != '\0' && msgid[0] != '\0') {
		if (verbose)
		    notice("Posting...");
		if (post_FILE(in, &bpos) == FALSE) {
		    tofailed(in, bpos, IN_COMING);
		    myerror("Failed posting - posting to in.coming");
		    rc = 1;
		} else {
		    if (verbose)
			notice("Posting successfuly");
		}
		erase_outgoing_msgid(msgid);
		newsgroups[0] = '\0';
		msgid[0] = '\0';
		flagONE = 0;
	    }
	}
	fgetpos(in, &pos);
    }
    fclose(in);
    if(verbose) notice("total %d", number);
#ifdef REMOVE_SUCCESS
    if (in != stdin) {
	if (remove(fname) < 0)
	    myerror("%s %s", fname, strerrro(errno));
    }
#endif
    return rc;
}

int check_failed()
{
    struct dirent *dir;
    DIR *dfd;
    char fullpath[PATH_MAX];
    int i, n, rc;

    if ((dfd = opendir(FAILED_POSTING)) == NULL) {
	myerror("(check_failed.c): %s  \'%s\'\n", strerror(errno),
	      FAILED_POSTING);
	return -1;
    }
    i = 0;
    n = 0;
    while ((dir = readdir(dfd)) != NULL) {
	if (strcmp(dir->d_name, ".") == 0 ||
	    strcmp(dir->d_name, "..") == 0)
	    continue;
	n++;
	snprintf(fullpath, sizeof(fullpath), "%s/%s", FAILED_POSTING, dir->d_name);
	rc = readnews(0, fullpath);
	switch (rc) {
	case 0:
	    i++;
	    remove(fullpath);
	    break;
	case 1:
	    remove(fullpath);
	    break;
	}
    }
    if (verbose)
	notice("Sending %d files from failed.posting (total %d)", i, n);
    closedir(dfd);
    return 0;
}

int tofailed(FILE *f, fpos_t bpos, char *fname)
{
    char ofn[PATH_MAX];
    int o;
    FILE *out;
    fpos_t pos;
    char buf[BUFSIZE];
    struct passwd *pw;
    unsigned char flagONE;

    snprintf(ofn, sizeof(ofn), "%s/XXXXXXXX", fname);
    if ((o = mkstemp(ofn)) < 0) {
	myerror("Cannot create failed posting file %s(%s). Skip", fname,
	      strerror(errno));
	return -1;
    }
    if ((out = fdopen(o, "w")) == NULL) {
	myerror("Cannot create failed posting file %s(%s). Skip", fname,
	      strerror(errno));
	return -1;
    }
    fsetpos(f, &bpos);
    pos = bpos;
    flagONE = 0;
    while (fgets(buf, BUFSIZE, f) != NULL) {
	if (flagONE == 1 && strncmp(buf, "#! rnews", 8) == 0) {
	    fsetpos(f, &pos);
	    break;
	}
	fputs(buf, out);
	fgetpos(f, &pos);
	flagONE = 1;
    }
    fclose(out);
    pw = getpwnam("news");
    chmod(ofn, 0666);
    chown(ofn, pw->pw_uid, pw->pw_gid);
    return 0;
}
