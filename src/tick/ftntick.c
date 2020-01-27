/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Process incoming TIC files
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
#include "getopt.h"
#include <utime.h>

#define PROGRAM		"ftntick"
#define CONFIG		DEFAULT_CONFIG_MAIN

#define MY_AREASBBS	"FAreasBBS"
#define MY_CONTEXT	"ff"

#define MY_FILESBBS	"files.bbs"

static char *unknown_tick_area = NULL;  /* config.main: UnknownTickArea */

static char in_dir[MAXPATH];    /* Input directory */

static char *exec_script = NULL;    /* -x --exec option */

#ifdef FTN_ACL
static int uplink_can_be_readonly = FALSE;  /* config: UplinkCanBeReadonly */
#endif                          /* FTN_ACL */

int autocreate_check_pass = TRUE;
extern int authorized_new;
char *autocreate_line = "";     /* config: AutoCreateLine */
int t_flag = FALSE;
char *name;
static char *areas_bbs;
static mode_t tick_mode = 0600;
static char pass_path[MAXPATH] = { 0 };

#ifdef RECODE_FILE_DESC
static char *cs_def;
static char *cs_out;
#endif                          /* RECODE_FILE_DESC */

/*
 * Prototypes
 */
int do_tic(int);
int process_tic(Tick *);
int move(Tick *, char *, char *, char *);
int add_files_bbs(Tick *, char *);
int do_seenby(LON *, LON *, LON *, LON *, int);
int check_file(Tick *);

void short_usage(void);
void usage(void);
short check_pass(Tick *, short);
int filefix_check_forbidden_area(char *);
int cmd_new_int(Node *, char *, char *);
void areafix_init(int);
int areafix_auth_check(Node *, char *, char);
char *areafix_name(void);

short int files_change_mode = FALSE;
short int int_uplinks = FALSE;
short int kill_dupe = FALSE;
short int ignore_soft_cr = FALSE;

/*
 * Processs *.tic files in Binkley inbound directory
 */
