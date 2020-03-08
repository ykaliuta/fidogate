/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * Common Areafix functions
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

#define MY_NAME_AF	"Areafix Daemon"
#define MY_CONTEXT_AF	"af"
#define MY_AREASBBS_AF	"AreasBBS"
#ifdef FTN_ACL
#define MY_TYPE_AF	TYPE_ECHO
#endif                          /* FTN_ACL */

#define MY_NAME_FF	"Filefix Daemon"
#define MY_CONTEXT_FF	"ff"
#define MY_AREASBBS_FF	"FAreasBBS"
#ifdef FTN_ACL
#define MY_TYPE_FF	TYPE_FECHO
#endif                          /* FTN_ACL */

#define MY_NAME		my_name
#define MY_CONTEXT	my_context
#ifdef FTN_ACL
#define MY_TYPE	my_type
#endif                          /* FTN_ACL */

#ifdef SUBSCRIBE_ZONEGATE
void zonegate_init(void);
#endif                          /* SUBSCRIBE_ZONEGATE */
void areafix_tlprintf(const char *, ...);
void areafix_stdprintf(const char *, ...);

int is_wildcard(char *);
int areafix_do_cmd(Node *, char *, Textlist *, Textlist *);
int cmd_new(Node *, char *, char *, int);
int cmd_new_int(Node *, char *, char *);
#ifndef AF_AVAIL
int cmd_list(Node *);
#else
int cmd_list(Node *, int);
#endif                          /* AF_AVAIL */
int cmd_listall(Node *);
int cmd_query(Node *);
int cmd_unlinked(Node *);
int cmd_sub(Node *, char *, Textlist *);
int cmd_unsub(Node *, char *, Textlist *);
int cmd_help(Node *);
int cmd_passwd(Node *, char *);
int cmd_delete(Node *, char *);
int cmd_passive(Node *, char *, Textlist *);
int cmd_active(Node *, char *, Textlist *);
short int send_rules(Node *, char *);
short int rulesup(char *);
short int hi_init(char *);

/*
 * Global vars
 */

/* Areafix (TRUE) / Filefix (FALSE) mode */
static int areafix = TRUE;

/* Program name, context, config areas.bbs name */
static char *my_name = MY_NAME_AF;
static char *my_context = MY_CONTEXT_AF;
static char *my_areasbbs = MY_AREASBBS_AF;
#ifdef FTN_ACL
static char my_type = MY_TYPE_AF;
#endif                          /* FTN_ACL */

static char *create_log_file = NULL;
#ifdef CREATE_LOG_FORWREQ
static char no_create_log_file = 1;
#endif                          /* CREATE_LOG_FORWREQ */
static char full_farea_dir[MAXPATH];
/* Name of areas.bbs file */
char *areas_bbs = NULL;

static int authorized = FALSE;
static int authorized_lvl = 1;
static char *authorized_key = "";
static char *authorized_name = "Sysop";
#ifdef SUB_LIMIT
static int authorized_lim_A = 0;
static int authorized_lim_G = 0;
static int lim_g = 0;
#endif                          /* SUB_LIMIT */
static Node authorized_node = { -1, -1, -1, -1, "" };

static int authorized_cmdline = FALSE;
int authorized_new = FALSE;
int authorized_fwd = FALSE;
static int authorized_delete = FALSE;
#ifdef AF_LISTALL_RESTRICTED
static char authorized_listall = FALSE;
#endif                          /* AF_LISTALL_RESTRICTED */

static char *fix_name;
short int fcreate_key;

/*
 * Output functions
 */
typedef void (*OFuncP)(const char *, ...);

static OFuncP areafix_printf = areafix_stdprintf;
static Textlist *areafix_otl = NULL;

void areafix_tlprintf(const char *fmt, ...)
{
    static char buf[4096];
#ifndef HAVE_SNPRINTF
    int n;
#endif
    va_list args;

    va_start(args, fmt);

#ifdef HAVE_SNPRINTF
    vsnprintf(buf, sizeof(buf), fmt, args);
#else
    n = vsprintf(buf, fmt, args);
    if (n >= sizeof(buf)) {
        fatal("Internal error - areafix_tlprintf() buf overflow", EX_SOFTWARE);
        /**NOT REACHED**/
        return;
    }
#endif
    tl_append(areafix_otl, buf);

    va_end(args);

    return;
}

void areafix_stdprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vprintf(fmt, args);
    printf("\n");

    va_end(args);

    return;
}

/*
 * Common Areafix init
 */
void areafix_init(int mode)
{
    areafix = mode;

    if (mode) {
        /* Areafix */
        my_name = MY_NAME_AF;
        my_context = MY_CONTEXT_AF;
        my_areasbbs = MY_AREASBBS_AF;
#ifdef FTN_ACL
        my_type = TYPE_ECHO;
#endif                          /* FTN_ACL */
    } else {
        /* Filefix */
        my_name = MY_NAME_FF;
        my_context = MY_CONTEXT_FF;
        my_areasbbs = MY_AREASBBS_FF;
#ifdef FTN_ACL
        my_type = TYPE_FECHO;
#endif                          /* FTN_ACL */
    }

    /* Get name of areas.bbs file from config file */
    if (!areas_bbs) {
        areas_bbs = cf_get_string(my_areasbbs, TRUE);
        if (areas_bbs == NULL) {
            fprintf(stderr, "%s: no areas.bbs specified\n", my_name);
            exit(EX_USAGE);
        }
    }
    if (!create_log_file)
        create_log_file = cf_get_string("AreaFixCreateLogFile", TRUE);

    return;
}

/*
 * Get/set name of areas.bbs file
 */
char *areafix_areasbbs(void)
{
    return areas_bbs;
}

void areafix_set_areasbbs(char *name)
{
    areas_bbs = name;
}

int areafix_check_forbidden_area(char *areaname)
{
    char *s;

    for (s = cf_get_string("AreaFixCreateForbiddenAreas", TRUE);
         s && *s; s = cf_get_string("AreaFixCreateForbiddenAreas", FALSE)) {
        if (wildmatch(areaname, s, TRUE))
            return TRUE;
    }

    for (s = cf_get_string("AreaFixCreateForbiddenAreasFile", TRUE);
         s && *s; s = cf_get_string("AreaFixCreateForbiddenAreasFile", FALSE)) {
        if (wildmatch_file(areaname, s, TRUE))
            return TRUE;
    }

    return FALSE;
}

int filefix_check_forbidden_area(char *areaname)
{
    char *s;

    for (s = cf_get_string("FileFixCreateForbiddenAreas", TRUE);
         s && *s; s = cf_get_string("FileFixCreateForbiddenAreas", FALSE)) {
        if (wildmatch(areaname, s, TRUE))
            return TRUE;
    }

    for (s = cf_get_string("FileFixCreateForbiddenAreasFile", TRUE);
         s && *s; s = cf_get_string("FileFixCreateForbiddenAreasFile", FALSE)) {
        if (wildmatch_file(areaname, s, TRUE))
            return TRUE;
    }

    return FALSE;
}

/*
 * Authorize functions
 */
void areafix_auth_init(void)
{
    authorized = FALSE;
    authorized_lvl = 1;
    authorized_key = "";
    authorized_name = "Sysop";
#ifdef SUB_LIMIT
    authorized_lim_A = 0;
    authorized_lim_G = 0;
#endif                          /* SUB_LIMIT */
    node_invalid(&authorized_node);
    authorized_cmdline = FALSE;
    authorized_new = FALSE;
    authorized_fwd = FALSE;
    authorized_delete = FALSE;
#ifdef AF_LISTALL_RESTRICTED
    authorized_listall = FALSE;
#endif                          /* AF_LISTALL_RESTRICTED */
}

