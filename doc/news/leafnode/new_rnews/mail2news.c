/*
 * (C) Maint Laboratory 2003
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#include <stdio.h>
#include <string.h>
#include "../common.h"

int mail2news(int argc, char *fname, char *newsgroups, char *subject)
{
    char buf[BUFSIZE];
    char msgid[128];
    char *s;
    FILE *in;
    int rc;

    if (fname == NULL) {
	in = stdin;
    } else {
	if ((in = fopen(fname, "r")) == NULL) {
	    myerror("cant open input file %s", fname);
	    fprintf(stderr, "cant open input file %s\n", fname);
	    return 1;
	}
    }
    if (!isgrouponserver(newsgroups)) {
	notice("Group not found: %s", newsgroups);
	fprintf(stderr, "Group not found: %s\n", newsgroups);
    }
    while (fgets(buf, BUFSIZE - 1, in)) {
	if ((s = strchr(buf, '\n')) != NULL)
	    *s = '\0';
	if (strncmp(buf, "Message-ID: ", 12) == 0) {
	    strncpy(msgid, &buf[12], 128);
	    if (ismsgidonserver(msgid)) {
		myerror("Message-ID of %s already in use upstream", msgid);
		fprintf(stderr, "Message-ID of %s already in use upstream\n",
			msgid);
		fclose(in);
		return 1;
	    }
	    break;
	}
    }
    rc = post_MAIL(in, newsgroups, subject);
    fclose(in);
    if (rc == FALSE)
	fprintf(stderr, "Failed posting\n");
    erase_outgoing_msgid(msgid);
    return !rc;
}
