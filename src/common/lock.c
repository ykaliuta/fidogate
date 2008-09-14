/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: lock.c,v 4.17 2004/08/22 20:19:11 n0ll Exp $
 *
 * File locking
 *
 *****************************************************************************
 * Copyright (C) 1990-2004
 *  _____ _____
 * |     |___  |   Martin Junius             <mj.at.n0ll.dot.net>
 * | | | |   | |   Radiumstr. 18
 * |_|_|_|@home|   D-51069 Koeln, Germany
 *
 * This file is part of FIDOGATE.
 *
 * FIDOGATE is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * FIDOGATE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FIDOGATE; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/

#include "fidogate.h"

#include <sys/types.h>
#include <fcntl.h>


/*
 * lock_fd() --- lock file using file descriptor, wait
 */
int lock_fd(int fd)
{
#ifndef HAS_FCNTL_LOCK
    return OK;
#else
    struct flock fl;
    int err;
    
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;
    
    do
    {
	err = fcntl(fd, F_SETLKW, &fl);
    }
    while(err == EINTR);
    
    return err;
#endif
}



/*
 * unlock_fd() --- unlock file using file descriptor
 */
int unlock_fd(int fd)
{
#ifndef HAS_FCNTL_LOCK
    return OK;
#else
    struct flock fl;
    int err;
    
    fl.l_type   = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;
    
    do
    {
	err = fcntl(fd, F_SETLKW, &fl);
    }
    while(err == EINTR);
    
    return err;
#endif
}



/*
 * lock_file() --- lock file using FILE *
 */
int lock_file(FILE *fp)
{
    return lock_fd( fileno(fp) );
}



/*
 * unlock_file() --- unlock file using FILE *
 */
int unlock_file(FILE *fp)
{
    return unlock_fd( fileno(fp) );
}



#ifdef NFS_SAFE_LOCK_FILES
/*
 * Create lock file, NFS-safe variant
 */
int lock_lockfile_nfs(char *name, int wait, char *id)
{
    char uniq_name[MAXPATH];
    int uniq_fd;
    int success;
    FILE *fp;
    struct stat st;
    
    BUF_COPY(uniq_name, name);
    str_printf(uniq_name+strlen(uniq_name),
	       sizeof(uniq_name)-strlen(uniq_name),
	       ".L%d", (int)getpid());

    /* create unique file */
    debug(7, "About to create unique %s (for lock %s)", uniq_name, name);

    uniq_fd = open(uniq_name, O_RDWR | O_CREAT | O_EXCL, BSY_MODE);
    if(uniq_fd == ERROR)
    {
	if(wait)
	{
	    logit("$ERROR: creating unique %s (for lock %s) failed",
		uniq_name, name);
	    exit(EX_OSFILE);
	}
	else
	{
	    logit("$WARNING: creating unique %s (for lock %s) failed",
		uniq_name, name);
	    return ERROR;
	}
    }
    if((fp = fdopen(uniq_fd, "w")))
    {
	if(id)
	    fprintf(fp, "%s\n", id);
	else
	    fprintf(fp, "%d\n", (int)getpid());
	fclose(fp);
    }
    close(uniq_fd);
    
    /* try to link to actual lock file */
    do
    {
	success = FALSE;
	if( link(uniq_name, name) == ERROR )
	{
	    /* Other errors than EEXIST are a failure */
	    if(errno != EEXIST)
	    {
		if(wait)
		{
		    logit("$ERROR: linking unique %s -> lock %s failed",
			uniq_name, name);
		    unlink(uniq_name);
		    exit(EX_OSFILE);
		}
		else
		{
		    logit("$WARNING: linking unique %s -> %s failed",
			uniq_name, name);
		    unlink(uniq_name);
		    return ERROR;
		}
	    }
	}
	else
	{
	    /* Link OK, check stat of unique */
	    if( stat(uniq_name, &st) == ERROR)
	    {
		/* Should not fail */
		logit("$ERROR: stat unique %s (for lock %s) failed",
		    uniq_name, name);
		unlink(uniq_name);
		exit(EX_OSFILE);
	    }
	    if(st.st_nlink == 2)
		success = TRUE;
	}

	debug(7, "Linking unique %s -> lock %s %s",
	      uniq_name, name, success ? "succeeded" : "failed");

	if(wait && !success)
	    sleep(5);
    }
    while(wait && !success);

    /* Always remove unique file */
    unlink(uniq_name);

    return success ? OK : ERROR;
}



