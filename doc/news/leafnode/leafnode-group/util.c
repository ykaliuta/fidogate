#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/file.h>
#include <pwd.h>
#include "../common.h"

#ifdef _BSD
#include <sys/syslimits.h>
#endif

extern FILE *inp_nntp;
extern FILE *out_nntp;
extern char *prgname;
extern unsigned char *verbose;

int listnewsgroups()
{
    char line[BUFSIZE];
    char *l, *s;

    putaline("LIST");
    if (nntpreply() != 215) {
	myerror("%s: reading newsgroups failed", prgname);
	return -1;
    }
    while ((l = get_line(line)) && (strcmp(l, "."))) {
	if (s = strpbrk(line, " \t"))
	    *s = '\0';
	printf("%s\n", l);
    }
    return 0;
}

int makeactivelist()
{
    FILE *fd;
    char *l, *l2, *l3;
    char line[MSGBUF];
    struct passwd *pass;

    if ((pass = getpwnam(LEAFNODE_OWNER)) == NULL) {
	myerror("Can't get uid and guid user %", LEAFNODE_OWNER);
	return -1;
    }

    if ((fd = fopen(ACTIVE, "w")) == NULL) {
	myerror("Can't open %s", ACTIVE);
	return -1;
    }

    putaline("LIST ACTIVE");
    if (nntpreply() != 215) {
	myerror("%s: reading newsgroups failed", prgname);
	return -1;
    }
    while ((l = get_line(line)) && (strcmp(l, "."))) {
	fprintf(fd, "%s\n", l);
    }
    fclose(fd);
    if (chmod(ACTIVE, 0664)) {
	myerror("Can't chmod %", ACTIVE);
	remove(ACTIVE);
	return 1;
    }
    if (chown(ACTIVE, pass->pw_uid, pass->pw_gid)) {
	myerror("Can't chown %", ACTIVE);
	remove(ACTIVE);
	return 1;
    }
    message("Create %s ", ACTIVE);
    return 0;
}

/*
 * flag - определяет локальная группа или нет. 1 - локальная
 */
int createnewsgroup(char *groupname, char *description, unsigned char flag)
{
    FILE *fd;
    if (flag) {
	if ((fd = fopen(LOCAL_GROUPINFO, "a+")) == NULL) {
	    myerror("Can't open %s", LOCAL_GROUPINFO);
	    return -1;
	}
	fprintf(fd, "%s\ty\t%s\n", groupname, description);
    } else {
	if ((fd = fopen(GROUPINFO, "a+")) == NULL) {
	    myerror("Can't open %s", GROUPINFO);
	    return -1;
	}
	fprintf(fd, "%s\ty\t0\t0\t0\ty\t0\t0\t0\t%s\n", groupname,
		description);
    }
    fclose(fd);
    message("Create newsgroup: %s ", groupname);
    return 0;
}

int activatenewsgroup(char *groupname)
{
    FILE *fd;
    char fname[PATH_MAX];
    struct passwd *pass;

    if(islocalnewsgroup(groupname)) return 0;
    if (isactivenewsgroup(groupname))
	return 0;
    if ((pass = getpwnam(LEAFNODE_OWNER)) == NULL) {
	myerror("Can't get uid and guid user %", LEAFNODE_OWNER);
	return -1;
    }
    snprintf(fname, sizeof(fname), "%s/%s", INTERESTING_GROUPS, groupname);
    if ((fd = fopen(fname, "w")) == NULL) {
	myerror("Can't create %s", fname);
	return -1;
    }
    fclose(fd);
    if (chmod(fname, 0664)) {
	myerror("Can't chmod %", fname);
	remove(fname);
	return 1;
    }
    if (chown(fname, pass->pw_uid, pass->pw_gid)) {
	myerror("Can't chown %", fname);
	remove(fname);
	return 1;
    }
    message("Activate newsgroup: %s", groupname);
    return 0;
}