int do_tic(int w_flag)
{
    Tick tic;
    char buf[MAXPATH];
    char pattern[16];
    int tic_wait = -1;
    struct stat st;
    time_t now;
    char *p;
    char bbslock[MAXPATH];

    /* Inicialize tick structure */
    tick_init(&tic);

    /* Set pattern dor searching tic files */
    BUF_COPY(pattern, "*.tic");

    dir_sortmode(DIR_SORTMTIME);
    if (dir_open(in_dir, pattern, TRUE) == ERROR) {
        fglog("$ERROR: can't open directory %s", in_dir);
        return ERROR;
    }

    for (name = dir_get(TRUE); name; name = dir_get(FALSE)) {
        debug(1, "ftntick: tick file %s", name);

        /* Read TIC file */
        if (tick_get(&tic, name) == ERROR) {
            fglog("$ERROR: reading %s", name);
            goto rename_to_bad;
        }

        /* Check file against Tick data */
        if (check_file(&tic) == -2) {
            if ((p = cf_get_string("TickWaitHour", TRUE))) {
                tic_wait = atoi(p);
            }
            now = time(NULL);
            if (stat(name, &st) == ERROR) {
                fglog("$ERROR: can't stat() file %s", buf);
                return EXIT_ERROR;
            }

            debug(4, "file: name=%s time=%ld now=%ld wait=%d",
                  tic.file, (long)st.st_mtime, (long)now,
                  tic_wait ? tic_wait * 3600 : tic_wait);

            if (tic_wait && (now - 3600 * tic_wait) < st.st_mtime)
                goto no_action;
            else {
                p = cf_get_string("TickWaitAction", TRUE);
                if (p == NULL) {
                    fglog("config: TickWaitAction not defined");
                    return EXIT_ERROR;
                }
                if (strcmp(p, "bad") == 0) {
                    debug(4, "rename to bad tic %s", name);
                    goto rename_to_bad;
                }
                if (strcmp(p, "delete") == 0) {
                    debug(4, "delete tic %s", name);
                    unlink(name);
                    continue;
                }
            }
        } else if (check_file(&tic) == ERROR)
            goto rename_to_bad;

        tick_debug(&tic, 3);

        fglog("area %s file %s (%lub) from %s", tic.area, tic.file, tic.size,
              znfp1(&tic.from));

        /*
         * Check dupe
         */
#ifdef TIC_HISTORY
        if (lock_program(cf_p_lock_history(), w_flag ? w_flag : NOWAIT) ==
            ERROR) {
            fglog("$ERROR: can't create lock file %s/%s", cf_p_lockdir(),
                  cf_p_lock_history());
            exit_free();
            exit(EXIT_BUSY);
        }

        hi_init_tic_history();
        sprintf(buffer, "%s %s %s %lx", tic.area, znfp1(&tic.origin), tic.file,
                tic.crc);
        if (hi_test(buffer)
            && (!tic.replaces || stricmp(tic.file, tic.replaces) != 0)) {
            hi_close();
            unlock_program(cf_p_lock_history());
            fglog("dupe from %s, crc=%lx, file=%s, area=%s, size=%lub",
                  znfp1(&tic.origin), tic.crc, tic.file, tic.area, tic.size);

            if (!kill_dupe)
                goto rename_to_bad;
            else {
                BUF_COPY(buffer, in_dir);
                BUF_APPEND(buffer, "/");
                BUF_APPEND(buffer, tic.file);
                unlink(buffer);
                p = strrchr(name, '/');
                fglog("delete %s, %s", tic.file, ++p);
                unlink(name);
                goto no_action;
            }
        }
        hi_close();
        unlock_program(cf_p_lock_history());
#endif                          /* TIC_HISTORY */

        BUF_COPY2(bbslock, areas_bbs, ".lock");
        if (lock_path(bbslock, w_flag ? w_flag : WAIT) == ERROR) {
            exit_free();
            exit(EXIT_BUSY);
        }

        if (process_tic(&tic) == ERROR) {
            areasbbs_rewrite();
            unlock_path(bbslock);
            fglog("%s: failed", name);
 rename_to_bad:
            /*
             * Error: rename .tic -> .bad
             */
            str_change_ext(buf, sizeof(buf), name, "bad");
            rename(name, buf);
            fglog("%s: renamed to %s", name, buf);
        } else {
/*
 * Write history information to database
 */
#ifdef TIC_HISTORY
            if (lock_program(cf_p_lock_history(), w_flag ? w_flag : WAIT) ==
                ERROR) {
                fglog("$ERROR: can't create lock file %s/%s", cf_p_lockdir(),
                      cf_p_lock_history());
                exit_free();
                exit(EXIT_BUSY);
            }

            hi_init_tic_history();
            sprintf(buffer, "%s %s %s %lx", tic.area, znfp1(&tic.origin),
                    tic.file, tic.crc);
            if (hi_write(tic.date, buffer) == ERROR)
                return ERROR;
            hi_close();
            unlock_program(cf_p_lock_history());
#endif                          /* TIC_HISTORY */

            areasbbs_rewrite();
            unlock_path(bbslock);
            /* Run -x script, if any */
            if (exec_script) {
                int ret;

                BUF_COPY3(buffer, exec_script, " ", name);

                debug(4, "Command: %s", buffer);
                ret = run_system(buffer);
                debug(4, "Exit code=%d", ret);
            }
            /* o.k., remove the TIC file */
            if (unlink(name) == ERROR)
                fglog("$ERROR: can't remove %s", name);
        }

 no_action:
        tmps_freeall();
    }

    dir_close();

    return OK;
}

/*
 * Process Tick
 */
