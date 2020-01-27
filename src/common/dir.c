/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Reading/sorting directories
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

/*
 * struct for sorted dir
 */
typedef struct st_direntry {
    char *name;
    off_t size;
    time_t mtime;
} DirEntry;

#define DIR_INITSIZE	50

static DirEntry *dir_array = NULL;  /* Array sorted dir */
static int dir_narray = 0;      /* Array size */
static int dir_nentry = 0;      /* Array entries */
static int dir_smode = DIR_SORTNAME;    /* Sort mode */

/*
 * Prototypes
 */
static void dir_resize(int);
static int dir_compare(const void *, const void *);

/*
 * Resize DirEntry array
 */
static void dir_resize(int new)
{
    DirEntry *old;
    int i;

    old = dir_array;

    dir_array = (DirEntry *) xmalloc(new * sizeof(DirEntry));

    /* Copy old entries */
    for (i = 0; i < dir_narray; i++)
        dir_array[i] = old[i];

    /* Init new entries */
    for (; i < new; i++) {
        dir_array[i].name = NULL;
        dir_array[i].size = 0;
        dir_array[i].mtime = 0;
    }

    xfree(old);

    dir_narray = new;
}

/*
 * Comparison function for qsort()
 */
int dir_compare(const void *pa, const void *pb)
{
    DirEntry *a, *b;
    int ret;

    a = (DirEntry *) pa;
    b = (DirEntry *) pb;

    switch (dir_smode) {
    case DIR_SORTNAME:
        return strcmp(a->name, b->name);
    case DIR_SORTNAMEI:
        return stricmp(a->name, b->name);
    case DIR_SORTSIZE:
        ret = a->size - b->size;
        if (ret != 0)
            return ret;
        return strcmp(a->name, b->name);
    case DIR_SORTMTIME:
        ret = a->mtime - b->mtime;
        if (ret != 0)
            return ret;
        return strcmp(a->name, b->name);
    case DIR_SORTNONE:
    default:
        return 0;
    }

    /**NOT REACHED**/
    return 0;
}

/*
 * Read dir into memory and sort
 */
int dir_open(char *dirname, char *pattern, int ic)
    /* ic --- TRUE=ignore case, FALSE=don't */
{
    char name[MAXPATH];
    char buf[MAXPATH];
    struct dirent *dir;
    DIR *dp;
    struct stat st;

    BUF_EXPAND(name, dirname);

    if (dir_array)
        dir_close();

    /* Open and read directory */
    if (!(dp = opendir(name)))
        return ERROR;

    dir_resize(DIR_INITSIZE);

    while ((dir = readdir(dp)))
        if (pattern == NULL || wildmatch(dir->d_name, pattern, ic)) {
            BUF_COPY3(buf, name, "/", dir->d_name);

            if (stat(buf, &st) == ERROR) {
                dir_close();
                return ERROR;
            }

            if (dir_nentry >= dir_narray)
                dir_resize(2 * dir_narray);
            dir_array[dir_nentry].name = strsave(buf);
            dir_array[dir_nentry].size = st.st_size;
            dir_array[dir_nentry].mtime = st.st_mtime;

            dir_nentry++;
        }

    closedir(dp);

    /* Sort it */
    qsort(dir_array, dir_nentry, sizeof(DirEntry), dir_compare);

    return OK;
}

/*
 * Delete sorted directory array
 */
void dir_close(void)
{
    int i;

    for (i = 0; i < dir_nentry; i++)
        xfree(dir_array[i].name);

    xfree(dir_array);

    dir_array = NULL;
    dir_narray = 0;
    dir_nentry = 0;
}

/*
 * Set sort mode
 */
void dir_sortmode(int mode)
{
    dir_smode = mode;
}

/*
 * Get first/next entry
 */
char *dir_get(int first)
{
    static int index = 0;

    if (first)
        index = 0;

    if (index < dir_nentry)
        return dir_array[index++].name;

    return NULL;
}

char *dir_get_mtime(time_t mtime, char first)
{
    static int index = 0;

    if (first)
        index = 0;
    else
        index++;

    for (; index < dir_nentry; index++)
        if (mtime < dir_array[index].mtime)
            return dir_array[index].name;

    return NULL;
}

/*
 * Search for file in directory, ignoring case
 */
char *dir_search(char *dirname, char *filename)
    /* name --- file name to find, overwritten with correct case if found */
{
    char name[MAXPATH];
    struct dirent *dir;
    DIR *dp;

    BUF_EXPAND(name, dirname);

    /* Open and read directory */
    if (!(dp = opendir(name)))
        return NULL;

    while ((dir = readdir(dp)))
        if (!stricmp(dir->d_name, filename)) {
            str_copy(filename, strlen(filename) + 1, dir->d_name);
            closedir(dp);
            return filename;
        }

    closedir(dp);
    return NULL;
}

int mkdir_r(char *dir, mode_t mode)
{
    char tmp[MAXPATH] = "";
    int i = 0, n;
    char *p;

    if (check_access(dir, CHECK_DIR) == TRUE)
        return OK;
    if (mkdir(dir, mode) == 0)
        return OK;
    if (errno == EEXIST)
        return OK;

    p = dir;
    n = strlen(dir);

    while (n > i) {
        tmp[i] = *p;
        p++;
        i++;
        while (n > i) {
            if (*p != '/')
                tmp[i] = *p;
            else
                break;
            p++;
            i++;
        }
        tmp[i + 1] = 0;

        if (check_access(tmp, CHECK_DIR) != TRUE) {
            fglog("make directory %s", tmp);
            if (mkdir(tmp, mode) != 0)
                return ERROR;
        }
    }

    return OK;
}

#ifdef TEST

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: testdir dir\n");
    } else {
        char *n;

        printf("%s (name):\n", argv[1]);
        dir_sortmode(DIR_SORTNAME);
        dir_open(argv[1], NULL, FALSE);
        for (n = dir_get(TRUE); n; n = dir_get(FALSE))
            printf(" %s\n", n);
        printf("\n");

        printf("%s (size):\n", argv[1]);
        dir_sortmode(DIR_SORTSIZE);
        dir_open(argv[1], NULL, FALSE);
        for (n = dir_get(TRUE); n; n = dir_get(FALSE))
            printf(" %s\n", n);
        printf("\n");

        printf("%s (*.pkt, mtime):\n", argv[1]);
        dir_sortmode(DIR_SORTMTIME);
        dir_open(argv[1], "*.pkt", TRUE);
        for (n = dir_get(TRUE); n; n = dir_get(FALSE))
            printf(" %s\n", n);
        printf("\n");
    }

    return 0;
}

#endif /**TEST**/