int areafix_auth_check(Node * node, char *passwd, int checkpass)
{
    Passwd *pwd;
    char *p, *s;

    /* Init */
    areafix_auth_init();
    authorized_node = *node;

    /* Check password */
    debug(3, "Node %s, passwd %s", znfp1(node), passwd);
    pwd = passwd_lookup(MY_CONTEXT, node);
    debug(3, "passwd entry: %s", pwd ? pwd->passwd : "-NONE-");

    if (checkpass == FALSE) {
        debug(3, "no passwd check needed - authorized");
        authorized = TRUE;
    } else {
        if (passwd && pwd && stricmp(passwd, pwd->passwd) == 0) {
            debug(3, "passwd check o.k. - authorized");
            authorized = TRUE;
        } else {
            debug(3, "invalid password: pkt_passwd(%s) != passwd(%s)",
                  passwd ? passwd : "-NONE-", pwd ? pwd->passwd : "-NONE-");
        }
    }

    if (pwd == NULL) {
        fglog("WARNING: node %s have null password", znfp1(node));
        return authorized;
    }

    /* Extract level, key, and real name from pwd->args */
    s = strsave(pwd->args);
    if ((p = xstrtok(s, " \t")))
        authorized_lvl = atoi(p);
    if ((p = xstrtok(NULL, " \t")))
        authorized_key = strsave(p);
    if ((p = xstrtok(NULL, " \t")))
        authorized_name = strsave(p);
#ifdef SUB_LIMIT
    if ((p = xstrtok(NULL, " \t")) && p[0] != '#') {
        char *s1 = NULL;

        if (strchr(p, '/')) {
            if ((s1 = xstrtok(p, "/")))
                authorized_lim_G = atoi(s1);
            if ((s1 = xstrtok(NULL, "/ \t")))
                authorized_lim_A = atoi(s1);
        } else {
            authorized_lim_A = -1;
            authorized_lim_G = atoi(p);
        }
    } else {
        authorized_lim_A = -1;
        authorized_lim_G = -1;
    }

    xfree(s);
#endif                          /* SUB_LIMIT */

    debug(3, "passwd lvl : %d", authorized_lvl);
    debug(3, "passwd key : %s", authorized_key);
    debug(3, "passwd name: %s", authorized_name);
#ifdef SUB_LIMIT
    debug(3, "passwd lim : %d/%d", authorized_lim_G, authorized_lim_A);
#endif                          /* SUB_LIMIT */

    if (strchr(authorized_key, '$')) {
        debug(3, "authorized for NEW command");
        authorized_new = TRUE;
    }
    if (strchr(authorized_key, '&')) {
        debug(3, "authorized for FORWARD request");
        authorized_fwd = TRUE;
    }
    if (strchr(authorized_key, '~')) {
        debug(3, "authorized for DELETE command");
        authorized_delete = TRUE;
    }
#ifdef AF_LISTALL_RESTRICTED
    if (strchr(authorized_key, '%')) {
        debug(3, "authorized for LISTALL command");
        authorized_listall = TRUE;
    }
#endif                          /* AF_LISTALL_RESTRICTED */

    return authorized;
}

void areafix_auth_cmd(void)
{
    authorized = authorized_cmdline = authorized_new
        = authorized_delete = authorized_fwd = TRUE;
#ifdef AF_LISTALL_RESTRICTED
    authorized_listall = TRUE;
#endif                          /* AF_LISTALL_RESTRICTED */
}

/*
 * Areafix name
 */
char *areafix_name(void)
{
    return my_name;
}

/*
 * Return authorized node
 */
Node *areafix_auth_node(void)
{
    return &authorized_node;
}

#ifdef SUBSCRIBE_ZONEGATE

static ZoneGate *zonegate_first = NULL;
static ZoneGate *zonegate_last = NULL;

/*
 * Init zonegate list
 */
void zonegate_init(void)
{
    char *s;
    ZoneGate *p;

    for (s = cf_get_string("ZoneGate", TRUE);
         s && *s; s = cf_get_string("ZoneGate", FALSE)) {
        p = (ZoneGate *) xmalloc(sizeof(ZoneGate));
        p->next = NULL;
        lon_init(&p->seenby);
        lon_add_string(&p->seenby, s);
        if (p->seenby.first) {
            p->node = p->seenby.first->node;
            lon_remove(&p->seenby, &p->node);
        } else
            node_invalid(&p->node);

        if (zonegate_first)
            zonegate_last->next = p;
        else
            zonegate_first = p;
        zonegate_last = p;
    }
}
#endif                          /* SUBSCRIBE_ZONEGATE */

/*
 * Process Areafix command from stdin
 */
void areafix_do(Node * node, char *subj, Textlist * tl, Textlist * out)
{
    char *passwd;
    char *p, *q;
    int q_flag = FALSE, l_flag = FALSE;
    Textline *tp;
    Textlist upl;

    areafix_auth_init();
    tl_init(&upl);

    /* Check password in Subject and process options */
    passwd = strtok(subj, " \t");
    while ((q = strtok(NULL, " \t"))) {
        if (!stricmp(q, "-q"))  /* -q = QUERY */
            q_flag = TRUE;
        if (!stricmp(q, "-l"))  /* -l = LIST */
            l_flag = TRUE;
    }
    areafix_auth_check(node, passwd, TRUE);

    /* Execute commands for subject options */
    if ((q_flag || l_flag) && out) {
        areafix_printf = areafix_tlprintf;
        debug(3, "output via textlist");
        areafix_otl = out;
    }
    if (q_flag)
        cmd_query(node);
    if (l_flag)
#ifndef AF_AVAIL
        cmd_list(node);
#else
        cmd_list(node, TRUE);   /* SNP:FIXME */
#endif                          /* !AF_AVAIL */

    /* Execute commands from stdin */
    for (tp = tl->first; tp; tp = tp->next) {
        p = tp->line;

        strip_crlf(p);          /* Strip CR/LF */
        strip_space(p);         /* Strip spaces */
        if (strneq(p, " * ", 3))    /* Skip " * blah" lines */
            continue;
        if (strneq(p, "---", 3))    /* Ignore cmds after --- */
            break;
        if (strneq(p, "--", 2)) /* Ignore cmds after --  */
            break;              /* (signature start)     */
        if (strneq(p, "--=20", 5))  /* dito, MIME            */
            break;
        for (; *p && is_space(*p); p++) ;   /* Skip white space */
        if (!*p)                /* Skip empty lines */
            continue;

        areafix_do_cmd(node, p, out, &upl);
    }

    send_request(&upl);

    return;
}

/*
 * Process command line
 */
#define CMD_LIST	1
#define CMD_QUERY	2
#define CMD_UNLINKED	3
#define CMD_SUB		4
#define CMD_UNSUB	5
#define CMD_HELP	6
#define CMD_PASSWD	7
#define CMD_LISTALL     8
#define CMD_NEW		9
#define CMD_DELETE	10
#define CMD_PASSIVE	11
#define CMD_ACTIVE	12
#ifdef AF_AVAIL
#define CMD_AVAIL	13
#endif                          /* AF_AVAIL */

int areafix_do_cmd(Node * node, char *line, Textlist * out, Textlist * upl)
{
    int cmd;
    char *arg;
    char buf[32];
    int i, ret;
    int percent = FALSE;

    /* Output */
    if (out) {
        debug(3, "output via textlist");
        areafix_otl = out;
        areafix_printf = areafix_tlprintf;
    } else {
        debug(3, "output via stdout");
        areafix_otl = NULL;
        areafix_printf = areafix_stdprintf;
    }

    debug(2, "node=%s command=%s", znfp1(node), line);

    if (areafix) {
        fix_name = cf_get_string("AreaFixName", TRUE);
    } else
        fix_name = cf_get_string("FileFixName", TRUE);

    if (line[0] == '%') {
        percent = TRUE;
        line++;
    }

    if (line[0] == '+') {
        cmd = CMD_SUB;
        arg = line + 1;
    } else if (line[0] == '-') {
        cmd = CMD_UNSUB;
        arg = line + 1;
    } else if (line[0] == '&') {
        cmd = CMD_NEW;
        arg = line + 1;
    } else if (line[0] == '~') {
        cmd = CMD_DELETE;
        arg = line + 1;
    } else {
        for (i = 0; line[i] && !is_space(line[i]) && i < sizeof(buf) - 1; i++)
            buf[i] = line[i];
        buf[i] = 0;
        arg = line + i;

        if (!stricmp(buf, "list"))
            cmd = CMD_LIST;
        else if (!stricmp(buf, "query"))
            cmd = CMD_QUERY;
        else if (!stricmp(buf, "unlinked"))
            cmd = CMD_UNLINKED;
        else if (!stricmp(buf, "subscribe"))
            cmd = CMD_SUB;
        else if (!stricmp(buf, "sub"))
            cmd = CMD_SUB;
        else if (!stricmp(buf, "unsubscribe"))
            cmd = CMD_UNSUB;
        else if (!stricmp(buf, "unsub"))
            cmd = CMD_UNSUB;
        else if (!stricmp(buf, "help"))
            cmd = CMD_HELP;
        else if (!stricmp(buf, "passwd"))
            cmd = CMD_PASSWD;
        else if (!stricmp(buf, "password"))
            cmd = CMD_PASSWD;
        else if (!stricmp(buf, "from"))
            cmd = CMD_PASSWD;
        else if (!stricmp(buf, "listall"))
            cmd = CMD_LISTALL;
        else if (!stricmp(buf, "new"))
            cmd = CMD_NEW;
        else if (!stricmp(buf, "create"))
            cmd = CMD_NEW;
        else if (!stricmp(buf, "delete"))
            cmd = CMD_DELETE;
        else if (!stricmp(buf, "passive"))
            cmd = CMD_PASSIVE;
        else if (!stricmp(buf, "pause"))
            cmd = CMD_PASSIVE;
        else if (!stricmp(buf, "active"))
            cmd = CMD_ACTIVE;
        else if (!stricmp(buf, "resume"))
            cmd = CMD_ACTIVE;
        else if (!stricmp(buf, "avail"))
#ifdef AF_AVAIL
            cmd = CMD_AVAIL;
#else
            cmd = CMD_LISTALL;
#endif                          /* AF_AVAIL */
        else {
            if (percent) {
                areafix_printf("Unknown command %%%s", buf);
                return OK;
            } else {
                /* Interpret line as area to add */
                /* TODO: YK: is it right condition check? */
                if (NULL == cf_get_string("AreaFixSubscribeOnlyIfPlus", TRUE)) {
                    debug(8, "config: AreaFixSubscribeOnlyIfPlus");
                    cmd = CMD_SUB;
                    arg = line;
                } else {
                    areafix_printf("Unknown command %s", buf);
                    return OK;
                }
            }
        }
    }

    while (*arg && is_space(*arg))
        arg++;

    debug(2, "cmd=%d node=%s arg=%s", cmd, znfp1(node), arg);

    ret = OK;
    switch (cmd) {
    case CMD_LIST:
#ifndef AF_AVAIL
        ret = cmd_list(node);
#else
        ret = cmd_list(node, TRUE);
#endif                          /* !AF_AVAIL */
        break;
    case CMD_QUERY:
        ret = cmd_query(node);
        break;
    case CMD_UNLINKED:
        ret = cmd_unlinked(node);
        break;
    case CMD_SUB:
        ret = cmd_sub(node, arg, upl);
        break;
    case CMD_UNSUB:
        ret = cmd_unsub(node, arg, upl);
        break;
    case CMD_HELP:
        ret = cmd_help(node);
        break;
    case CMD_PASSWD:
        ret = cmd_passwd(node, arg);
        break;
    case CMD_LISTALL:
        ret = cmd_listall(node);
        break;
    case CMD_NEW:
        ret = cmd_new(node, arg, NULL, FALSE);
        break;
    case CMD_DELETE:
        ret = cmd_delete(node, arg);
        break;
    case CMD_PASSIVE:
        ret = cmd_passive(node, arg, upl);
        break;
    case CMD_ACTIVE:
        ret = cmd_active(node, arg, upl);
        break;
#ifdef AF_AVAIL
    case CMD_AVAIL:
        ret = cmd_list(node, FALSE);
        break;
#endif                          /* AF_AVAIL */
    }

    return ret;
}