int process_tic(Tick * tic)
{
    AreasBBS *bbs;
    LON new;
    LNode *p;
    char old_name[MAXPATH];
    char new_name[MAXPATH];
    int is_unknown = FALSE;
    Node from_node;

#ifdef FECHO_PASSTHROUGHT
    char full_farea_dir[MAXPATH];
#endif                          /* FECHO_PASSTHROUGHT */
    char *tmp = NULL;
    short create_flag = FALSE;
    char *s1;
    int errlvl;

    AreaUplink *a;
    /*
     * Lookup file area
     */
    if (!tic->area) {
        fglog("ERROR: missing area in %s", tic->file);
        return ERROR;
    }
    if ((bbs = areasbbs_lookup(tic->area)) == NULL) {
        from_node = tic->from;

        debug(5, "unknow area %s from %s", tic->area, znfp1(&tic->from));

        if (unknown_tick_area && (bbs = areasbbs_lookup(unknown_tick_area))) {
            is_unknown = TRUE;
            fglog("unknown area %s, using %s instead",
                  tic->area, unknown_tick_area);
        } else {
            if (!check_pass(tic, FALSE))
                return ERROR;
            if (autocreate_check_pass)
                if (!check_pass(tic, TRUE))
                    return ERROR;

            areafix_init(FALSE);
            areafix_auth_check(&tic->from, tic->pw, TRUE);

            if (!authorized_new) {
                fglog
                    ("node %s not authorized to create filearea %s (config restriction)",
                     znfp1(&tic->from), tic->area);
                return ERROR;
            }

            if (filefix_check_forbidden_area(tic->area)) {
                fglog("filearea %s is forbidden to create", tic->area);
                /* Unknown filearea */
                return ERROR;
            }
            if (!int_uplinks) {
                uplinks_init();
                int_uplinks = TRUE;
            }

            a = uplinks_line_get(FALSE, &tic->from);
            if (a != NULL && a->options != NULL) {
                tmp = (char *)xmalloc(strlen(tic->area)
                                      + strlen(autocreate_line)
                                      + strlen(a->options) + 3);
                sprintf(tmp, "%s %s %s", tic->area,
                        autocreate_line, a->options);
            } else {
                tmp = (char *)xmalloc(strlen(tic->area)
                                      + strlen(autocreate_line) + 2);
                sprintf(tmp, "%s %s", tic->area, autocreate_line);
            }

            errlvl = cmd_new_int(&tic->from, tmp, "-");
            if (tmp != NULL)
                xfree(tmp);
            if (errlvl == ERROR) {
                fglog
                    ("can't create filarea %s from %s (cmd_new() returned ERROR)",
                     tic->area, znfp1(&tic->from));
                return ERROR;
            }
            if (NULL == (bbs = areasbbs_lookup(tic->area))) {
                fglog
                    ("can't create filearea %s from %s (not found after creation)",
                     tic->area, znfp1(&tic->from));
                return ERROR;
            }
            fglog("New filearea %s from %s", tic->area, znfp1(&tic->from));
            create_flag = TRUE;
        }
    }
    cf_set_zone(bbs->zone);
#ifdef BEST_AKA
    if (bbs->nodes.first)
        cf_set_best(bbs->nodes.first->node.zone, bbs->nodes.first->node.net,
                    bbs->nodes.first->node.node);
#endif                          /* BEST_AKA */
    tic->to = cf_n_addr();

    if (!is_unknown) {
        /*
         * Check that sender is listed in FAreas.BBS
         */
        if (!lon_search(&bbs->nodes, &tic->from)) {
            fglog("insecure tic area %s from %s", tic->area, znfp1(&tic->from));
            return ERROR;
        }

#ifdef FTN_ACL
        if (uplink_can_be_readonly
            || !lon_is_uplink(&(bbs->nodes), bbs->uplinks, &(tic->from)))
            if (ftnacl_isreadonly(&tic->from, bbs->area, TYPE_FECHO)) {
                fglog("tic to read only area %s from %s", tic->area,
                      znfp1(&tic->from));
                return ERROR;
            }
#endif                          /* FTN_ACL */

        bbs->time = time(NULL);

        if ((NULL != bbs->state) && (areasbbs_isstate(bbs->state, 'U') ||
                                     areasbbs_isstate(bbs->state, 'W') ||
                                     areasbbs_isstate(bbs->state, 'F'))) {
            fglog("setting state 'S' for filearea %s", bbs->area);
            areasbbs_chstate(&(bbs->state), "UWF", 'S');
        }
        areasbbs_changed();

        /*
         * Replaces: move or delete old file
         */
        if (tic->replaces && !cf_get_string("TickReplacedIgnore", TRUE)) {
            char *xrdir = cf_get_string("TickReplacedDir", TRUE);

#ifdef FECHO_PASSTHROUGHT
            if (create_flag == TRUE)
                BUF_COPY3(old_name, full_farea_dir, "/", tic->replaces);
            else
                BUF_COPY3(old_name, bbs->dir, "/", tic->replaces);
            if (check_access(old_name, CHECK_FILE) == TRUE &&
                !(bbs->flags & AREASBBS_PASSTHRU))
#else
            BUF_COPY3(old_name, bbs->dir, "/", tic->replaces);
            if (check_access(old_name, CHECK_FILE) == TRUE)
#endif                          /* FECHO_PASSTHROUGHT */
            {
                if (xrdir) {
                    char rdir[MAXPATH];
                    BUF_EXPAND(rdir, xrdir);
                    /* Copy to ReplacedFilesDir */
                    BUF_COPY3(new_name, rdir, "/", tic->replaces);
                    debug(1, "%s -> %s", old_name, new_name);
                    if (copy_file(old_name, new_name, rdir) == ERROR) {
                        fglog("$ERROR: can't copy %s -> %s", old_name,
                              new_name);
                        return ERROR;
                    }
                    fglog("area %s file %s replaces %s, moved to %s",
                          tic->area, tic->file, tic->replaces, rdir);
                } else
                    fglog("area %s file %s replaces %s, removed",
                          tic->area, tic->file, tic->replaces);

                /* Remove old file, no error if this fails */
                unlink(old_name);

                /* Remove old file from FILES.BBS */
        /**FIXME**/
            }
        }
    }

    /*
     * Move file from inbound to file area, add description to FILES.BBS
     */
#ifdef FECHO_PASSTHROUGHT
    if (!(bbs->flags & AREASBBS_PASSTHRU)) {
        BUF_COPY3(old_name, in_dir, "/", tic->file);
        BUF_COPY3(new_name, bbs->dir, "/", tic->file);
        debug(9, "%s -> %s", old_name, new_name);
        if (check_access(bbs->dir, CHECK_DIR) == ERROR) {
            if (mkdir_r(bbs->dir, FILE_DIR_MODE) == -1)
                return ERROR;
        }
        if (move(tic, old_name, new_name, bbs->dir) == ERROR) {
            return ERROR;
        }
        if (files_change_mode)
            chmod(new_name, files_change_mode);
        add_files_bbs(tic, bbs->dir);
    } else
        BUF_COPY3(new_name, in_dir, "/", tic->file);
#else
    BUF_COPY3(old_name, in_dir, "/", tic->file);
    BUF_COPY3(new_name, bbs->dir, "/", tic->file);
    debug(1, "%s -> %s", old_name, new_name);
    if (check_access(bbs->dir, CHECK_DIR) == ERROR) {
        if (mkdir_r(bbs->dir, FILE_DIR_MODE) == -1)
            return ERROR;
    }
    if (move(tic, old_name, new_name, bbs->dir) == ERROR)
        return ERROR;
    if (files_change_mode)
        chmod(new_name, files_change_mode);
    add_files_bbs(tic, bbs->dir);
#endif                          /* FECHO_PASSTHROUGHT */
    if (!is_unknown) {
        int uplink = 1;

        /*
         * Add us to Path list
         */
        tick_add_path(tic);

        /*
         * Add sender to SEEN-BY if not already there
         */
        if (!lon_search(&tic->seenby, &tic->from))
            lon_add(&tic->seenby, &tic->from);

        /*
         * We're the sender
         */
        tic->from = cf_n_addr();

        if (!lon_search(&tic->seenby, &tic->from))
            lon_add(&tic->seenby, &tic->from);

        /*
         * Add nodes not already in SEEN-BY to seenby and new.
         */
        lon_init(&new);
        do_seenby(&tic->seenby, &bbs->nodes, &new, &(bbs->passive),
                  bbs->uplinks);
        if (new.size > 0)
            lon_debug(3, "Send to new nodes: ", &new, TRUE);
        else
            debug(3, "area havn't downlinks");

        /*
         * Send file to all nodes in LON new
         */
        BUF_COPY(old_name, new_name);

        if (!(new.size > 0) && create_flag == TRUE &&
            bbs->flags & AREASBBS_PASSTHRU) {
            Textlist req;
            char *fix_name = NULL;

            tl_init(&req);

            areafix_init(FALSE);

            if (!authorized_new)
                areafix_auth_check(&from_node, tic->pw, TRUE);

            if (!authorized_new) {
                fglog
                    ("node %s not authorized to delete filearea %s (config restriction)",
                     znfp1(&from_node), tic->area);
                return ERROR;
            }

            if ((a = uplinks_line_get(FALSE, &from_node))) {
                fglog("unsubscribe from area %s (no downlinks)", tic->area);
                fix_name = cf_get_string("FileFixName", TRUE);

                tl_appendf(&req, "%s,%s,%s,%s,-%s",
                           znfp1(&a->uplink), a->robotname,
                           fix_name ? fix_name : areafix_name(),
                           a->password, str_upper(tic->area));
                send_request(&req);
            } else
                fglog
                    ("uplink entry for area %s(%s) not found, unsubscribe failed",
                     tic->area, znfp1(&from_node));

            unlink(old_name);
            fglog("delete %s", old_name);
            areasbbs_not_changed();
            return OK;
        }

        if (pass_path[0] == '\0') {
            fglog("ERROR: config: PassthroughtBoxesDir not defined");
            return ERROR;
        }
        BUF_COPY3(new_name, pass_path, "/", tic->file);
        if ((errlvl = link(old_name, new_name)) == -1) {
            debug(9, "can't link %s -> %s %s", old_name, new_name,
                  strerror(errno));
            if (copy_file(old_name, new_name, pass_path) == ERROR) {
                fglog("$ERROR: can't copy %s -> %s %s", old_name, new_name,
                      strerror(errno));
                return ERROR;
            }
        }
        debug(9, "link %s -> %s", old_name, new_name);

        for (p = new.first; p; p = p->next, uplink++) {

#ifdef FECHO_PASSTHROUGHT
            if (tick_send(tic, &p->node, new_name, 1, tick_mode,
                          pass_path) == ERROR)
#else
            if (tick_send(tic, &p->node, new_name, tick_mode) == ERROR)
#endif                          /* FECHO_PASSTHROUGHT */
            {
                fglog("ERROR: send area %s file %s to %s failed",
                      tic->area, tic->file, znfp1(&p->node));
                return ERROR;
            }
        }
        debug(9, "unlink %s", new_name);
        unlink(new_name);
        if (bbs->flags & AREASBBS_PASSTHRU) {
            debug(9, "unlink %s", old_name);
            unlink(old_name);
        }
        /*
         * TickFileAction
         */
        sprintf(buffer, "TickFileAction");

        for (s1 = cf_get_string(buffer, TRUE); s1 && *s1;
             s1 = cf_get_string(buffer, FALSE)) {
            int ret;
            char *str_save;
            char *area;
            char *wild_file;
            char *tick_action;
            debug(8, "config: TickFileAction %s", s1);
            str_save = strsave(s1);
            area = xstrtok(str_save, " \t");
            if (!stricmp(tic->area, area)) {
                wild_file = xstrtok(NULL, " \t");
                tick_action = xstrtok(NULL, "\n");
                if (wildmatch(tic->file, wild_file, TRUE)) {
                    sprintf(buffer, tick_action, new_name);
                    debug(8, "exec: %s", buffer);
                    ret = run_system(buffer);
                    if (ret)
                        fglog("exec: %s failed, exit code = %d", buffer, ret);
                    else
                        fglog("exec: %s complete", buffer);
                    xfree(str_save);
                    break;
                }
            }
            xfree(str_save);
        }
#ifdef FECHO_PASSTHROUGHT
        if (bbs->flags & AREASBBS_PASSTHRU)
            unlink(new_name);
#endif                          /* FECHO_PASSTHROUGHT */
    }

    return OK;
}