int deactivatenewsgroup(char *groupname)
{
    int rc = 0;
    char fname[PATH_MAX];

    if(islocalnewsgroup(groupname)) return 0;
    if (isactivenewsgroup(groupname)) {
	snprintf(fname, sizeof(fname), "%s/%s", INTERESTING_GROUPS, groupname);
	rc = remove(fname);
	if (rc)
	    myerror("Can't remove %s\n", fname);
	else
	    message("Deactivate newsgroup: %s", groupname);
    }
    return rc;
}

int islocalnewsgroup(char *groupname)
{
    FILE *fd;
    char buf[4096];
    char *s;
    int rc = 0;

    if ((fd = fopen(LOCAL_GROUPINFO, "r")) == NULL) {
	myerror("Can't open %s", LOCAL_GROUPINFO);
	return -1;
    }
    while (fgets(buf, 4095, fd) != NULL) {
	if (s = strpbrk(buf, " \t"))
	    *s = '\0';
	if (strcmp(buf, groupname) == 0) {
	    rc = 2;
	    break;
	}
    }
    fclose(fd);
    return rc;
}

int isactivenewsgroup(char *groupname)
{
    char fname[PATH_MAX];

    if(islocalnewsgroup(groupname)) return 0;
    snprintf(fname, sizeof(fname), "%s/%s", INTERESTING_GROUPS, groupname);
    if (access(fname, F_OK) == 0){
        if(verbose) message("Group %s inactive", fname);
	return 1;
    } else {
        if(verbose) message("Group %s active, fname");
	return 0;
    }
}

int removenewsgroup(char *groupname)
{
    FILE *ifd, *ofd;
    int fd;
    char *fname;
    char buf[4096];
    char tmpname[4096];
    char *s;
    struct passwd *pass;

    if ((pass = getpwnam(LEAFNODE_OWNER)) == NULL) {
	myerror("Can't get uid and guid user %", LEAFNODE_OWNER);
	return -1;
    }

    if (!isgrouponserver(groupname) && !islocalnewsgroup(groupname)) {
	myerror("Group not found %s", groupname);
	return 1;
    }
    if (isactivenewsgroup(groupname)) {
	if (deactivatenewsgroup(groupname)) {
	    myerror("Can't deactivate group %s", groupname);
	    return 1;
	}
    }

    if (islocalnewsgroup(groupname))
	removefromfile(LOCAL_GROUPINFO, groupname);
    removefromfile(GROUPINFO, groupname);
    message("Remove newsgroup: %s", groupname);
    return 0;
}
int removefromfile(char *fname, char *groupname)
{
    FILE *ifd, *ofd;
    int fd;
    char buf[4096];
    char tmpname[4096];
    char *s;
    struct passwd *pass;

    if ((pass = getpwnam(LEAFNODE_OWNER)) == NULL) {
	myerror("Can't get uid and guid user %", LEAFNODE_OWNER);
	return -1;
    }
    strcpy(tmpname, "/tmp/leafnodeXXXXXX");
    if ((fd = mkstemp(tmpname)) < 0) {
	myerror("Can't create temporary file");
	return 1;
    }
    if ((ifd = fopen(fname, "r")) == NULL) {
	myerror("Can't open %s", fname);
	return -1;
    }
    if (flock(fileno(ifd), LOCK_EX)) {
	myerror("Can't lock file %s", fname);
	return 1;
    }
    if ((ofd = fdopen(fd, "w")) == NULL) {
	myerror("Can't create temporary file");
	return 1;
    }
    while (fgets(buf, 4095, ifd) != NULL) {
	if (s = strpbrk(buf, " \t"))
	    *s = '\0';
	if (strcmp(buf, groupname)) {
	    *s = '\t';
	    fputs(buf, ofd);
	}
    }
    flock(fileno(ifd), LOCK_UN);
    fclose(ofd);
    fclose(ifd);
    movefile(tmpname, fname);
    if (chmod(fname, 0664)) {
	myerror("Can't chmod %", fname);
	remove(fname);
	return 1;
    }
    if (chown(fname, pass->pw_uid, pass->pw_gid)) {
	myerror("Can't chown %", fname);
	remove(fname);
	return 1;
    }
    return 0;
}

