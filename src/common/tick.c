/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * TIC file processing
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

#define MY_CONTEXT	"ff"

/*
 * Init Tick struct
 */
void tick_init(Tick * tic)
{
    node_invalid(&tic->origin);
    node_invalid(&tic->from);
    node_invalid(&tic->to);
    tic->area = NULL;
    tic->file = NULL;
    tic->replaces = NULL;
    tl_init(&tic->desc);
    tl_init(&tic->ldesc);
    tic->crc = 0;
    tic->created = NULL;
    tic->size = 0;
    tl_init(&tic->path);
    lon_init(&tic->seenby);
    tic->pw = 0;
    tic->release = -1;
    tic->date = -1;
    tl_init(&tic->app);
}

/*
 * Delete Tick struct
 */
void tick_delete(Tick * tic)
{
    node_invalid(&tic->origin);
    node_invalid(&tic->from);
    node_invalid(&tic->to);
    xfree(tic->area);
    tic->area = NULL;
    xfree(tic->file);
    tic->file = NULL;
    xfree(tic->replaces);
    tic->replaces = NULL;
    tl_clear(&tic->desc);
    tl_clear(&tic->ldesc);
    tic->crc = 0;
    xfree(tic->created);
    tic->created = NULL;
    tic->size = 0;
    tl_clear(&tic->path);
    lon_delete(&tic->seenby);
    xfree(tic->pw);
    tic->pw = NULL;
    tic->release = -1;
    tic->date = -1;
    tl_clear(&tic->app);
}

/*
 * Write TIC file
 */
int tick_put(Tick * tic, char *name, mode_t mode)
{
    FILE *fp;
    LNode *p;
    Textline *pl;

    if (!creat(name, mode)) {
        fglog("ERROR: can't create file %s (%s)", name, strerror(errno));
        return ERROR;
    }

    if ((fp = fopen(name, W_MODE)) == NULL)
        return ERROR;

//    if(chmod(name, mode) == -1)
//  fglog("WARNING: can't change mode for file %s (%s)", name, strerror(errno));

    fprintf(fp, "Area %s\r\n", tic->area);
    fprintf(fp, "Origin %s\r\n", znf1(&tic->origin));
    fprintf(fp, "From %s\r\n", znf1(&tic->from));
    fprintf(fp, "File %s\r\n", tic->file);
    if (tic->replaces)
        fprintf(fp, "Replaces %s\r\n", tic->file);
    fprintf(fp, "Desc %s\r\n", tic->desc.first ? tic->desc.first->line : "");
    if (tic->ldesc.first)
        fprintf(fp, "LDesc %s\r\n", tic->desc.first->line);
    fprintf(fp, "CRC %08lX\r\n", tic->crc);
    fprintf(fp, "Created %s\r\n", tic->created);
    fprintf(fp, "Size %lu\r\n", tic->size);
    fprintf(fp, "Date %ld\r\n", (long)tic->date);
    for (pl = tic->path.first; pl; pl = pl->next)
        fprintf(fp, "Path %s\r\n", pl->line);
    for (p = tic->seenby.first; p; p = p->next)
        fprintf(fp, "Seenby %s\r\n", znf1(&p->node));
    fprintf(fp, "Pw %s\r\n", tic->pw);

    return fclose(fp);
}

/*
 * Read TIC file
 */
int tick_get(Tick * tic, char *name)
{
    FILE *fp;
    char *key, *arg;
    Node node;

    tick_delete(tic);

    if ((fp = fopen(name, R_MODE)) == NULL) {
        fglog("$WARNING: can't open file %s for reading (%s)", name,
              strerror(errno));
        return ERROR;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        strip_crlf(buffer);

        key = strtok(buffer, " \t");    /* Keyword */
        arg = strtok(NULL, ""); /* Arg(s) */

        if (!key)
            continue;
        if (!arg)
            arg = "";

        if (!stricmp(key, "Origin")) {
            if (asc_to_node(arg, &node, FALSE) == OK)
                tic->origin = node;
        }

        if (!stricmp(key, "From")) {
            if (asc_to_node(arg, &node, FALSE) == OK)
                tic->from = node;
        }

        if (!stricmp(key, "Area")) {
            tic->area = strsave(arg);
            str_upper(tic->area);
        }

        if (!stricmp(key, "File")) {
            tic->file = strsave(arg);
            str_lower(tic->file);
        }

        if (!stricmp(key, "Replaces")) {
            tic->replaces = strsave(arg);
            str_lower(tic->replaces);
        }

        if (!stricmp(key, "Desc")) {
            if (!*arg)
                arg = "--no description--";
            tl_append(&tic->desc, arg);
        }

        if (!stricmp(key, "LDesc")) {
            tl_append(&tic->ldesc, arg);
        }

        if (!stricmp(key, "CRC")) {
            sscanf(arg, "%lx", &tic->crc);
        }

        if (!stricmp(key, "Created")) {
            tic->created = strsave(arg);
        }

        if (!stricmp(key, "Size")) {
            tic->size = atol(arg);
        }

        if (!stricmp(key, "Path")) {
            tl_append(&tic->path, arg);
        }

        if (!stricmp(key, "Seenby")) {
            lon_add_string(&tic->seenby, arg);
        }

        if (!stricmp(key, "Pw")) {
            tic->pw = strsave(arg);
        }

        if (!stricmp(key, "Release")) {
            tic->release = atol(arg);
        }

        if (!stricmp(key, "Date")) {
            tic->date = atol(arg);
        }

        if (!stricmp(key, "App")) {
            tl_append(&tic->app, arg);
        }
    }

    fclose(fp);
    return OK;
}