/*
 * Check password
 */
short check_pass(Tick * tic, short mode)
{

    Passwd *pwd;
    char *passwd;

    /*
     * Get password for from node
     */
    if ((pwd = passwd_lookup(MY_CONTEXT, &tic->from)))
        passwd = pwd->passwd;
    else
        passwd = NULL;
    if (passwd)
        debug(3, "ftntick: password %s", passwd);

    if (!mode)
        if (passwd || t_flag)
            return TRUE;

    /*
     * Require password unless -t option is given
     */
    if (!t_flag && !passwd) {
        fglog("%s: no password for %s in PASSWD", name, znfp1(&tic->from));
        return FALSE;
    }

    /*
     * Check password
     */
    if (passwd) {
        if (tic->pw) {
            if (stricmp(passwd, tic->pw)) {
                fglog("%s: wrong password from %s: ours=%s his=%s",
                      name, znfp1(&tic->from), passwd, tic->pw);
                return FALSE;
            }
        } else {
            fglog("%s: no password from %s: ours=%s", name,
                  znfp1(&tic->from), passwd);
            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Move file (copy then unlink)
 */
int move(Tick * tic, char *old, char *new, char *dir)
{
#ifndef FTNTICK_NOCRC
    unsigned long crc;
#endif                          /* FTNTICK_NOCRC */
    struct utimbuf ut;

    /* Copy */
    if (copy_file(old, new, dir) == ERROR) {
        fglog("$ERROR: can't copy %s -> %s", old, new);
        return ERROR;
    }

#ifndef FTNTICK_NOCRC
    /* Compute CRC again to be sure */
    crc = crc32_file(new);
    if (crc != tic->crc) {
        fglog("ERROR: error while copying to %s, wrong CRC", new);
        unlink(new);
        return ERROR;
    }
#endif                          /* FTNTICK_NOCRC */

    /* o.k., now unlink file in inbound */
    if (unlink(old) == ERROR) {
        fglog("$ERROR: can't remove %s", old);
        return ERROR;
    }

    /* Set a/mtime to time from TIC */
    if ((tic->date != -1) && (NULL == cf_get_string("TickDontSetTime", TRUE))) {
        ut.actime = ut.modtime = tic->date;
        if (utime(new, &ut) == ERROR) {
#ifndef __CYGWIN32__            /* Some problems with utime() here */
            fglog("$WARNING: can't set time of %s", new);
#endif
#if 0
            return ERROR;
#endif
        }
    }

    return OK;
}

/*
 * Add description to FILES.BBS
 */
int add_files_bbs(Tick * tic, char *dir)
{
#ifdef RECODE_FILE_DESC
    char buf1[BUFSIZ];
    char buf[BUFSIZ];
    char *p;
#endif                          /* RECODE_FILE_DESC */
    char files_bbs[MAXPATH];
#ifdef DESC_DIR
    char desc_dir[MAXPATH];
#endif                          /* DESC_DIR */
    FILE *fp;

#ifdef DESC_DIR
    BUF_COPY4(files_bbs, dir, "/" DESC_DIR "/", tic->file, ".desc");
    BUF_COPY2(desc_dir, dir, "/" DESC_DIR);
    if (mkdir_r(desc_dir, DIR_MODE) == ERROR) {
        fglog("$ERROR: can't create desc dir %s", desc_dir);
        return ERROR;
    }
    if ((fp = fopen(files_bbs, W_MODE)) == NULL)
#else
    BUF_COPY3(files_bbs, dir, "/", MY_FILESBBS);
    if ((fp = fopen(files_bbs, A_MODE)) == NULL)
#endif                          /* DESC_DIR */
    {
        fglog("$ERROR: can't append to %s", files_bbs);
        return ERROR;
    }

#ifdef RECODE_FILE_DESC
    if (tic->desc.first) {
        size_t srclen;
        size_t dstlen;

        BUF_COPY(buf1, tic->desc.first->line);
        msg_xlate_line(buf1, sizeof(buf1), tic->desc.first->line,
                       ignore_soft_cr);

        srclen = strlen(buf1) + 1;
        charset_recode_buf(&p, &dstlen, buf1, srclen, cs_def, cs_out);
        BUF_COPY(buf, p);
    } else {
        BUF_COPY(buf, "--no description--");
    }
#endif                          /* RECODE_FILE_DESC */
#ifdef DESC_DIR
#ifndef RECODE_FILE_DESC
    fprintf(fp, "%s\n",
            tic->desc.first ? tic->desc.first->line : "--no description--");
#else
    fprintf(fp, "%s\n", buf);
#endif                          /* RECODE_FILE_DESC */
#else
#ifndef RECODE_FILE_DESC
    fprintf(fp, "%-12s  %s\n", tic->file,
            tic->desc.first ? tic->desc.first->line : "--no description--");
#else
    fprintf(fp, "%-12s  %s\n", tic->file, buf);
#endif                          /* RECODE_FILE_DESC */
#endif                          /* DESC_DIR */

    fclose(fp);

    if (files_change_mode)
        chmod(files_bbs, files_change_mode);

    return OK;
}

/*
 * Add nodes to SEEN-BY (4D)
 */
int do_seenby(LON * seenby, LON * nodes, LON * new, LON * passive, int bbsupl)
{
    LNode *p;
    int uplinks = 1;

    for (p = nodes->first; p; p = p->next, uplinks++) {
        if (bbsupl >= uplinks)
            continue;

        if (lon_search(passive, &(p->node)))
            continue;
        if (!lon_search(seenby, &p->node)) {
            lon_add(seenby, &p->node);
            if (new)
                lon_add(new, &p->node);
        }
    }

    return OK;
}

/*
 * Check file
 */
int check_file(Tick * tic)
{
    struct stat st;
#ifndef FTNTICK_NOCRC
    unsigned long crc;
#endif                          /* FTNTICK_NOCRC */
    char name[MAXPATH];
    char orig[MAXPATH];

    if (!tic->file) {
        fglog("ERROR: no file name");
        return ERROR;
    }

    /* Search file */
    if (dir_search(in_dir, tic->file) == NULL) {
        debug(4, "ERROR: can't find file %s", tic->file);
        return -2;
    }

    /* Full path name */
    BUF_COPY3(orig, in_dir, "/", tic->file);
    str_lower(tic->file);
    BUF_COPY3(name, in_dir, "/", tic->file);
    if (strcmp(name, orig)) {
        if (rename(orig, name))
            fglog("$ERROR: can't rename() file %s to lower case", name);
    }
    if (stat(name, &st) == ERROR) {
        fglog("$ERROR: can't stat() file %s", name);
        return ERROR;
    }

    /*
     * File size
     */
    if (tic->size) {
        if (tic->size != st.st_size) {
            fglog("ERROR: wrong size for file %s: got %lu, expected %lu",
                  name, (long)st.st_size, tic->size);
            return ERROR;
        }
    } else
        tic->size = st.st_size;

    /*
     * File date
     */
    if (tic->date == -1)
        tic->date = st.st_mtime;

#ifndef FTNTICK_NOCRC
    /*
     * File CRC
     */
    crc = crc32_file(name);
    if (tic->crc == 0 && crc != 0)
        tic->crc = crc;
    else {
        if (tic->crc != crc) {
            fglog("ERROR: wrong CRC for file %s: got %08lx, expected %08lx",
                  name, crc, tic->crc);
            return ERROR;
        }
    }

#endif                          /* FTNTICK_NOCRC */

    return OK;
}

/*
 * Usage messages
 */
void short_usage(void)
{
    fprintf(stderr, "usage: %s [-options]\n", PROGRAM);
    fprintf(stderr, "       %s --help  for more information\n", PROGRAM);
    exit(EX_USAGE);
}

void usage(void)
{
    fprintf(stderr, "FIDOGATE %s  %s %s\n\n",
            version_global(), PROGRAM, version_local(VERSION));

    fprintf(stderr, "usage:   %s [-options]\n\n", PROGRAM);
    fprintf(stderr, "\
options:  -b --fareas-bbs NAME         use alternate FAREAS.BBS\n\
          -I --inbound DIR             set inbound dir (default: PINBOUND)\n\
          -t --insecure                process TIC files without password\n\
          -x --exec SCRIPT             exec script for incoming TICs,\n\
                                       called as SCRIPT FILE.TIC\n\
\n\
          -v --verbose                 more verbose\n\
	  -h --help                    this help\n\
          -c --config name             read config file (\"\" = none)\n\
	  -a --addr Z:N/F.P            set FTN address\n\
	  -u --uplink-addr Z:N/F.P     set FTN uplink address\n\
          -w --wait [TIME]             wait for areas.bbs lock to be released\n");

    exit(0);
}

/***** main() ****************************************************************/

int main(int argc, char **argv)
{
    char *p;
    int c;
    char *I_flag = NULL;
    char *c_flag = NULL;
    char *a_flag = NULL, *u_flag = NULL;
    int w_flag = FALSE;

    int option_index;
    static struct option long_options[] = {
        {"fareas-bbs", 1, 0, 'b'},
        {"insecure", 0, 0, 't'},    /* Insecure */
        {"inbound", 1, 0, 'I'}, /* Set tick inbound */
        {"exec", 1, 0, 'x'},    /* Run script */

        {"verbose", 0, 0, 'v'}, /* More verbose */
        {"help", 0, 0, 'h'},    /* Help */
        {"config", 1, 0, 'c'},  /* Config file */
        {"addr", 1, 0, 'a'},    /* Set FIDO address */
        {"uplink-addr", 1, 0, 'u'}, /* Set FIDO uplink address */
        {"wait", 1, 0, 'w'},
        {0, 0, 0, 0}
    };

    /* Set log and debug output */
    log_program(PROGRAM);

    /* Init configuration */
    cf_initialize();

    /* Reading command line options */
    while ((c = getopt_long(argc, argv, "b:tI:x:vhc:w:a:u:",
                            long_options, &option_index)) != EOF)
        switch (c) {
    /***** ftntick options *****/
        case 'b':
            areas_bbs = optarg;
            break;
        case 't':
            t_flag = TRUE;
            break;
        case 'I':
            I_flag = optarg;
            break;
        case 'x':
            exec_script = optarg;
            break;

        /***** Common options *****/
        case 'v':
            verbose++;
            break;
        case 'h':
            usage();
            break;
        case 'c':
            c_flag = optarg;
            break;
        case 'a':
            a_flag = optarg;
            break;
        case 'u':
            u_flag = optarg;
            break;
        case 'w':
            if (optarg)
                w_flag = atoi(optarg);
            else
                w_flag = WAIT;
            break;
        default:
            short_usage();
            break;
        }

    /*
     * Read config file (make hash)
     */
    cf_read_config_file(c_flag ? c_flag : CONFIG);

    /*
     * Process config options (set our address and uplink address ifs it set)
     */
    if (a_flag)
        cf_set_addr(a_flag);
    if (u_flag)
        cf_set_uplink(u_flag);

    /* printing debug log information if verboselevel >= 8 */
    cf_debug();

    /* reading/init routing file */
    routing_init(cf_p_routing());   /* SNP:FIXME: add `-r' command-line option? */

    /* Set inbound directory */
    BUF_EXPAND(in_dir, I_flag ? I_flag : cf_p_pinbound());

    /*
     * Process optional config statements
     */
    if ((p = cf_get_string("UnknownTickArea", TRUE))) {
        unknown_tick_area = p;
    }
    if (cf_get_string("AutoCreateFileechoDontCheckPassword", TRUE)) {
        autocreate_check_pass = FALSE;
    }
    if ((p = cf_get_string("AutoCreateFileechoLine", TRUE))) {
        autocreate_line = p;
    }
#ifdef FTN_ACL
    if (cf_get_string("UplinkCanBeReadonly", TRUE)) {
        uplink_can_be_readonly = TRUE;
    }
#endif                          /* FTN_ACL */
    if ((p = cf_get_string("PassthroughtBoxesDir", TRUE))) {
        BUF_EXPAND(pass_path, p);
    }

    if ((p = cf_get_string("FileEchoFilesModeChange", TRUE))) {
        files_change_mode = atooct(p) & 0777;
    }
    if ((p = cf_get_string("TickMode", TRUE))) {
        tick_mode = atooct(p) & 0777;
    } else {
        tick_mode = PACKET_MODE;
    }
    if (cf_get_string("DontIgnoreSoftCR", TRUE)) {
        ignore_soft_cr = TRUE;
    }
    if (cf_get_string("DontIgnore0x8d", TRUE)) {
        ignore_soft_cr = FALSE;
    }

    /*
     * Get name of fareas.bbs file from config file
     */
    if (areas_bbs == NULL)
        areas_bbs = cf_get_string(MY_AREASBBS, TRUE);

    if (areas_bbs == NULL) {
        fprintf(stderr, "%s: no areas.bbs specified\n", PROGRAM);
        exit_free();
        exit(EX_USAGE);
    }
#ifdef RECODE_FILE_DESC
    if ((p = cf_get_string("DefaultCharset", TRUE))) {
        debug(8, "config: DefaultCharset %s", p);
        cs_def = strtok(p, ":");
        strtok(NULL, ":");
        cs_out = strtok(NULL, ":");

        charset_init();
        charset_set_in_out(cs_def, cs_out);
    }
#endif                          /* RECODE_FILE_DESC */

    if (cf_get_string("KillTickDupe", TRUE)) {
        debug(8, "config: KillTickDupe");
        kill_dupe = TRUE;
    }

    /* Read PASSWD */
    passwd_init();
    /* Read FAreas.BBS */
    if (areasbbs_init(areas_bbs) == ERROR) {
        fglog("$ERROR: can't open %s", areas_bbs);
        exit_free();
        return EXIT_ERROR;
    }
#ifdef FTN_ACL
    /* Read ACL */
    ftnacl_init();
#endif                          /* FTN_ACL */

    /* Set lockfile in lock directoy (with wait for it released) */
    if (lock_program(PROGRAM, NOWAIT) == ERROR) {
        /* exit if released time is out or it not set */
        exit_free();
        exit(EXIT_BUSY);
    }

    /* Search tick files and processing it */
    do_tic(w_flag);

    /* removing lock file */
    unlock_program(PROGRAM);

    exit_free();
    exit(0);
}
