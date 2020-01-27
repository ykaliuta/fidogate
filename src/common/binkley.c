/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * BinkleyTerm-style outbound directory functions
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
 * BinkleyTerm flavors and FLO/OUT file extensions
 */
#define NOUTB		5

static struct st_outb {
    int type;
    char flo[4];
    char out[4];
    char flav[8];
    char shrt[2];
} outb_types[NOUTB] = {
    {FLAV_NONE, "", "", "None", "-"},
    {FLAV_HOLD, "hlo", "hut", "Hold", "H"},
    {FLAV_NORMAL, "flo", "out", "Normal", "N"},
    {FLAV_DIRECT, "dlo", "dut", "Direct", "D"},
    {FLAV_CRASH, "clo", "cut", "Crash", "C"}
};

/*
 * FLAV_* flavor code to string
 */
char *flav_to_asc(int flav)
{
    int i;

    for (i = 0; i < NOUTB; i++)
        if (outb_types[i].type == flav)
            return outb_types[i].flav;

    return "Normal";
}

/*
 * String to FLAV_* flavor code
 */
/*int asc_to_flav(char *flav)
{
    int i;

    for(i=0; i<NOUTB; i++)
	if(!stricmp(outb_types[i].flav, flav))
	    return outb_types[i].type;

    return ERROR;
}*/

/*
 * Convert node address to outbound base name
 */
char *bink_out_name(Node * node)
{
    static char buf[MAXPATH];
    char *out, *outbound;
    int aso = FALSE;

    if (cf_get_string("AmigaStyleOutbound", TRUE) != NULL)
        aso = TRUE;

    if (aso)
        out = cf_zones_out(0);
    else
        out = cf_zones_out(node->zone);

    if (!out)
        return NULL;
    outbound = cf_p_btbasedir();
    if (!outbound)
        return NULL;

    if (aso) {
        str_printf(buf, sizeof(buf), "%s/%s/%d.%d.%d.%d.",
                   outbound, out, node->zone,
                   node->net, node->node, node->point);
    } else {
        if (node->point > 0)
            str_printf(buf, sizeof(buf), "%s/%s/%04x%04x.pnt/0000%04x.",
                       outbound, out, node->net, node->node, node->point);
        else
            str_printf(buf, sizeof(buf), "%s/%s/%04x%04x.",
                       outbound, out, node->net, node->node);
    }

    return buf;
}

/*
 * Name of BSY file for a node
 */
char *bink_bsy_name(Node * node)
{
    static char buf[MAXPATH];
    char *out;

    out = bink_out_name(node);
    if (!out)
        return NULL;

    BUF_COPY2(buf, out, "bsy");
    debug(6, "node=%s bsy file=%s", znfp1(node), buf);
    return buf;
}

/*
 * Test for existing BSY file
 */
int bink_bsy_test(Node * node)
{
    char *name = bink_bsy_name(node);

    if (!name)
        return FALSE;

    return check_access(name, CHECK_FILE) == TRUE;
}

/*
 * Create BSY file for a node
 */
int bink_bsy_create(Node * node, int wait)
{
#ifdef DO_BSY_FILES
    char *name = bink_bsy_name(node);

    if (!name)
        return ERROR;

    /* Create directory if necessary */
    if (bink_mkdir(node) == ERROR)
        return ERROR;

    /* Create BSY file */
#ifdef NFS_SAFE_LOCK_FILES
    return lock_lockfile_nfs(name, wait, NULL);
#else
    return lock_lockfile(name, wait);
#endif
#else
    return OK;
#endif
}

/*
 * Delete BSY file for a node
 */
int bink_bsy_delete(Node * node)
{
#ifdef DO_BSY_FILES
    char *name = bink_bsy_name(node);
    int ret;

    if (!name)
        return ERROR;

    ret = unlink(name);
    debug(5, "Deleting BSY file %s %s.",
          name, ret == -1 ? "failed" : "succeeded");

    return ret == -1 ? ERROR : OK;
#else
    return OK;
#endif
}

/*
 * Find FLO file for node
 *
 * flav==NULL: only return non-NULL if existing FLO file found.
 * flav!=NULL: return existing FLO file or name of new FLO file according
 *             to flav.
 */
char *bink_find_flo(Node * node, char *flav)
{
    static char buf[MAXPATH];
    char *outb, *flo = NULL;
    int i;

    outb = bink_out_name(node);
    if (!outb)
        return NULL;

    /*
     * Search existing FLO files first
     */
    for (i = 1; i < NOUTB; i++) {
        BUF_COPY2(buf, outb, outb_types[i].flo);
        if (access(buf, F_OK) == 0) {
            /* FLO file exists */
            debug(5, "found FLO file %s", buf);
            return buf;
        }
    }

    if (!flav)
        return NULL;

    /*
     * No FLO file exists, new one with flavor from arg
     */
    for (i = 1; i < NOUTB; i++) {
        if (!stricmp(outb_types[i].flav, flav) ||
            !stricmp(outb_types[i].shrt, flav) ||
            !stricmp(outb_types[i].flo, flav))
            flo = outb_types[i].flo;
    }
    if (!flo)
        return NULL;

    BUF_COPY2(buf, outb, flo);
    debug(5, "new FLO file %s", buf);
    return buf;
}

/*
 * Find OUT file for node
 *
 * flav==NULL: only return non-NULL if existing OUT file found.
 * flav!=NULL: return existing OUT file or name of new OUT file according
 *             to flav.
 */