/*
 * Debug output of Tick struct
 */
void tick_debug(Tick * tic, int lvl)
{
    Textline *pl;
    LNode *p;

    debug(lvl, "Origin 	 : %s", znfp1(&tic->origin));
    debug(lvl, "From   	 : %s", znfp1(&tic->from));
    debug(lvl, "To     	 : %s", znfp1(&tic->to));
    debug(lvl, "Area   	 : %s", tic->area);
    debug(lvl, "File   	 : %s", tic->file);
    debug(lvl, "Replaces : %s", tic->replaces ? tic->replaces : "-NONE-");
    for (pl = tic->desc.first; pl; pl = pl->next)
        debug(lvl, "Desc     : %s", pl->line);
    for (pl = tic->ldesc.first; pl; pl = pl->next)
        debug(lvl, "LDesc    : %s", pl->line);
    debug(lvl, "CRC    	 : %08lX", tic->crc);
    debug(lvl, "Created	 : %s", tic->created);
    debug(lvl, "Size   	 : %lu", tic->size);
    for (pl = tic->path.first; pl; pl = pl->next)
        debug(lvl, "Path     : %s", pl->line);
    for (p = tic->seenby.first; p; p = p->next)
        debug(lvl, "Seenby   : %s", znfp1(&p->node));
    debug(lvl, "Pw       : %s", tic->pw);
    debug(lvl, "Release  : %ld", (long)tic->release);
    debug(lvl, "Date     : %ld", (long)tic->date);
    for (pl = tic->app.first; pl; pl = pl->next)
        debug(lvl, "App      : %s", pl->line);
}

/*
 * Send file and TIC to node
 */
#ifdef FECHO_PASSTHROUGHT
int tick_send(Tick * tic, Node * node, char *name, int type,
              mode_t mode, char *pass_path)
