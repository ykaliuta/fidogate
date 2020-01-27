/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * File locking
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
 *  _____ _____
 * |     |___  |   Martin Junius             <mj@fidogate.org>
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

#include <signal.h>
#include <sys/types.h>
#include <errno.h>

#define LOCKFILE_UNLOCKED	0
#define LOCKFILE_LOCKED		1
#define LOCKFILE_ERROR		-1

/*
 * May be there is as stale lock?
 */
int check_stale_lock(char *name)
{
    char buff[32];
    pid_t pid;
    FILE *fp;
    char *ret;

    if ((fp = fopen(name, "r")) == NULL) {
        if (errno == ENOENT)
            return LOCKFILE_UNLOCKED;
        else
            return LOCKFILE_ERROR;
    }
    ret = fgets(buff, sizeof(buff), fp);
    fclose(fp);

    if (ret == NULL) {
        fglog("$ERROR: cannot read lock file");
        return LOCKFILE_ERROR;
    }

    if (strlen(buff) > 0)
        pid = (pid_t) atoi(buff);
    else
        pid = -1;

    if (kill(pid, 0) == 0 || errno == EPERM)
        return LOCKFILE_LOCKED;

    fglog("$WARNING: stale lock file %s (pid = %d) found", name, pid);
    if (unlink(name) != 0) {
        debug(7, "Deleteting stale lock file %s failed.", name);
        return LOCKFILE_ERROR;
    }
    return LOCKFILE_UNLOCKED;
}

/*
 * lock_fd() --- lock file using file descriptor, wait
 */
int lock_fd(int fd)
{
#ifndef HAVE_FCNTL_H
    return OK;
#else
    struct flock fl;
    int err;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    do {
        err = fcntl(fd, F_SETLKW, &fl);
    }
    while (err == EINTR);

    return err;
#endif
}

/*
 * unlock_fd() --- unlock file using file descriptor
 */
int unlock_fd(int fd)
{
#ifndef HAVE_FCNTL_H
    return OK;
#else
    struct flock fl;
    int err;

    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    do {
        err = fcntl(fd, F_SETLKW, &fl);
    }
    while (err == EINTR);

    return err;
#endif
}

/*
 * lock_file() --- lock file using FILE *
 */
int lock_file(FILE * fp)
{
    return lock_fd(fileno(fp));
}

/*
 * unlock_file() --- unlock file using FILE *
 */
int unlock_file(FILE * fp)
{
    return unlock_fd(fileno(fp));
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
    str_printf(uniq_name + strlen(uniq_name),
               sizeof(uniq_name) - strlen(uniq_name), ".L%d", (int)getpid());

    /* create unique file */
    debug(7, "About to create unique %s (for lock %s)", uniq_name, name);

    uniq_fd = open(uniq_name, O_RDWR | O_CREAT | O_EXCL, BSY_MODE);
    if (uniq_fd == ERROR) {
        if (wait) {
            fglog("$ERROR: creating unique %s (for lock %s) failed",
                  uniq_name, name);
            exit(EX_OSFILE);
        } else {
            fglog("$WARNING: creating unique %s (for lock %s) failed",
                  uniq_name, name);
            return ERROR;
        }
    }
    if ((fp = fdopen(uniq_fd, "w"))) {
        if (id)
            fprintf(fp, "%s\n", id);
        else
            fprintf(fp, "%d\n", (int)getpid());
        fclose(fp);
    }
    close(uniq_fd);

    /* try to link to actual lock file */
    do {
        success = FALSE;
        check_stale_lock(name);

        if (link(uniq_name, name) == ERROR) {
            /* Other errors than EEXIST are a failure */
            if (errno != EEXIST) {
                if (wait) {
                    fglog("$ERROR: linking unique %s -> lock %s failed",
                          uniq_name, name);
                    unlink(uniq_name);
                    exit(EX_OSFILE);
                } else {
                    fglog("$WARNING: linking unique %s -> %s failed",
                          uniq_name, name);
                    unlink(uniq_name);
                    return ERROR;
                }
            }
        } else {
            /* Link OK, check stat of unique */
            if (stat(uniq_name, &st) == ERROR) {
                /* Should not fail */
                fglog("$ERROR: stat unique %s (for lock %s) failed",
                      uniq_name, name);
                unlink(uniq_name);
                exit(EX_OSFILE);
            }
            if (st.st_nlink == 2)
                success = TRUE;
        }

        debug(7, "Linking unique %s -> lock %s %s",
              uniq_name, name, success ? "succeeded" : "failed");

        if (wait && !success)
            sleep(5);
    }
    while (wait && !success);

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

    if (unlink(name) == ERROR) {
        fglog("$WARNING: removing lock %s failed", name);
        ret = ERROR;
    }

    return ret;
}

