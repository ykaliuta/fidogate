/*
 * (C) Maint Laboratory 2000-2004
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
/*
 * Пересылка файла ifn в файл ofn */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#if _WIN32 || MSDOS
#include <io.h>
#endif

int movefile(const char *ifn, const char *ofn)
{
    int ifd;
    int ofd;
    char buf[4096];
    int n_read;

    if (rename(ifn, ofn)) {
	if ((ifd = open(ifn, O_RDONLY
#if _WIN32 || MSDOS
			| O_BINARY
#endif
	     )) < 0) {
	    return 1;
	}
	if ((ofd = open(ofn, O_WRONLY | O_TRUNC | O_CREAT
#if _WIN32 || MSDOS
			| O_BINARY
#endif
			, 0600)) < 0) {
	    return 2;
	}
	while ((n_read = read(ifd, buf, 4096)) > 0) {
	    if (write(ofd, buf, n_read) < n_read) {
		return 3;
	    }
	}
	close(ifd);
	close(ofd);
	remove(ifn);
    }
    return 1;
}