/*
 * Internal new command
 */
int cmd_new_int(Node * node, char *line, char *dwnl)
{
    int ret;
    OFuncP areafix_savedprintf = areafix_printf;
    Textlist *areafix_savedotl = areafix_otl;

    areafix_printf = fglog;
    areafix_otl = NULL;

    ret = cmd_new(node, line, dwnl, TRUE);

    areafix_printf = areafix_savedprintf;
    areafix_otl = areafix_savedotl;

    return ret;
}

/*
 * New command
 */
int cmd_new(Node * node, char *line, char *dwnl, int inter)
{
    AreasBBS *p;
    char *name, *o1, *o2, *autocreate_script_cmd;
    char *s1, *s2;
#ifndef ACTIVE_LOOKUP
    Area *autocreate_area;
#endif                          /* ACTIVE_LOOKUP */
    FILE *fd;
    int i, ignore_prl = FALSE;
    char *autocreate_fecho_path = NULL;

    if (!(authorized_new || (authorized_fwd && inter)) || !authorized) {
        areafix_printf("Command not authorized.");
        return OK;
    }

    name = xstrtok(line, " \t");

    if ((p = areasbbs_lookup(name))) {
        areafix_printf("%-41s: area already exists,\r\n"
                       "%-41s  can't create new one.", name, " ");
        return OK;
    }

    if ((o1 = cf_get_string("ForbiddenChar", TRUE))) {
        for (i = 0; o1[i] != '\x0'; i++)
            if (strchr(name, o1[i]) != NULL) {
                fglog
                    ("WARNING: Area \"%s\"  have forbidden char, can't create.",
                     name);
                areafix_printf("%-41s: have forbidden char, can't create.",
                               name);
                return -2;
            }
    }

    if (!authorized_cmdline) {
        if (areafix && areafix_check_forbidden_area(name)) {
            areafix_printf("%-41s: forbidden area, can't create.", name);
            return OK;
        }
        if (!areafix && filefix_check_forbidden_area(name)) {
            areafix_printf("%-41s: forbidden area, can't create.", name);
            return OK;
        }
    }

    /* Create new areas.bbs entry */
    p = areasbbs_new();
    p->area = strsave(str_upper(name));
    p->zone = node->zone;

    p->time = time(NULL);

    if (cf_get_string("IgnorePRLKey", TRUE)) {
        ignore_prl = TRUE;
    }

    /* Parse options:
     *
     *     -#            passthru
     *     -p            passthru
     *     -r            read-only
     *     -l LVL        Areafix access level
     *     -k KEY        Areafix access key
     *     -z Z          zone
     *     -d DESC  descriptor
     *     -a ADDR       our aka
     *     -s STATE      state
     *     -e DAYS       expire time (no traffic)
     *     -n DAYS       expire time (forward request)
     */
    while ((o1 = xstrtok(NULL, " \t"))) {

        if (!ignore_prl || inter) {
            if ((streq(o1, "-#") || streq(o1, "-p")) && (!dwnl || inter))   /* -# */
                p->flags |= AREASBBS_PASSTHRU;
#ifndef FTN_ACL
            if (streq(o1, "-r"))    /* -r */
                p->flags |= AREASBBS_READONLY;
#endif                          /* FTN_ACL */
            if (streq(o1, "-l")) {  /* -l LVL */
                if (!(o2 = xstrtok(NULL, " \t")))
                    break;
                p->lvl = atoi(o2);
            }
        }

        if (streq(o1, "-k")) {  /* -k KEY */
            if (!(o2 = xstrtok(NULL, " \t")))
                break;
            p->key = strsave(o2);
        }

        if (streq(o1, "-d")) {  /* -d DESC */
            if (!(o2 = xstrtok(NULL, " \t")))
                break;
            p->desc = strsave(o2);
        }
        if (streq(o1, "-z")) {  /* -z Z */
            if (!(o2 = xstrtok(NULL, " \t")))
                break;
            p->zone = atoi(o2);
        }
        if (streq(o1, "-a")) {  /* -a ADDR */
            if (!(o2 = xstrtok(NULL, " \t")))
                break;
            asc_to_node(o2, &p->addr, FALSE);
        }
        if (streq(o1, "-s")) {  /* -s STATE */
            o2 = xstrtok(NULL, " \t");
            p->state = strsave(o2);
        }
        if (streq(o1, "-e")) {  /* -e DAYS */
            if (!(o2 = xstrtok(NULL, " \t")))
                break;
            p->expire_n = atoi(o2);
        }
        if (streq(o1, "-n")) {  /* -n DAYS */
            if (!(o2 = xstrtok(NULL, " \t")))
                break;
            p->expire_t = atoi(o2);
        }
    }

    if (p->state == NULL)
        p->state = strsave("S");
    /* if mode filefix */
    if (!strcmp(my_context, "ff")) {
        if (p->flags & AREASBBS_PASSTHRU)   /* -# */
            p->dir = strsave("-");
        else {
//      p->flags &= AREASBBS_PASSTHRU;
            autocreate_fecho_path = cf_get_string("AutoCreateFechoPath", TRUE);
            if (autocreate_fecho_path == NULL) {
                fglog
                    ("CONFIG: AutoCreateFechoPath not defined and filearea not passthru");
                p->dir = strsave("-");
            }
            sprintf(full_farea_dir, "%s/%s", autocreate_fecho_path,
                    str_lower(name));
            p->dir = strsave(full_farea_dir);
            if (check_access(full_farea_dir, CHECK_DIR) == ERROR) {
                if (check_access(autocreate_fecho_path, CHECK_DIR) == ERROR) {
                    if (mkdir(autocreate_fecho_path, FILE_DIR_MODE) == -1) {
                        fglog("$ERROR: can't create directory %s",
                              autocreate_fecho_path);
                        return ERROR;
                    } else {
                        chmod(autocreate_fecho_path, DIR_MODE);
                        fglog("create fileecho directory %s",
                              autocreate_fecho_path);
                    }
                }
                if (mkdir(full_farea_dir, FILE_DIR_MODE) == -1) {
                    fglog("$ERROR: can't create directory %s", full_farea_dir);
                    return ERROR;
                } else {
                    chmod(full_farea_dir, DIR_MODE);
                    fglog("create directory %s", full_farea_dir);
                }
            }
        }
    } else
        p->dir = strsave("-");

    lon_init(&p->nodes);
    lon_add(&p->nodes, node);

    areasbbs_add(p);

#ifdef CREATE_LOG_FORWREQ
    if (no_create_log_file == 1) {
#endif                          /* CREATE_LOG_FORWREQ */
        if (create_log_file) {
            if ((fd = fopen_expand_name(create_log_file, "a", FALSE)) != NULL) {
                fprintf(fd, "%s %s %s %s %lu\n", my_context, p->area,
                        znf1(node), dwnl ? dwnl : "<null>",
                        (unsigned long)time(NULL));
                fclose(fd);
            } else
                fglog("ERROR: can't open create log file %s", create_log_file);
        }
#ifdef CREATE_LOG_FORWREQ
    } else
        no_create_log_file = 1;
#endif                          /* CREATE_LOG_FORWREQ */
#ifndef FTN_ACL
    fglog("%s %s: new %s lvl=%d key=%s desc=\"%s\"%s%s",
#else
    fglog("%s %s: new %s lvl=%d key=%s desc=\"%s\"%s",
#endif                          /* !FTN_ACL */
          my_context,
          znfp1(node),
          p->area,
          p->lvl, p->key ? p->key : "<none>", p->desc ? p->desc : "<none>",
#ifndef FTN_ACL
          p->flags & AREASBBS_PASSTHRU ? " passthru" : "",
          p->flags & AREASBBS_READONLY ? " ro" : "");
#else
          p->flags & AREASBBS_PASSTHRU ? " passthru" : "");
#endif                          /* !FTN_ACL */

    areafix_printf("%-41s: created", p->area);

    if ((autocreate_script_cmd = cf_get_string("AutoCreateCmd", TRUE))) {
#ifndef FTN_ACL
        sprintf(buffer, "%s %s %s %s %s %d %s %s %d %s %s",
#else
        sprintf(buffer, "%s %s %s %s %s %d %s %s %d %s",
#endif                          /* !FTN_ACL */
                autocreate_script_cmd,
                p->area,
                znfp1(node),
                p->state,
                znfp1(&p->addr),
                p->lvl,
                p->key ? p->key : "NONE", p->desc ? p->desc : "NONE", p->zone,
#ifndef FTN_ACL
                p->flags & AREASBBS_PASSTHRU ? "-#" : "-",
                p->flags & AREASBBS_READONLY ? "-r" : "-");
#else
                p->flags & AREASBBS_PASSTHRU ? "-#" : "-");
#endif                          /* !FTN_ACL */
        if (run_system(buffer))
            debug(7, "exec autocreate script %s", buffer);
        else
            fglog("ERROR: failed exec autocreate script %s", buffer);
    } else
        areasbbs_changed();

    if (!strcmp(my_context, "ff")) {

        for (s1 = cf_get_string("AutoCreateSubscribeFileechoNodes", TRUE);
             s1 && *s1;
             s1 = cf_get_string("AutoCreateSubscribeFileechoNodes", FALSE)) {

            Node node, old;
            old.zone = cf_zone();
            old.net = old.node = old.point = -1;

            for (s2 = xstrtok(s1, " \t"); s2 && *s2; s2 = xstrtok(NULL, " \t"))
                if (OK == asc_to_node_diff(s2, &node, &old)) {
                    old = node;
                    lon_add(&(p->nodes), &node);
                } else
                    fglog("config: AutoCreateSubscribeFileechoNodes: \
			    invalid entry \"%s\"", s2);
        }
        xfree(s1);
    } else {
        /*
         * Subscribe nodes if needed
         */
        for (s1 = cf_get_string("AutoCreateSubscribeNodes", TRUE);
             s1 && *s1; s1 = cf_get_string("AutoCreateSubscribeNodes", FALSE)) {

            Node node, old;
            old.zone = cf_zone();
            old.net = old.node = old.point = -1;

            for (s2 = xstrtok(s1, " \t"); s2 && *s2; s2 = xstrtok(NULL, " \t"))
                if (OK == asc_to_node_diff(s2, &node, &old)) {
                    old = node;
                    lon_add(&(p->nodes), &node);
                    debug(5, "subscribe node %s", s2);
                } else
                    fglog("config: AutoCreateSubscribeNodes: invalid entry \
			\"%s\"", s2);
        }
        xfree(s1);
    }
#ifndef ACTIVE_LOOKUP
    if (cf_get_string("AutoCreateNG", TRUE)) {
        if (autocreate_area = areas_lookup(p->area, NULL, node)) {
            fglog("create newsgroup %s", autocreate_area->group);
            BUF_COPY2(buffer, "%N/ngoper create ", autocreate_area->group);
            if (0 != run_system(buffer))
                fglog("ERROR: can't create newsgroup (rc != 0)");
        } else {
            fglog("ERROR: can't create newsgroup (not found in areas)");
        }
    }
#endif                          /* ACTIVE_LOOKUP */

    return OK;
}