#else /**!NFS_SAFE_LOCKFILES**/
/*
 * Create lock file with PID (id==NULL) or arbitrary string (id!=NULL)
 * Update: id != NULL lock is removed, is not used
 */
int lock_lockfile(char *name, int wait)
{
    FILE *fp = NULL;
    short int wait_time = FALSE;
    short int exists_lock = FALSE;

    if (wait && wait != WAIT)
        wait_time = TRUE;

    /* Create lock file */
    debug(7, "Creating lock file %s ...", name);
    do {
        /* if there is a stale lock, this function will remove it */
        switch (check_stale_lock(name)) {
        case LOCKFILE_UNLOCKED:
            if ((fp = fopen(name, "w")) == NULL)
                return ERROR;
            fprintf(fp, "%d\n", (int)getpid());
            fclose(fp);
            return OK;

        case LOCKFILE_LOCKED:
            debug(7, "Lock exists %s", name);
            exists_lock = TRUE;
            break;

        default:
            return ERROR;
        }

        if (wait > 0) {
            if (wait_time) {
                sleep(1);
                wait--;
            } else
                sleep(5);
        }
    }
    while (exists_lock && wait > 0);

    return exists_lock ? ERROR : OK;
}

/*
 * Remove lock file
 */
int unlock_lockfile(char *name)
{
    int ret;

    ret = unlink(name);
    debug(7, "Deleting lock file %s %s.",
          name, ret == -1 ? "failed" : "succeeded");

    return ret == -1 ? ERROR : OK;
}
#endif /**NFS_SAFE_LOCK_FILES**/

int lock_program(char *name, int wait)
{
    char buf[MAXPATH];

    BUF_COPY3(buf, cf_p_lockdir(), "/", name);

#ifdef NFS_SAFE_LOCK_FILES
    return lock_lockfile_nfs(buf, wait, NULL);
#else
    return lock_lockfile(buf, wait);
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

int lock_path(char *path, int wait)
{
    char xpath[MAXPATH];

    BUF_EXPAND(xpath, path);
#ifdef NFS_SAFE_LOCK_FILES
    return lock_lockfile_nfs(xpath, wait, NULL);
#else
    return lock_lockfile(xpath, wait);
#endif
}

void unlock_path(char *path)
{
    char xpath[MAXPATH];

    BUF_EXPAND(xpath, path);
#ifdef NFS_SAFE_LOCK_FILES
    unlock_lockfile_nfs(xpath);
#else
    unlock_lockfile(xpath);
#endif
    return;
}

/***** Test program *********************************************************/
#ifdef TEST

int main(int argc, char *argv[])
{
    char *file;
    FILE *fp;
    int c;

    if (argc < 2) {
        fprintf(stderr, "usage: lock.c-TEST file\n");
        exit(1);
    }
    file = argv[1];

    if ((fp = fopen(file, "a")) == NULL) {
        fprintf(stderr, "lock.c-TEST: can't open %s: ", file);
        perror("");
        exit(1);
    }

    printf("Locking %s ...\n", file);
    if (lock_file(fp)) {
        fprintf(stderr, "lock.c-TEST: can't lock %s: ", file);
        perror("");
        exit(1);
    }
    printf("%s locked.\n", file);

    printf("Press <Return> ...");
    fflush(stdout);
    while ((c = getchar()) != '\n') ;

    printf("Unlocking %s ...\n", file);
    if (unlock_file(fp)) {
        fprintf(stderr, "lock.c-TEST: can't unlock %s: ", file);
        perror("");
        exit(1);
    }
    printf("%s unlocked.\n", file);

    exit(0);
}

#endif /**TEST**/
