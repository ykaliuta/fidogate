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
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "../common.h"

#ifdef _BSD
#include <sys/syslimits.h>
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

    if ((dir_fd = opendir(OUTGOING)) == NULL) {
	error("Cannot open %s (%s)", OUTGOING, sys_errlist[errno]);
	return 1;
    }
    while ((dir = readdir(dir_fd)) != NULL) {
	if (strcmp(dir->d_name, ".") == 0 ||
	    strcmp(dir->d_name, "..") == 0)
	    continue;
	sprintf(curr_name, "%s/%s", OUTGOING, dir->d_name);
	if (stat(curr_name, &stbuf) < 0)
	    continue;
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
	    continue;
	if((fd = fopen(curr_name, "r")) == NULL){
	    error("%s: %s", curr_name, sys_errlist[errno]);
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
			error("%s: %s", curr_name, sys_errlist[errno]);
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