/*
 * ListAll command
 */
int cmd_listall(Node * node)
{
    AreasBBS *p;
    AreaUplink *a;
    LON *l;
    int find;
    char *n, *f1, *f2;
    char *t;
    FILE *fp;
    char buf[BUFSIZ];

#ifdef AFSEND_ECHO_STATUS
    char tmp[35];
#endif                          /* AFSEND_ECHO_STATUS */
    fglog("%s: listall", znfp1(node));

#ifndef AF_LISTALL_RESTRICTED
    if (!authorized)
#else
    if (!authorized || !authorized_listall)
#endif                          /* !AF_LISTALL_RESTRICTED */
    {
        areafix_printf("Command LISTALL: not authorized.");
        return OK;
    }

    areafix_printf("");
    areafix_printf("ALL available areas:");
    areafix_printf("");

    BUF_COPY2(buf, cf_p_vardir(), "/avail");
    hi_init(buf);

    for (p = areasbbs_first(); p; p = p->next) {
        char *mark;
#ifdef FTN_ACL
        char *mark_r, *mark_m;
#endif                          /* FTN_ACL */

        hi_write_avail(p->area, "");

        l = &p->nodes;
        if (lon_search(l, node))
            mark = (lon_search(&p->passive, node) ? "P" : "*");
        else
            mark = " ";

#ifdef FTN_ACL
        if ((cf_get_string("UplinkCanBeReadonly", TRUE) ||
             !lon_is_uplink(&(p->nodes), p->uplinks, node)) &&
            ftnacl_isreadonly(node, p->area, my_type)) {
            mark_r = "R";
        } else {
            mark_r = " ";
        }
        mark_m = (ftnacl_ismandatory(node, p->area, my_type) ? "M" : " ");
#endif                          /* FTN_ACL */

        if (p->desc)
#ifndef FTN_ACL
            sprintf(buffer, "%s Z%-3d %-39s: %s",
                    mark, p->zone, p->area, p->desc);
#else
            sprintf(buffer, "%s %s %s Z%-3d %-35s: %s",
                    mark, mark_r, mark_m, p->zone, p->area, p->desc);
#endif                          /* !FTN_ACL */
        else
#ifndef FTN_ACL
            sprintf(buffer, "%s Z%-3d %s", mark, p->zone, p->area);
#else
            sprintf(buffer, "%s %s %s Z%-3d %s",
                    mark, mark_r, mark_m, p->zone, p->area);
#endif                          /* !FTN_ACL */
#ifdef AFSEND_ECHO_STATUS
        sprintf(tmp, "   '%s' %s", p->state, ctime(&p->time));
        tmp[strlen(tmp) - 1] = 0;
        BUF_APPEND(buffer, tmp);
#endif                          /* AFSEND_ECHO_STATUS */
        areafix_printf("%s", buffer);
    }

    for (a = uplinks_first(); a; a = a->next) {
        t = strsave(a->areas);

        if (a->areafix == areafix) {
            areafix_printf("");
            areafix_printf("Areas from uplink %s", znfp1(&a->uplink));
            areafix_printf("");

            for (n = strtok(t, ","); n; n = strtok(NULL, ",")) {
                if (is_wildcard(n))
                    continue;

                if (*n == '/' || *n == '%' || *n == '.') {
                    debug(14, "Reading uplink area file %s", n);
                    fp = fopen_expand_name(n, R_MODE_T, FALSE);
                    if (!fp)
                        continue;

                    while (cf_getline(buf, BUFFERSIZE, fp)) {
                        if (!*buf)
                            continue;
                        f2 = xstrtok(buf, " \t");
                        f1 = xstrtok(NULL, "\n");

                        find = FALSE;
                        if (hi_test(str_upper(f2)))
                            find = TRUE;

                        if (!find) {
                            if (f1) {
                                hi_write_avail(f2, f1);
#ifndef FTN_ACL
                                sprintf(buffer, "  Z%-3d %-39s: %s",
                                        (a->uplink).zone, f2, f1);
#else
                                sprintf(buffer, "      Z%-3d %-35s: %s",
                                        (a->uplink).zone, f2, f1);
#endif                          /* !FTN_ACL */
                            } else {
                                hi_write_avail(f2, "");
#ifndef FTN_ACL
                                sprintf(buffer, "  Z%-3d %s",
                                        (a->uplink).zone, f2);
#else
                                sprintf(buffer, "      Z%-3d %s",
                                        (a->uplink).zone, f2);
#endif                          /* !FTN_ACL */
                            }
                            areafix_printf("%s", buffer);
                        }
                    }
                    fclose(fp);
                } else {
#ifndef FTN_ACL
                    areafix_printf("  Z%-3d %s", (a->uplink).zone, n);
#else
                    areafix_printf("      Z%-3d %s", (a->uplink).zone, n);
#endif                          /* !FTN_ACL */
                }
            }
            xfree(t);
        }
    }
    hi_close();
    unlink(BUF_COPY2(buf, cf_p_vardir(), "/avail"));
    unlink(BUF_COPY2(buffer, buf, ".dir"));
    unlink(BUF_COPY2(buffer, buf, ".pag"));

    areafix_printf("");
    areafix_printf("* = linked to this area");
    areafix_printf("P = passive mode (write only)");
#ifdef FTN_ACL
    areafix_printf("R = read only");
    areafix_printf("M = mandatory (can't unsubscribe)");
#endif                          /* FTN_ACL */
#ifdef AFSEND_ECHO_STATUS
    areafix_printf("W = area subscribed at uplink but no traffic yet"
                   "    (return from passive mode)");
    areafix_printf("F = area requested from uplink but no traffic yet");
    areafix_printf("U = area is currently not subscribed at uplink");
    areafix_printf("S = area is currently subscribed at uplink");
#endif                          /* AFSEND_ECHO_STATUS */
    areafix_printf("");

    return OK;
}

