/*
 * (C) Maint Laboratory 2003
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "../common.h"

#ifdef _BSD
#define <sys/syslimits.h>
#endif

int verbose;
char *prgname;
void usage(void);
void help(void);
char *version = "send-fidogate 0.1";
char *outgoing;
char *rfc2ftn;

int main(int argc, char *argv[])
{
    struct dirent *dir;
    DIR *dfd;
    char fullpath[PATH_MAX];
    int i;
    char c;
    char cmd[PATH_MAX];

    prgname = strrchr(argv[0], '/');
    if (prgname == NULL)
	prgname = argv[0];
    else
	prgname++;

    loginit("send-fidogate", DIRLOG);
    verbose = 0;
    outgoing = OUTGOING;
    rfc2ftn = RFC2FTN;
    if (argc > 1) {
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
	    c = argv[i][1];
	    switch (c) {
	    case 'h':
		help();
		exit(0);
	    case 'v':
		verbose = 1;
		break;
	    default:
		usage();
	    }
	}
    }
    if (setgid(getegid()) < 0) {
	error("cannot setgid to %d", getegid());
	exit(1);
    }
    if (setuid(geteuid()) < 0) {
	error("cannot setuid to %d", geteuid());
	exit(1);
    }
    if ((dfd = opendir(outgoing)) == NULL) {
	error("%s  \'%s\'\n", sys_errlist[errno], outgoing);
	return 1;
    }
    i = 0;
    while ((dir = readdir(dfd)) != NULL) {
	if (strcmp(dir->d_name, ".") == 0 ||
	    strcmp(dir->d_name, "..") == 0)
	    continue;
	i++;
	if (verbose)
	    notice("send %s started", dir->d_name);
	snprintf(fullpath, sizeof(fullpath), "%s/%s", outgoing, dir->d_name);
	snprintf(cmd, sizeof(cmd), "%s %s/%s | %s",
		DELETE_CTRL_D, outgoing, dir->d_name, rfc2ftn);
	if (system(cmd)) {
	    if (verbose)
		notice("send %s failed (%s)", dir->d_name, cmd);
	} else {
	    if (verbose)
		notice("send %s ok", dir->d_name);
	    if (remove(fullpath)) {
		error("%s  \'%s\'\n", sys_errlist[errno], fullpath);
	    }
	}
    }
    if (verbose)
	notice("Sending %d files", i);
    closedir(dfd);
    fflush(stderr);
    logclose();
    exit(0);
}
void usage(void)
{
    printf("usage: %s [-h][-v]", prgname);
}
void help(void)
{
    printf("\t%s\n\t-----------------\n"
	   "usage: send-fidogate [-options]\n"
	   "options:\n"
	   "\t-h              this help\n"
	   "\t-v              verbose\n"
	   "\n"
	   "Copyright (C) 2003  Igor Elohin <maint@unona.ru>\n"
	   "-------------------------------------------------------------\n"
	   "Eagle's send-fidogate comes with ABSOLUTELY NO WARRANTY;\n"
	   "This is free software, and you are welcome to redistribute it\n"
	   "under certain conditions.\n",
	   version);
}

