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
#include <pwd.h>
#include "../common.h"

char *prgname;
void usage(void);
void help(void);
char *version = "leafnode-util 0.1";
int verbose;

int main(int argc, char *argv[])
{
    int i, rc;
    char c;
    char *groupname;
    unsigned char flagLIST = 0;
    unsigned char flagCHECK = 0;
    unsigned char flagMAKE = 0;
    unsigned char flagMAKE_LOCAL = 0;
    unsigned char flagACTIVATE = 0;
    unsigned char flagDEACTIVATE = 0;
    unsigned char flagREMOVE = 0;
    unsigned char flagACTIVELIST = 0;

    prgname = strrchr(argv[0], '/');
    if (prgname == NULL)
	prgname = argv[0];
    else
	prgname++;

    if (argc > 1) {
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
	    c = argv[i][1];
	    switch (c) {
	    case 'h':
		help();
		return 0;
	    case 'v':
		verbose = 1;
		break;
	    case 'l':
		flagLIST = 1;
		break;
	    case 'c':
		flagCHECK = 1;
		break;
	    case 'm':
		flagMAKE = 1;
		break;
	    case 'M':
		flagMAKE = 1;
		flagACTIVATE = 1;
		break;
	    case 'a':
		flagACTIVATE = 1;
		break;
	    case 'd':
		flagDEACTIVATE = 1;
		break;
	    case 'o':
		flagMAKE_LOCAL = 1;
		break;
	    case 'r':
		flagREMOVE = 1;
		break;
	    case 'L':
		flagACTIVELIST = 1;
		break;
	    default:
		help();
		return 0;
	    }
	}
    } else {
	usage();
	return 0;
    }
    if (i >= argc && !flagLIST && !flagACTIVELIST) {
	usage();
	return 0;
    }
    loginit("leafnode-group", NEWSLOGDIR);
    if (setgid(getegid()) < 0) {
	error("cannot setgid to %d", getegid());
	exit(1);
    }
    if (setuid(geteuid()) < 0) {
	error("cannot setuid to %d", geteuid());
	exit(1);
    }
    if (connect_server("localhost", 119) < 0) {
	error("connect failed");
	die(1);
    }

    groupname = argv[i];
    if (flagLIST)
	rc = listnewsgroups(NULL);
    else if (flagACTIVELIST)
	rc = makeactivelist();
    else if (flagCHECK){
	rc = !isgrouponserver(groupname);
	if(verbose){
	    if(rc) message("%s", groupname);
	    else message("Group not found %s", groupname);
	}
    } else if (flagREMOVE) {
	rc = removenewsgroup(groupname);
    } else if (flagMAKE || flagMAKE_LOCAL) {
	if (!isgrouponserver(groupname)) {
	    char comment[4096];
	    snprintf(comment, sizeof(comment), diagtime());
	    rc = createnewsgroup(groupname, comment,
				 (flagMAKE == 1 ? 0 : 1));
	}
	rc = 0;
    } else if (flagDEACTIVATE) {
	if (!islocalnewsgroup(groupname))
	    rc = deactivatenewsgroup(groupname);
    }
    if (flagACTIVATE) {
	if (!islocalnewsgroup(groupname))
	    rc = activatenewsgroup(groupname);
    }

    disconnect_server();
    logclose();
    return rc;
}
void die(int rc)
{
    logclose();
    exit(rc);
}
void usage(void)
{
    printf("usage: %s [-options] [group]\n", prgname);
}
void help(void)
{
    printf("\t%s\n\t------------------\n"
	   "usage: %s [-options] [group]\n"
	   "options:\n"
	   "\t-h           this help\n"
	   "\t-v           verbose\n"
	   "\t-l           list groups\n"
	   "\t-m           create group\n"
	   "\t-M           create and activate group\n"
	   "\t-o           create local group\n"
	   "\t-a           activate group\n"
	   "\t-d           deactivate group\n"
	   "\t-r           remove group\n"
	   "\t-c           check group\n"
	   "\t-L           make \'active\' file\n"
	   "\n"
	   "Copyright (C) 2003  Igor Elohin <maint@unona.ru>\n"
	   "-------------------------------------------------------------\n"
	   "Eagle's %s comes with ABSOLUTELY NO WARRANTY;\n"
	   "This is free software, and you are welcome to redistribute it\n"
	   "under certain conditions\n",
	   prgname, prgname, version);
}