/*
 * List command
 */
#ifndef AF_AVAIL
int cmd_list(Node * node)
#else
int cmd_list(Node * node, int flag) /* FALSE -> %avail; TRUE -> %list */
#endif                          /* !AF_AVAIL */
{
    AreasBBS *p;
    LON *l;
    char *s, *mark;
    int key_ok;
    int list_f = FALSE;
#ifdef AFSEND_ECHO_STATUS
    char tmp[35];
#endif                          /* AFSEND_ECHO_STATUS */
#ifdef FTN_ACL
    char *mark_r, *mark_m;
#endif                          /* FTN_ACL */
#ifdef AF_AVAIL
    int print_all = FALSE;
#endif                          /* AF_AVAIL */

    fglog("%s: list", znfp1(node));

    if (!authorized) {
        areafix_printf("Command LIST: not authorized.");
        return OK;
    }

    areafix_printf("");

#ifdef AF_AVAIL
    if (cf_get_string("AreafixAvailPrintsAllAreas", TRUE))
        print_all = TRUE;
#endif                          /* AF_AVAIL */
    if (cf_get_string("FStatusAreaFixList", TRUE))
        list_f = TRUE;

    areafix_printf("Areas available to %s:", znf1(node));
#ifdef AF_AVAIL
    if (TRUE == flag) {
        areafix_printf("(currently subscribed from uplinks)", znf1(node));
    } else {
        if (!print_all) {
            areafix_printf("(currently not subscribed from uplinks)",
                           znf1(node));
        }
    }
#endif                          /* AF_AVAIL */
    areafix_printf("");

    /* Check for unscribed areas & define echolist in config file */
#ifdef AF_AVAIL
    if (flag == TRUE || !(s = cf_get_string("AvailFile", TRUE))) {
#endif
        for (p = areasbbs_first(); p; p = p->next) {

            l = &p->nodes;

            if (!lon_search(l, node)) {
                /* Check permissions */
                if (p->lvl > authorized_lvl)
                    continue;
                if (p->key) {
                    key_ok = TRUE;
                    for (s = p->key; *s; s++)
                        if (!strchr(authorized_key, *s)) {
                            key_ok = FALSE;
                            break;
                        }
                    if (!key_ok)
                        continue;
                }

                /* Check zone */
                if (p->zone != node->zone
                    && ((p->zone) > 6 || (node->zone) > 6))
                    continue;

#ifdef AF_AVAIL
                if (NULL == p->state || areasbbs_isstate(p->state, 'U'))
                    continue;

                if (TRUE == flag) {
                    if (areasbbs_isstate(p->state, 'W') ||
                        (areasbbs_isstate(p->state, 'F') && (list_f != TRUE)))
                        continue;
                } else if (!print_all)
                    if (areasbbs_isstate(p->state, 'W') ||
                        areasbbs_isstate(p->state, 'F'))
                        continue;
            }
#endif                          /* AF_AVAIL */
            if (lon_search(l, node))
                mark = ((lon_search(&p->passive, node)) ? "P" : "*");
            else
                mark = " ";

#ifdef FTN_ACL
            if ((cf_get_string("UplinkCanBeReadonly", TRUE) ||
                 !lon_is_uplink(&(p->nodes), p->uplinks, node)) &&
                ftnacl_isreadonly(node, p->area, my_type)) {
                mark_r = "R";
            } else {
                mark_r = " ";
            }
            mark_m = (ftnacl_ismandatory(node, p->area, my_type) ? "M" : " ");
#endif                          /* FTN_ACL */
            if (p->desc)
#ifndef FTN_ACL
                sprintf(buffer, "%s %-39s: %s", mark, p->area, p->desc);
#else
                sprintf(buffer, "%s %s %s %-35s: %s", mark, mark_r, mark_m,
                        p->area, p->desc);
#endif                          /* !FTN_ACL */
            else
#ifndef FTN_ACL
                sprintf(buffer, "%s %s", mark, p->area);
#else
                sprintf(buffer, "%s %s %s %s", mark, mark_r, mark_m, p->area);
#endif                          /* !FTN_ACL */
#ifdef AFSEND_ECHO_STATUS
            sprintf(tmp, "   '%s' %s", p->state, ctime(&p->time));
            tmp[strlen(tmp) - 1] = 0;
            BUF_APPEND(buffer, tmp);
#endif                          /* AFSEND_ECHO_STATUS */
            areafix_printf("%s", buffer);
        }
    }
#ifdef AF_AVAIL
    /* Check for define echolist in config file */
    else {
        char *n;
        FILE *fp;
        int first = TRUE;
        while ((n = xstrtok(first ? s : NULL, ",")) != NULL) {
            first = FALSE;
            fp = fopen_expand_name(n, R_MODE_T, FALSE);
            if (!fp)
                continue;
            while (cf_getline(buffer, BUFFERSIZE, fp)) {
                areafix_printf("%s", buffer);
            }
        }
    }

#endif                          /* AF_AVAIL */

    areafix_printf("");
    areafix_printf("* = linked to this area");
    areafix_printf("P = passive mode (write only)");
#ifdef FTN_ACL
    areafix_printf("R = read only");
    areafix_printf("M = mandatory (can't unsubscribe)");
#endif                          /* FTN_ACL */
#ifdef AFSEND_ECHO_STATUS
    areafix_printf("W = area subscribed at uplink but no traffic yet"
                   "    (from passive) mode");
    areafix_printf("F = area requested from uplink but no traffic yet");
    areafix_printf("U = area is currently not subscribed at uplink");
    areafix_printf("S = area is currently subscribed at uplink");
#endif                          /* AFSEND_ECHO_STATUS */
    areafix_printf("");

    return OK;
}

/*
 * Query command
 */
int cmd_query(Node * node)
{
    AreasBBS *p;
    LON *l;

    fglog("%s: query", znfp1(node));

    if (!authorized) {
        areafix_printf("Command QUERY: not authorized.");
        return OK;
    }

    areafix_printf("");
    areafix_printf("%s is linked to the following areas:", znf1(node));
    areafix_printf("");

    for (p = areasbbs_first(); p; p = p->next) {
#ifdef FTN_ACL
        char *mark_r, *mark_m;
#endif                          /* FTN_ACL */
        char *mark_p;
        l = &p->nodes;

#ifdef FTN_ACL
        if ((cf_get_string("UplinkCanBeReadonly", TRUE) ||
             !lon_is_uplink(&(p->nodes), p->uplinks, node)) &&
            ftnacl_isreadonly(node, p->area, my_type)) {
            mark_r = "R";
        } else {
            mark_r = " ";
        }
        mark_m = (ftnacl_ismandatory(node, p->area, my_type) ? "M" : " ");
#endif                          /* FTN_ACL */
        mark_p = (lon_search(&p->passive, node) ? "P" : " ");
        if (lon_search(l, node))
#ifndef FTN_ACL
            areafix_printf("%s %s", mark_p, p->area);
#else
            areafix_printf("%s %s %s %s", mark_p, mark_r, mark_m, p->area);
#endif                          /* !FTN_ACL */
    }

    areafix_printf("");
    areafix_printf("P = passive mode (write only)");
#ifdef FTN_ACL
    areafix_printf("R = read only");
    areafix_printf("M = mandatory (can't unsubscribe)");
#endif                          /* FTN_ACL */
    areafix_printf("");

    return OK;
}

/*
 * Unlinked command
 */
