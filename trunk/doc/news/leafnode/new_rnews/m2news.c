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
#include <pwd.h>
#include "../common.h"
#include "../config.h"

char *prgname;
int verbose;
void usage(void);
void help(void);
char *version = "m2news"VERSION;

int main(int argc, char *argv[])
{
    int rc, i;
    FILE *in;
    char *s;
    char c;
    char subject[BUFSIZE];
    char newsgroups[BUFSIZE];

    verbose = 0;
    prgname = strrchr(argv[0], '/');
    if (prgname == NULL)
	prgname = argv[0];
    else
	prgname++;
    subject[0] = 0;
    newsgroups[0] = 0;

    if (setuid(geteuid()) < 0) {
	fprintf(stderr, "cannot setuid to %d", geteuid());
	myerror("cannot setuid to %d", geteuid());
	exit(1);
    }

    if (setgid(getegid()) < 0) {
	fprintf(stderr, "cannot setgid to %d", getegid());
	myerror("cannot setgid to %d", getegid());
	exit(1);
    }
    readcfg();
    loginit("m2news", util_logdir);
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
	    case 's':
		i++;
		if (i < argc)
		    strcpy(subject, argv[i]);
		break;
	    case 'd':
		strcpy(subject, diagtime());
		break;
	    case 'n':
		i++;
		if (i < argc)
		    strcpy(newsgroups, argv[i]);
		break;
	    case 'p':
		i++;
                if (i < argc){
                    if(isnum(argv[i])){
                        port_newsserv = atoi(argv[i]);
                    } else {
                        myerror("Port failed: %s", argv[i]);
                        exit(1);
                    }
                }
		break;
	    default:
		usage();
		exit(1);
	    }
	}
    }
    if ((rc = connect_server("localhost", port_newsserv)) < 0) {
	myerror("connect failed");
	die(1);
    }

    if (i >= argc)
	mail2news(argc, NULL, newsgroups, subject);
    else
	mail2news(argc, argv[i], newsgroups, subject);

    disconnect_server();
    logclose();
    exit(0);
}
void die(int rc)
{
    logclose();
    exit(rc);
}
void usage(void)
{
    printf("usage: %s -n newsgroups [-d][-s subject] [-h][-v] [-p port] [input]\n",
	   prgname);
}
void help(void)
{
    printf("\t%s\n\t------------\n"
	   "usage: m2news [-options] [inputfile]\n"
	   "options:\n"
	   "\t-n newsgroups   newsgroups leafnode\n"
	   "\t-s subject      subjects news\n"
	   "\t-d              current date to subject\n"
	   "\t-p port         port newsserv\n"
	   "\t-h              this help\n"
	   "\t-v              verbose\n"
	   "\n"
	   "Copyright (C) 2003-2004  Igor' Elohin <maint@unona.ru>\n"
	   "-------------------------------------------------------------\n"
	   "Eagle's rnews comes with ABSOLUTELY NO WARRANTY;\n"
	   "This is free software, and you are welcome to redistribute it\n"
	   "under certain conditions.\n",
	   version);
}