/*
 * Delete lock file, NFS-safe variant
 */
int unlock_lockfile_nfs(char *name)
{
    int ret = OK;
    
    if( unlink(name) == ERROR )
    {
	logit("$WARNING: removing lock %s failed", name);
	ret = ERROR;
    }

    return ret;
}



#else /**!NFS_SAFE_LOCKFILES**/
/*
 * Create lock file with PID (id==NULL) or arbitrary string (id!=NULL)
 */
int lock_lockfile_id(char *name, int wait, char *id)
{
    int fd;
    FILE *fp;
    
    /* Create lock file */
    debug(7, "Creating lock file %s ...", name);
    do
    {
	/*
	 * Use open() with flag O_EXCL, this will fail if the
	 * lock file already exists
	 */
	fd = open(name, O_RDWR | O_CREAT | O_EXCL, BSY_MODE);
	debug(7, "Creating lock file %s %s.",
	      name, fd==-1 ? "failed" : "succeeded");
	if(fd != -1)
	{
	    if((fp = fdopen(fd, "w")))
	    {
		if(id)
		    fprintf(fp, "%s\n", id);
		else
		    fprintf(fp, "%d\n", (int)getpid());
		fclose(fp);
	    }
	    close(fd);
	}
	else if(wait)
	    sleep(5);
    }
    while(fd==-1 && wait);

    return fd==-1 ? ERROR : OK;
}



/*
 * Remove lock file
 */
int unlock_lockfile(char *name)
{
    int ret;
    
    ret = unlink(name);
    debug(7, "Deleting lock file %s %s.",
	  name, ret==-1 ? "failed" : "succeeded");

    return ret==-1 ? ERROR : OK;
}
#endif /**NFS_SAFE_LOCK_FILES**/



/*
 * Create lock file for program in SPOOLDIR/LOCKS
 */
int lock_program_id(char *name, int wait, char *id)
{
    char buf[MAXPATH];

    BUF_COPY3(buf, cf_p_lockdir(), "/", name);

#ifdef NFS_SAFE_LOCK_FILES
    return lock_lockfile_nfs(buf, wait, id);
#else
    return lock_lockfile_id(buf, wait, id);
#endif
}


int lock_program(char *name, int wait)
{
    char buf[MAXPATH];

    BUF_COPY3(buf, cf_p_lockdir(), "/", name);

#ifdef NFS_SAFE_LOCK_FILES
    return lock_lockfile_nfs(buf, wait, NULL);
#else
    return lock_lockfile_id(buf, wait, NULL);
#endif
}



/*
 * Remove lock file for program in SPOOLDIR/LOCKS
 */
int unlock_program(char *name)
{
    char buf[MAXPATH];
    
    BUF_COPY3(buf, cf_p_lockdir(), "/", name);

#ifdef NFS_SAFE_LOCK_FILES
    return unlock_lockfile_nfs(buf);
#else
    return unlock_lockfile(buf);
#endif
}



/***** Test program *********************************************************/
#ifdef TEST

int main(int argc, char *argv[])
{
    char *file;
    FILE *fp;
    int c;
    
    if(argc < 2)
    {
	fprintf(stderr, "usage: lock.c-TEST file\n");
	exit(1);
    }
    file = argv[1];

    if((fp = fopen(file, "a")) == NULL)
    {
	fprintf(stderr, "lock.c-TEST: can't open %s: ", file);
	perror("");
	exit(1);
    }    

    printf("Locking %s ...\n", file);
    if(lock_file(fp))
    {
	fprintf(stderr, "lock.c-TEST: can't lock %s: ", file);
	perror("");
	exit(1);
    }    
    printf("%s locked.\n", file);

    printf("Press <Return> ..."); fflush(stdout);
    while((c = getchar()) != '\n') ;

    printf("Unlocking %s ...\n", file);
    if(unlock_file(fp))
    {
	fprintf(stderr, "lock.c-TEST: can't unlock %s: ", file);
	perror("");
	exit(1);
    }    
    printf("%s unlocked.\n", file);

    exit(0);
}

#endif /**TEST**/