int cmd_unlinked(Node * node)
{
    AreasBBS *p;
    LON *l;
    char *s;
    int key_ok;

    fglog("%s: unlinked", znfp1(node));

    if (!authorized) {
        areafix_printf("Command UNLINKED: not authorized.");
        return OK;
    }

    areafix_printf("");
    areafix_printf("%s is not linked to the following available areas:",
                   znf1(node));
    areafix_printf("");

    for (p = areasbbs_first(); p; p = p->next) {
#ifdef FTN_ACL
        char *mark_r, *mark_m;
#endif                          /* FTN_ACL */

        l = &p->nodes;

        /* Check permissions */
        if (p->lvl > authorized_lvl)
            continue;
        if (p->key) {
            key_ok = TRUE;
            for (s = p->key; *s; s++)
                if (!strchr(authorized_key, *s)) {
                    key_ok = FALSE;
                    break;
                }
            if (!key_ok)
                continue;
        }

        /* Check zone */
        if (areafix && p->zone != node->zone)
            continue;

#ifdef FTN_ACL
        if ((cf_get_string("UplinkCanBeReadonly", TRUE) ||
             !lon_is_uplink(&(p->nodes), p->uplinks, node)) &&
            ftnacl_isreadonly(node, p->area, my_type)) {
            mark_r = "R";
        } else {
            mark_r = " ";
        }
        mark_m = (ftnacl_ismandatory(node, p->area, my_type) ? "M" : " ");
#endif                          /* FTN_ACL */

        if (!lon_search(l, node))
#ifndef FTN_ACL
            areafix_printf("  %s", p->area);
#else
            areafix_printf("%s %s %s", mark_r, mark_m, p->area);
#endif                          /* !FTN_ACL */
    }

    areafix_printf("");
#ifdef FTN_ACL
    areafix_printf("R = read only");
    areafix_printf("M = mandatory (can't unsubscribe)");
#endif                          /* FTN_ACL */
    areafix_printf("");

    return OK;
}

#define		DELETE 2

/*
 * Add command
 */
int cmd_sub(Node * node, char *area_in, Textlist * upl)
{

    AreasBBS *p, *n;
    AreaUplink *a;
    LON *l, *ln;
    int sub_areas = 0;
    int iswc;
    char buf[BUFSIZ];
    char area[strlen(area_in) + 1];
    int an;
#ifdef AFSEND_ECHO_STATUS
    char tmp[64];
#endif                          /* AFSEND_ECHO_STATUS */
#ifdef SUB_LIMIT
    int lim_a;
#endif
#ifdef SUBSCRIBE_ZONEGATE
    int zg_flag = FALSE;

    zonegate_init();
    if (zonegate_first) {
        ZoneGate *pz;

        for (pz = zonegate_first; pz; pz = pz->next)
            if (node_eq(node, &pz->node))
                zg_flag = TRUE;
    }
    debug(7, "Zonegate : %s", zg_flag ? "TRUE" : "FALSE");
    if (!authorized || !zg_flag)
#else
    if (!authorized)
#endif                          /* SUBSCRIBE_ZONEGATE */
    {
        areafix_printf("Command SUBSCRIBE: not authorized.");
        return OK;
    }

    BUF_COPY(area, area_in);

#ifdef SUB_LIMIT
    if (lim_g < 1)
        for (p = areasbbs_first(); p; p = p->next) {
            l = &p->nodes;

            if (l && l->size > 1 && (lon_search(l, node) ||
                                     lon_search(&p->passive, node)))
                lim_g++;
        }
#endif                          /* SUB_LIMIT */
    iswc = is_wildcard(area);

    BUF_COPY(buf, area);

    for (p = areasbbs_first(); p; p = p->next) {

        l = &p->nodes;

        if (sub_areas && !iswc)
            return OK;

        if (wildmatch(p->area, buf, TRUE)) {
            sub_areas++;

            if (!authorized_cmdline) {  /* Command line may do everything */
                /* Check permissions */
                if (p->lvl > authorized_lvl) {
                    areafix_printf("%-41s: access denied (level)", p->area);
                    if (upl == NULL)
                        return DELETE;

                    continue;
                }
                if (p->key) {
                    if (!strpbrk(p->key, authorized_key)) {
                        areafix_printf("%-41s: access denied (key)", p->area);
                        if (upl == NULL)
                            return DELETE;
                        continue;
                    }
                }

                if (lon_search(l, node)) {
                    if (lon_search(&p->passive, node)) {
                        lon_remove(&p->passive, node);
                        areasbbs_changed();
                        areafix_printf("%-41s: set active", p->area);
#ifdef SUB_LIMIT
                        lim_g++;
#endif                          /* SUB_LIMIT */
                    } else
                        areafix_printf("%-41s: already active", p->area);
                    continue;
                }

#ifdef SUB_LIMIT
                if (authorized_lim_G > 0 && authorized_lim_G <= lim_g) {
                    areafix_printf("%-41s: access denied (general limit %d)",
                                   p->area, lim_g);
                    if (upl == NULL)
                        return DELETE;

                    continue;
                }
                if (authorized_lim_A > 0) {
                    for (lim_a = 0, n = areasbbs_first(); n; n = n->next) {
                        ln = &n->nodes;

                        if (ln && ln->size > 1 && l->first &&
                            (lon_search(ln, node)
                             || lon_search(&p->passive, node))
                            && lon_search(ln, &l->first->node))
                            lim_a++;
                    }
                    if (authorized_lim_A <= lim_a) {
                        areafix_printf("%-41s: access denied\r\n"
                                       "%-41s  (limit for uplink %s = %d)",
                                       p->area, " ", znfp1(&l->first->node),
                                       lim_a);
                        if (upl == NULL)
                            return DELETE;

                        continue;
                    }
                }
#endif                          /* SUB_LIMIT */

                /* Check zone */
                if (areafix && p->zone != node->zone) {
                    if (!p->zone) {
                        areafix_printf("%-41s: no uplink, dead area", p->area);
                        fglog("%s: dead area %s, delete", znfp1(node), p->area);
                        areasbbs_remove1(p);
                        areasbbs_changed();
                        continue;
                    }
                    if ((p->zone > 6) || (node->zone > 6)) {
                        areafix_printf("%-41s: different zone (Z%d), not added",
                                       p->area, p->zone);
                        continue;
                    }
                }
            }

            if (l->first) {
                lon_add(l, node);
                areasbbs_changed();

                sprintf(buffer, "%-41s: ", p->area);
#ifndef ANSWER_OK
                BUF_APPEND(buffer, "subscribed");
#else
                BUF_APPEND(buffer, "Ok");
#endif                          /* ANSWER_OK */
#ifdef AFSEND_ECHO_STATUS
                snprintf(tmp, sizeof(tmp), " Stat: '%s' last msg: %s",
                        p->state, ctime(&p->time));
                BUF_APPEND(buffer, tmp);
#endif                          /* AFSEND_ECHO_STATUS */
                areafix_printf("%s", buffer);

#ifdef SUB_LIMIT
                lim_g++;
#endif                          /* SUB_LIMIT */

                if (areasbbs_isstate(p->state, 'U') ||
                    areasbbs_isstate(p->state, 'P')) {
                    if ((a =
                         uplinks_line_get(areafix, &l->first->node)) != NULL) {
                        /* Subscribe from uplink */
                        tl_appendf(upl, "%s,%s,%s,%s,+%s",
                                   znf1(&l->first->node),
                                   a->robotname,
                                   fix_name ? fix_name : areafix_name(),
                                   a->password, str_upper(p->area));

                        areasbbs_chstate(&p->state, "UP", 'W');
                        p->time = time(NULL);

                        if (lon_search(&p->passive, node)) {
                            lon_remove(&p->passive, node);
                        }

                        /* Not subscribed at uplink, print note */
                        areafix_printf
                            ("        (this area is currently not subscribed at uplink %s)",
                             znf1(&l->first->node));
                        fglog
                            ("%s: +%s (not subscribed at uplink, request forwarded)",
                             znfp1(node), p->area);
                    } else {
                        fglog
                            ("WARNING: no entry for uplink %s in uplink config file",
                             znf1(&l->first->node));
                        areafix_printf
                            ("        Please forward this message to sysop:\r\n"
                             "        no entry for uplink %s in uplink config file",
                             znf1(&l->first->node));

                        if (upl == NULL)
                            return DELETE;
                    }
                } else if (areasbbs_isstate(p->state, 'W')) {
                    /* Subscribed at uplink, but no traffic yet, print note */
                    areafix_printf
                        ("        (this area subscribed at uplink %s, but no traffic yet)",
                         znf1(&l->first->node));
                    fglog("%s: +%s (subscribed at uplink, but no traffic yet)",
                          znfp1(node), p->area);
                } else if (areasbbs_isstate(p->state, 'F') && upl) {
                    /* Requested from uplink, but no traffic yet, print note */
                    areafix_printf
                        ("        (this area requested from uplink %s, but no traffic yet)",
                         znf1(&l->first->node));
                    fglog
                        ("%s: +%s (already requested from uplink, but no traffic yet)",
                         znfp1(node), p->area);
                } else
                    fglog("%s: +%s", znfp1(node), p->area);

                if (!iswc)
                    send_rules(node, str_upper(area));
            } else {
                areafix_printf("%-41s: no uplink, dead area", p->area);
                fglog("%s: dead area %s, delete", znfp1(node), p->area);
                areasbbs_remove1(p);
                areasbbs_changed();
            }
        }
    }

    if (!authorized_fwd) {
        areafix_printf("%-41s: forward request not authorized", area_in);
        return OK;
    }

    if (iswc || sub_areas == 0) {
        for (a = uplinks_lookup(areafix, area); a; a = a->next) {
            /* Create area */
            if (a != NULL && a->options != NULL)
                BUF_COPY3(buf, a->areas, " ", a->options);
            else
                BUF_COPY(buf, a->areas);
            an = authorized_fwd;
            authorized_fwd = TRUE;
#ifdef CREATE_LOG_FORWREQ
            no_create_log_file = NULL;
#endif                          /* CREATE_LOG_FORWREQ */
            if (NULL != (p = areasbbs_lookup(a->areas)))
                continue;
            if (is_wildcard(a->areas)) {
                debug(9,
                      "WARNING: wildcard in requested area %s and uplinks file %s",
                      area, a->areas);
                continue;
            }

            sub_areas++;

            if (OK != cmd_new_int(&a->uplink, buf, znfp1(node))) {
                areafix_printf
                    ("%s: internal areafix error (can't create area)\r\n"
                     "Please forward this message to sysop", a->areas);
                fglog("ERROR: can't create area %s (cmd_new() returned ERROR)",
                      a->areas);

                continue;
            }
            authorized_fwd = an;
            if (NULL == (p = areasbbs_lookup(a->areas))) {
                fglog("ERROR: can't create area %s (not found after creation)",
                      a->areas);
                areafix_printf
                    ("%s: internal areafix error (can't create area)\r\n"
                     "Please forward this message to sysop", a->areas);
                continue;
            }

            areasbbs_chstate(&(p->state), "S", 'F');
            areasbbs_changed();

            if (cmd_sub(node, p->area, NULL) != OK) {
                areasbbs_remove1(p);
                areasbbs_changed();
                continue;
            }

            /* Subscribe from uplink */
            tl_appendf(upl, "%s,%s,%s,%s,+%s",
                       znfp1(&a->uplink),
                       a->robotname,
                       fix_name ? fix_name : areafix_name(),
                       a->password, str_upper(a->areas));

            areafix_printf("    Request forwarded to uplink: %s",
                           znfp1(&a->uplink));
        }
        uplinks_lookup_free();
    }
    if (sub_areas) {
        if (iswc)
            areafix_printf("* %d areas requested from pattern %s", sub_areas,
                           area);
    } else
        areafix_printf("%s: no such area, or no area matching pattern\r\n"
                       "    No uplink found to forward this request", area);

    return OK;
}

