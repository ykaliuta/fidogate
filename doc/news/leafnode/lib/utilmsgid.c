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
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "../common.h"
#include "../config.h"

#ifdef _BSD
#include <limits.h>
#endif

int erase_outgoing_msgid(char *msgid)
{
    struct dirent *dir;
    struct stat stbuf;
    DIR *dir_fd;
    char curr_name[PATH_MAX];
    FILE *fd;
    char buf[BUFSIZE];
    char *s;

    if ((dir_fd = opendir(outgoing)) == NULL) {
	myerror("Cannot open %s (%s)", outgoing, strerror(errno));
	return 1;
    }
    while ((dir = readdir(dir_fd)) != NULL) {
	if (strcmp(dir->d_name, ".") == 0 ||
	    strcmp(dir->d_name, "..") == 0)
	    continue;
	snprintf(curr_name, sizeof(curr_name), "%s/%s", outgoing, dir->d_name);
	if (stat(curr_name, &stbuf) < 0)
	    continue;
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
	    continue;
	if((fd = fopen(curr_name, "r")) == NULL){
	    myerror("%s: %s", curr_name, strerror(errno));
            continue;
	}
	while (fgets(buf, BUFSIZE, fd)) {
	    if (strncmp(buf, "Message-ID: ", 12) == 0) {
		if ((s = strchr(buf, 0x0a)) != NULL)
		    *s = '\0';
		if ((s = strchr(buf, 0x0d)) != NULL)
		    *s = '\0';
		if (strcmp(msgid, &buf[12]) == 0) {
		    fclose(fd);
		    if (remove(curr_name)) {
			myerror("%s: %s", curr_name, strerror(errno));
		    }
                    goto b1;
		}
	    }
	}
        fclose(fd);
    }
b1:
    closedir(dir_fd);
    return 0;
}
int isfidogroup(char *str)
{
    char list[128], ie[128], *s, *b;
    char t[128];

    if(ffgroups[0] == '\0') return 1;

    strcpy(list, ffgroups);
    b = list;
    while((s = strtok(b, ", ")) != NULL){
        strcpy(t, s);
        strcat(t, ".");
        if(!strncmp(t, str, strlen(t))) return 1;
        b = NULL;
    }
    return 0;
}