#else
int tick_send(Tick * tic, Node * node, char *name, mode_t mode)
#endif                          /* FECHO_PASSTHROUGHT */
{
    Passwd *pwd;
    char *pw = "";
    static char *flav_dfl = NULL;
    char *flav;
    Routing *r;
    LNode *p;
#ifdef FECHO_PASSTHROUGHT
    int errlvl;
#endif
    if (!flav_dfl) {
        flav_dfl = cf_get_string("TickFlav", TRUE);
        if (flav_dfl == NULL)
            flav_dfl = "Normal";
    }

    flav = flav_dfl;

    for (r = routing_first; r; r = r->next)
        if (TYPE_TICK == r->type && CMD_SEND == r->cmd)
            for (p = r->nodes.first; p; p = p->next)
                if (node_match(node, &(p->node))) {
                    if (FLAV_NORMAL == r->flav)
                        flav = "Normal";
                    else if (FLAV_HOLD == r->flav)
                        flav = "Hold";
                    else if (FLAV_CRASH == r->flav)
                        flav = "Crash";
                    else if (FLAV_DIRECT == r->flav)
                        flav = "Direct";
                    goto ready;
                }
 ready:

    /*
     * Attach file
     */
    debug(4, "attach %s (%s)", name, flav);

#ifndef FECHO_PASSTHROUGHT
    if (bink_attach(node, 0, name, flav, TRUE) == ERROR) {
        fglog("$WARNING: can't attach file %s for %s", name, znfp1(node));
        return ERROR;
    }
#else
    if (type == 1) {
        if (!pass_path) {
            fglog("$ERROR: config: PassthroughtBoxesDir not defined");
            return ERROR;
        }

        str_printf(buffer, sizeof(buffer), "%s/%d.%d.%d.%d",
                   pass_path, node->zone, node->net, node->node, node->point);

        if (mkdir_r(buffer, DIR_MODE) == ERROR) {
            fglog("$WARNING: can't create dir %s", buffer);
            return ERROR;
        }
        BUF_APPEND2(buffer, "/", tic->file);
        debug(4, "link file %s -> %s", name, buffer);

        /* Create hardlink */
        errlvl = link(name, buffer);
        /* if fault... */
        if (errlvl == -1) {
            /* diffirent partition */
            if (errno == EXDEV) {
                copy_file(name, buffer, "dir fault!");
            }
            /* file exist */
            else if (errno == EEXIST) {
#ifndef OVERWRITEN_FECHO_FILE_TO_LINK
                fglog("$ERROR: can't link file %s -> %s (%s)", name, buffer,
                      strerror(errno));
                return ERROR;
#else
                unlink(buffer);
                errlvl = link(name, buffer);
                if (errlvl == -1) {
                    fglog("$ERROR: after unlink cannot link file %s -> %s",
                          name, buffer);
                    return ERROR;
                }
#endif                          /* OVERWRITEN_FECHO_FILE_TO_LINK */
            } else {
                fglog("$ERROR: can't link file %s -> %s, %s", name, buffer,
                      strerror(errno));
                return ERROR;
            }
        }
#ifndef USE_FILEBOX
        if (bink_attach(node, '^', buffer, flav, TRUE) == ERROR)
            return ERROR;
#endif                          /* USE_FILEBOX */
    } else {
        if (bink_attach(node, 0, name, flav, TRUE) == ERROR)
            return ERROR;
    }
#endif                          /* !FECHO_PASSTHROUGHT */

    /*
     * Get password
     */
    if ((pwd = passwd_lookup(MY_CONTEXT, node)))
        pw = pwd->passwd;
    debug(4, "passwd: %s", pwd ? pwd->passwd : "-NONE-");

    tic->to = *node;
    tic->pw = strsave(pw);

    /*
     * Make sure tick dir exists
     */
#if !defined(USE_FILEBOX) || !defined(FECHO_PASSTHROUGHT)
    BUF_EXPAND(buffer, cf_p_tick_hold());
    if (mkdir_r(buffer, DIR_MODE) == -1)
        return ERROR;
#else
    if (!pass_path) {
        fglog("$ERROR: config: PassthroughtBoxesDir not defined");
        return ERROR;
    }

    str_printf(buffer, sizeof(buffer), "%s/%d.%d.%d.%d",
               pass_path, node->zone, node->net, node->node, node->point);
    if (mkdir_r(buffer, DIR_MODE) == ERROR) {
        fglog("$ERROR: can't create dir %s", buffer);
        return ERROR;
    }
#endif                          /* USE_FILEBOX */
    /*
     * Create TIC
     */
    str_printf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer),
               "/tk%06ld.tic", sequencer(cf_p_seq_tick()) % 1000000);
    debug(4, "creating %s", buffer);
    if (tick_put(tic, buffer, mode) == ERROR) {
        fglog("ERROR: tick_put() return -1");
        return ERROR;
    }

#ifndef USE_FILEBOX
    /*
     * Attach TIC
     */
    debug(4, "attach %s (%s)", buffer, flav);
    if (bink_attach(node, '^', buffer, flav, TRUE) == ERROR)
        return ERROR;
#endif                          /* USE_FILEBOX */

    fglog("area %s file %s (%lub) to %s",
          tic->area, tic->file, tic->size, znfp1(node));

    return OK;
}

/*
 * Add Path line with our address
 */
void tick_add_path(Tick * tic)
{
    time_t now;

    now = time(NULL);
    tl_appendf(&tic->path, "%s %ld %s",
               znf1(cf_addr()), (long)now, date(DATE_TICK_PATH, &now));

}

/*
 * Copy file
 */
int copy_file(char *old, char *new, char *dir)
{
    FILE *fold, *fnew;
    int nr, nw;

    /* Open */
    if ((fold = fopen(old, R_MODE)) == NULL) {
        return ERROR;
    }
    if ((fnew = fopen(new, W_MODE)) == NULL) {
        if (errno == ENOENT) {
            mkdir_r(dir, DIR_MODE);
        }
        if ((fnew = fopen(new, W_MODE)) == NULL) {
            fclose(fold);
            return ERROR;
        }
    }

    /* Copy */
    do {
        nr = fread(buffer, sizeof(char), sizeof(buffer), fold);
        if (ferror(fold)) {
            fglog("$ERROR: can't read from %s", old);
            fclose(fold);
            fclose(fnew);
            unlink(new);
            return ERROR;
        }

        nw = fwrite(buffer, sizeof(char), nr, fnew);
        if (nw != nr) {
            fglog("$ERROR: can't write to %s", new);
            fclose(fold);
            fclose(fnew);
            unlink(new);
            return ERROR;
        }
    }
    while (!feof(fold));

    /* Close */
    fclose(fold);
    fclose(fnew);

    return OK;
}