/*
 * Unsubscribe command
 */
int cmd_unsub(Node * node, char *area, Textlist * upl)
{
    AreasBBS *p;
    LON *l;
    int match = FALSE;
    AreaUplink *a;
    int iswild = strchr(area, '*') || strchr(area, '?');

    if (!authorized) {
        areafix_printf("Command UNSUBSCRIBE: not authorized.");
        return OK;
    }

    for (p = areasbbs_first(); p; p = p->next) {
        l = &p->nodes;

        if (wildmatch(p->area, area, TRUE)) {
            match = TRUE;

            if (l && l->size < 1) {
                areafix_printf("%-41s: no uplink, dead area", p->area);
                fglog("%s: dead area %s, delete", znfp1(node), p->area);
                areasbbs_remove1(p);
                areasbbs_changed();
                continue;
            }
            if (!lon_search(l, node)) {
                if (!areafix || p->zone == node->zone)
                    if (!iswild)
                        areafix_printf("%-41s: not subscribed", p->area);
            } else {
#ifdef FTN_ACL
                if (ftnacl_ismandatory(node, p->area, my_type))
                    areafix_printf("%-41s: mandatory area, can't unsubscribe",
                                   p->area);
                else {
#endif                          /* FTN_ACL */
                    lon_remove(l, node);
                    if (lon_search(&(p->passive), node))
                        lon_remove(&(p->passive), node);
                    areasbbs_changed();
                    areafix_printf("%-41s: unsubscribed", p->area);

#ifdef SUB_LIMIT
                    lim_g--;
#endif                          /* SUB_LIMIT */
                    fglog("%s: -%s", znfp1(node), p->area);

                    if ((l->size == 1 && p->flags & AREASBBS_PASSTHRU)) {
                        if ((a = uplinks_line_get(areafix, &l->first->node))) {
                            fglog("unsubscribe from area %s (no downlinks)",
                                  p->area);
                            areasbbs_chstate(&(p->state), "SWF", 'U');
                            areasbbs_changed();

                            tl_appendf(upl, "%s,%s,%s,%s,-%s",
                                       znfp1(&a->uplink),
                                       a->robotname,
                                       fix_name ? fix_name : areafix_name(),
                                       a->password, str_upper(p->area));
                            continue;
                        } else {
                            fglog
                                ("uplink entry for area %s(%s) not found, delete area",
                                 p->area, znfp1(&l->first->node));
                            areasbbs_remove1(p);
                            areasbbs_changed();
                            continue;
                        }
                    }
                    if (l->size == 0 && p->flags & AREASBBS_PASSTHRU) {
                        fglog("delete area %s (no uplink)", p->area);
                        areasbbs_remove1(p);
                        areasbbs_changed();
                        continue;
                    }
#ifdef FTN_ACL
                }
#endif                          /* FTN_ACL */
            }
        }
    }
    if (!match) {
        if (iswild)
            areafix_printf("%-41s: no area matching pattern", area);
        else
            areafix_printf("%-41s: no such area", area);
    }

    return OK;
}

/*
 * Help command
 */
int cmd_help(Node * node)
{
    FILE *fp;
    char *helpfile;

    fglog("%s: help", znfp1(node));

    if ((helpfile = cf_get_string("AreaFixHelp", TRUE))) {
        if ((fp = fopen_expand_name(helpfile, R_MODE, FALSE))) {
            while (fgets(buffer, sizeof(buffer), fp)) {
                strip_crlf(buffer);
                areafix_printf(buffer);
            }
            fclose(fp);
            return OK;
        } else {
            fglog("ERROR: can't open %s", helpfile);
            areafix_printf("(can't open areafix help file)\r\n"
                           "Please forward this message to sysop");
        }
    } else
        fglog("WARNING: AreaFixHelp not defined");

    areafix_printf("Sorry, no help available.");

    return OK;
}

/*
 * Passwd command
 */
int cmd_passwd(Node * node, char *arg)
{
    char *p;
    Node n;

    fglog("%s: passwd", znfp1(node));

    authorized = FALSE;

    p = strtok(arg, " \t");     /* Node address */
    if (!p) {
        areafix_printf("Command PASSWORD: missing Z:N/F.P address.");
        return OK;
    }
    if (asc_to_node(p, &n, FALSE) == ERROR) {
        areafix_printf("Command PASSWORD: illegal address %s.", p);
        return OK;
    }
    *node = n;
    cf_set_zone(node->zone);

    p = strtok(NULL, " \t");    /* Password */
    if (!p) {
        areafix_printf("Command PASSWORD: no password given!");
        authorized = FALSE;
        return OK;
    }

    areafix_auth_check(node, p, TRUE);
    if (!authorized)
        areafix_printf("Command PASSWORD: authorization for %s failed.",
                       znfp1(node));

    return OK;
}

/*
 * Delete command
 */
int cmd_delete(Node * node, char *area)
{

    AreasBBS *p;
    int changed = FALSE;
#ifndef ACTIVE_LOOKUP
    Area *autocreate_area;
#endif                          /* ACTIVE_LOOKUP */

    if (!authorized_delete) {
        areafix_printf("Command DELETE: not authorized.");
        return OK;
    }

    for (p = areasbbs_first(); p; p = p->next) {
        if (wildmatch(p->area, area, TRUE)) {
            areafix_printf("%-41s: delete", area);
            areasbbs_remove1(p);
            changed = TRUE;
        }
    }

    if (!changed) {
        areafix_printf("%-41s: area does not exist, can't delete", area);
        return OK;
    } else
        areasbbs_changed();

#ifndef ACTIVE_LOOKUP
    if (cf_get_string("AutoCreateNG", TRUE)) {
        if (NULL != (autocreate_area = areas_lookup(area, NULL, NULL))) {
            BUF_COPY2(buffer, "%N/ngoper remove ", autocreate_area->group);
            if (0 != run_system(buffer))
                fglog("ERROR: can't create newsgroup (rc != 0)");
        } else {
            fglog("ERROR: can't create newsgroup (not found in areas)");
        }
    }
#endif                          /* ACTIVE_LOOKUP */

    return OK;
}

/*
 * Passive command
 */