char *bink_find_out(Node * node, char *flav)
{
    static char buf[MAXPATH];
    char *outb, *out = NULL;
    int i;

    outb = bink_out_name(node);
    if (!outb)
        return NULL;

    /*
     * Search existing OUT files first
     */
    for (i = 1; i < NOUTB; i++) {
        BUF_COPY2(buf, outb, outb_types[i].out);
        if (access(buf, F_OK) == 0) {
            /* OUT file exists */
            debug(5, "found OUT file %s", buf);
            return buf;
        }
    }

    if (!flav)
        return NULL;

    /*
     * No OUT file exists, new one with flavor from arg
     */
    for (i = 1; i < NOUTB; i++) {
        if (!stricmp(outb_types[i].flav, flav) ||
            !stricmp(outb_types[i].shrt, flav) ||
            !stricmp(outb_types[i].out, flav))
            out = outb_types[i].out;
    }
    if (!out)
        return NULL;

    BUF_COPY2(buf, outb, out);
    debug(5, "new OUT file %s", buf);
    return buf;
}

/*
 * Attach file to FLO control file
 */
int bink_attach(Node * node, int mode, char *name, char *flav, int bsy)
{
    FILE *fp;
    char *n;
    char *line;
    int lmode, found;
    static char buf[MAXPATH];

    if (mode)
        debug(4, "attach mode=%c (^=delete, #=trunc)", mode);
    debug(4, "attach name=%s", name);

    if (cf_dos()) {             /* MSDOS translation enabled? */
        n = cf_dos_xlate(name);
        if (!n) {
            fglog("can't convert file name to MSDOS: %s", name);
            return ERROR;
        }
        debug(4, "attach MSDOS name=%s", n);
    } else
        n = name;

    if (flo_openx(node, bsy, flav, TRUE) == ERROR)
        return ERROR;
    fp = flo_file();

    /* seek to start of flo file */
    if (fseek(fp, 0L, SEEK_SET) == ERROR) {
        fglog("$fseek EOF FLO file node %s failed", znfp1(node));
        flo_close(node, TRUE, FALSE);
        return ERROR;
    }

    /* read FLO entries, check if file attachment exists */
    found = FALSE;
    while ((line = flo_gets(buf, sizeof(buf)))) {
        if (*line == '~')
            continue;
        lmode = ' ';
        if (*line == '^' || *line == '#')
            lmode = *line++;

        debug(5, "FLO entry: %c %s", lmode, line);
        if (streq(line, n)) {
            found = TRUE;
            debug(5, "           found entry");
        }
    }

    /* We're there ...  */
    if (found)
        debug(4, "FLO file already contains an entry, not attaching file");
    else {
        debug(4, "FLO file open and locking succeeded, attaching file");
        if (mode)
            fprintf(fp, "%c%s%s", mode, n, cf_dos()? "\r\n" : "\n");
        else
            fprintf(fp, "%s%s", n, cf_dos()? "\r\n" : "\n");
    }

    flo_close(node, bsy, FALSE);

    return OK;
}

/*
 * Check access for file/directory
 */
int check_access(char *name, int check)
{
    struct stat st;

    if (stat(name, &st) == -1)
        return ERROR;

    if (check == CHECK_FILE && S_ISREG(st.st_mode))
        return TRUE;
    if (check == CHECK_DIR && S_ISDIR(st.st_mode))
        return TRUE;

    return FALSE;
}

/*
 * Create directory for zone/points if needed
 */
int bink_mkdir(Node * node)
{
    char buf[MAXPATH];
    char *base;
    size_t rest;
    int aso = FALSE;

    if (cf_get_string("AmigaStyleOutbound", TRUE) != NULL)
        aso = TRUE;

    /*
     * Outbound dir + zone dir
     */
    BUF_COPY2(buf, cf_p_btbasedir(), "/");

    if (aso)
        base = cf_zones_out(0);
    else
        base = cf_zones_out(node->zone);

    if (base == NULL)
        return ERROR;
    BUF_APPEND(buf, base);
    base = buf + strlen(buf);
    rest = sizeof(buf) - strlen(buf);

    if (check_access(buf, CHECK_DIR) == ERROR) {
        if (mkdir(buf, DIR_MODE) == -1) {
            fglog("$WARNING: can't create dir %s", buf);
            return ERROR;
        }
        chmod(buf, DIR_MODE);
    }

    if (!aso) {
        /*
         * Point directory for point addresses
         */
        if (node->point > 0) {
            str_printf(base, rest, "/%04x%04x.pnt", node->net, node->node);
            if (check_access(buf, CHECK_DIR) == ERROR) {
                if (mkdir(buf, DIR_MODE) == -1) {
                    fglog("$WARNING: can't create dir %s", buf);
                    return ERROR;
                }
                chmod(buf, DIR_MODE);
            }
        }
    }
    return OK;
}

/*
 * Get file size
 */
long check_size(char *name)
{
    struct stat st;

    if (stat(name, &st) == -1)
        return ERROR;
    else
        return st.st_size;
}

/*
 * Check for old archive (m_time older than dt)
 */
int check_old(char *name, time_t dt)
{
    struct stat st;
    TIMEINFO ti;
    time_t t;

    GetTimeInfo(&ti);
    t = ti.time;

    if (stat(name, &st) == -1)
        return ERROR;

    return t - st.st_mtime > dt;
}
