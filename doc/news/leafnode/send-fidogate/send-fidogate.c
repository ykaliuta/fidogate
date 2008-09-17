/*
 * (C) Maint Laboratory 2003-2004
 * Author: Elohin Igor'
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
#include "../config.h"

#ifdef _BSD
#include <limits.h>
#endif

int verbose;
char *prgname;
void usage(void);
void help(void);
char *version = "send-fidogate"VERSION;

int main(int argc, char *argv[])
{
    struct dirent *dir;
    DIR *dfd;
    char fullpath[PATH_MAX];
    int i;
    char c;
    char cmd[PATH_MAX];
    char buf[512];
    FILE *fd;
    unsigned char flagFOUND;

    prgname = strrchr(argv[0], '/');
    if (prgname == NULL)
	prgname = argv[0];
    else
        prgname++;

    readcfg();
    loginit("send-fidogate", util_logdir);
    verbose = 0;
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
	myerror("cannot setgid to %d", getegid());
	exit(1);
    }
    if (setuid(geteuid()) < 0) {
	myerror("cannot setuid to %d", geteuid());
	exit(1);
    }
    if ((dfd = opendir(outgoing)) == NULL) {
	myerror("%s  \'%s\'\n", strerror(errno), outgoing);
	return 1;
    }
    i = 0;
    while ((dir = readdir(dfd)) != NULL) {
	if (strcmp(dir->d_name, ".") == 0 ||
	    strcmp(dir->d_name, "..") == 0)
	    continue;
	snprintf(fullpath, sizeof(fullpath), "%s/%s", outgoing, dir->d_name);
        if((fd = fopen(fullpath, "r")) == NULL){
            myerror("Can't open \'%s\' (%s)\n", fullpath, strerror(errno));
            continue;
        }
        flagFOUND = 0;
        while(fgets(buf, 512, fd)){
            if(strncmp(buf, "Newsgroups: ", 12) == 0){
                if(isfidogroup(&buf[12])){
                    flagFOUND = 1;
                    break;
                }
            }
        }
        if(!flagFOUND){
            myerror("Newsgroups not found\n");
            fclose(fd);
            continue;
        }
        fclose(fd);
        i++;
        if (verbose)
	    notice("send %s started", dir->d_name);
	snprintf(cmd, sizeof(cmd), "%s %s/%s | %s",
		delete_ctrl_d, outgoing, dir->d_name, rfc2ftn);
	if (system(cmd)) {
	    if (verbose)
		notice("send %s failed (%s)", dir->d_name, cmd);
	} else {
	    if (verbose)
		notice("send %s ok", dir->d_name);
	    if (remove(fullpath)) {
		myerror("%s  \'%s\'\n", strerror(errno), fullpath);
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
    printf("\t%s\n\t-------------------\n"
	   "usage: send-fidogate [-options]\n"
	   "options:\n"
	   "\t-h              this help\n"
	   "\t-v              verbose\n"
	   "\n"
	   "Copyright (C) 2003-2004  Igor' Elohin <maint@unona.ru>\n"
	   "-------------------------------------------------------------\n"
	   "Eagle's send-fidogate comes with ABSOLUTELY NO WARRANTY;\n"
	   "This is free software, and you are welcome to redistribute it\n"
	   "under certain conditions.\n",
	   version);
}