int cmd_passive(Node * node, char *area, Textlist * upl)
{
    AreasBBS *p;
    LON *l;
    LNode *n;
    AreaUplink *a;

    if (!authorized) {
        areafix_printf("Command PASSIVE: not authorized.");
        return OK;
    }
    fglog("%s: passive", znfp1(node));

    for (p = areasbbs_first(); p; p = p->next) {
        l = &p->nodes;

        if ((0 != strlen(area)) && !wildmatch(p->area, area, TRUE))
            continue;

        if (lon_search(l, node) && l->size > 1) {
            if (!lon_search(&p->passive, node)) {
#ifdef FTN_ACL
                if (ftnacl_ismandatory(node, p->area, my_type))
                    areafix_printf("%-41s: mandatory area, can't set passive",
                                   p->area);
                else {
#endif                          /* FTN_ACL */
                    if (l->size <= 2) {
                        n = l->first;
                        a = uplinks_line_get(areafix, &n->node);
                        if (!a) {
                            fglog("WARNING: can't find uplink for %s area",
                                  p->area);
                            return OK;
                        }
                        tl_appendf(upl, "%s,%s,%s,%s,-%s",
                                   znfp1(&a->uplink),
                                   a->robotname,
                                   fix_name ? fix_name : areafix_name(),
                                   a->password, str_upper(p->area));

                        p->time = time(NULL);
                        if ((NULL != p->state) &&
                            (areasbbs_isstate(p->state, 'W') ||
                             areasbbs_isstate(p->state, 'F') ||
                             areasbbs_isstate(p->state, 'S'))) {
                            fglog("setting state 'P' for area %s", p->area);
                            areasbbs_chstate(&(p->state), "WFS", 'P');
                        }
                    }
                    lon_remove(&p->nodes, node);
                    lon_add(&p->passive, node);
                    areasbbs_changed();
                    areafix_printf("%-41s: set passive", p->area);
#ifdef FTN_ACL
                }
#endif                          /* FTN_ACL */
            }
        }
    }

    return OK;
}

/*
 * Active command
 */
int cmd_active(Node * node, char *area, Textlist * upl)
{
    AreasBBS *p;
    LON *l;
    AreaUplink *a;
    LNode *n;

    if (!authorized) {
        areafix_printf("Command ACTIVE: not authorized.");
        return OK;
    }
    fglog("%s: active", znfp1(node));

    for (p = areasbbs_first(); p; p = p->next) {
        l = &p->nodes;

        if ((0 != strlen(area)) && !wildmatch(p->area, area, TRUE))
            continue;

        if (lon_search(&p->passive, node)) {
            if (!lon_search(l, node)) {
                n = l->first;
                if (l->size == 1 && !node_eq(&n->node, node)) {
                    a = uplinks_line_get(areafix, &n->node);
                    if (!a) {
                        fglog("WARNING: can't find uplink for %s area",
                              p->area);
                        return OK;
                    }
                    tl_appendf(upl, "%s,%s,%s,%s,+%s",
                               znfp1(&a->uplink),
                               a->robotname,
                               fix_name ? fix_name : areafix_name(),
                               a->password, str_upper(p->area));
                    if (areasbbs_isstate(p->state, 'P') ||
                        areasbbs_isstate(p->state, 'U')) {
                        areasbbs_chstate(&(p->state), "P", 'W');
                    }
                }
                lon_remove(&p->passive, node);
                lon_add(l, node);

                areasbbs_changed();
                areafix_printf("%-41s: set active", p->area);
            }
        } else if (lon_search(l, node))
            areafix_printf("%-41s: already active", p->area);
    }

    return OK;
}

/*
 * Set areas_bbs_changed flag
 */
void areafix_set_changed(void)
{
    areasbbs_changed();
}

/*
 * Sending subscribe/unsubscribe requests to uplinks
 */
void send_request(Textlist * upl)
{

    Textline *tl;
    Textlist out, send;
    char links[BUFSIZ];
    char *link = NULL, *l, *s;
    Node tmp;
    Message msg = { 0 };

    tl_init(&out);
    tl_init(&send);

    pkt_outdir(cf_p_outpkt(), NULL);
    BUF_COPY(links, "");

    while (1) {
        for (tl = upl->first; NULL != tl; tl = tl->next) {
            s = strsave(tl->line);
            l = xstrtok(s, ",");

            if (l) {
                if (s) {
                    if (!link && !strstr(links, l)) {
                        link = strsave(l);
                        BUF_APPEND2(links, link, ",");
                    }
                    if (link && strcmp(l, link) == 0) {
                        fglog("request %s", tl->line);
                        tl_append(&out, tl->line);
                    }
                    xfree(s);
                }
            }
        }

        if (!link)
            break;

        xfree(link);
        link = NULL;
        tl = out.first;
        node_clear(&tmp);

        msg.attr = MSG_DIRECT;
        msg.cost = 0;
        msg.date = time(NULL);
        msg.area = NULL;
        asc_to_node(xstrtok(tl->line, ","), &tmp, FALSE);
        msg.node_to = tmp;
        BUF_COPY(msg.name_to, xstrtok(NULL, ","));
        BUF_COPY(msg.name_from, xstrtok(NULL, ","));
        BUF_COPY(msg.subject, xstrtok(NULL, ","));
        cf_set_best(tmp.zone, tmp.net, tmp.node);
        msg.node_from = cf_n_addr();

        l = xstrtok(NULL, ",");
        tl_appendf(&send, "\r\n%s", l);

        tl = tl->next;

        for (; tl != NULL; tl = tl->next) {
            l = strrchr(tl->line, ',') + 1;
            tl_append(&send, l);
        }
        outpkt_netmail(&msg, &send, areafix_name(), NULL, NULL);

        tl_clear(&send);
        tl_clear(&out);
    }
}

/*
 * Sending echo rules for listed links
 */
short int send_rules(Node * link, char *area)
{
    FILE *fp;
    Message msg = { 0 };
    Textlist send;
    char *filen = NULL;
    char *p;
    Node node;

    tl_init(&send);

    for (p = cf_get_string("RulesSendTo", TRUE);
         p && *p; p = cf_get_string("RulesSendTo", FALSE)) {
        asc_to_node(p, &node, TRUE);
        if (!node_eq(link, &node))
            return OK;
    }

    if ((p = cf_get_string("RulesConfig", TRUE))) {
        fp = fopen_expand_name(p, R_MODE, FALSE);
        debug(8, "config: RulesConfig %s", p);

        if (!fp && errno == ENOENT) {
            if (!rulesup(p))
                return ERROR;
            fp = fopen_expand_name(p, R_MODE, FALSE);
        }

        if (!fp) {
            fglog("ERROR: can't open %s for reading", p);
            return ERROR;
        }
    } else
        return ERROR;

    while (cf_getline(buffer, BUFSIZ, fp)) {
        p = xstrtok(buffer, " \t");

        if (p && !stricmp(area, p)) {
            if (!(filen = xstrtok(NULL, " \t"))) {
                fglog("ERROR: can't find rules file correspond for area %s",
                      area);
                return ERROR;
            }
            break;
        }
    }

    if (!filen) {
        debug(5, "find rules-file for %s area", area);
        return ERROR;
    }

    if (!(fp = fopen(filen, R_MODE))) {
        fglog("ERROR: can't open %s for reading", filen);
        return ERROR;
    }

    while (cf_getline(buffer, BUFFERSIZE, fp))
        tl_append(&send, buffer);

    msg.attr = MSG_DIRECT;
    msg.cost = 0;
    msg.date = time(NULL);
    msg.area = NULL;
    msg.node_to = *link;
    BUF_COPY(msg.name_to, "SysOp");
    BUF_COPY(msg.name_from, "Fidogate Daemon");
    sprintf(msg.subject, "%s Rules", area);
    cf_set_best(link->zone, link->net, link->node);
    msg.node_from = cf_n_addr();

    pkt_outdir(cf_p_outpkt(), NULL);

    outpkt_netmail(&msg, &send, areafix_name(), NULL, NULL);

    return OK;
}

short int rulesup(char *rulesc)
{
    char *p, *s, *rulesc1, *rdir, *buf;
    FILE *fp, *fp1;

    if (!rulesc) {
        rulesc1 = cf_get_string("RulesConfig", TRUE);
        if (rulesc1 == NULL)
            debug(8, "config: RulesConfig not found");
    } else
        rulesc1 = rulesc;

    if (rulesc1 && (s = cf_get_string("RulesDir", TRUE))) {
        rdir = strsave(s);

        if (dir_open(rdir, "*.rul", TRUE) == ERROR) {
            debug(7, "can't open directory %s", rdir);
            return ERROR;
        }
        fp1 = fopen_expand_name(rulesc1, W_MODE, FALSE);
        if (!fp1) {
            debug(7, "ERROR: reading %s", rulesc1);
            return ERROR;
        }
        for (buf = dir_get(TRUE); buf; buf = dir_get(FALSE)) {
            debug(9, "processing file %s", buf);

            fp = fopen_expand_name(buf, R_MODE, FALSE);
            if (!fp) {
                debug(7, "ERROR: reading %s", buf);
                fclose(fp1);
                return ERROR;
            }
            p = cf_getline(buffer, BUFFERSIZE, fp);
            fclose(fp);
            if (p) {
                if (strchr(p, ':')) {
                    p = strchr(p, ':') + 1;
                    if ((s = strtok(p, " \t"))) {
                        BUF_COPY4(buffer, s, "\t", buf, "\n");
                        fputs(buffer, fp1);
                    }
                } else
                    fglog("WARNING: rulesfile %s is broken", buf);
            }
        }
        fclose(fp1);
        return OK;
    }
    return ERROR;
}

void areafix_free(void)
{
    if (strlen(authorized_key) > 0)
        xfree(authorized_key);
    if (strlen(authorized_name) > 5)
        xfree(authorized_name);
}
