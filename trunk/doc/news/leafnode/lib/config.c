/*
 * (C) Maint Laboratory 2003-2004
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#define READ_CONFIG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include "../common.h"
#include "../config.h"
#include "getini.h"
#ifdef _BSD
#include <limits.h>
#define CFG1 "/usr/local/etc/util-leafnode/util-leafnode.conf"
#define CFG2 "/usr/local/etc/leafnode/util-leafnode.conf"
#else
#define CFG1 "/etc/util-leafnode/util-leafnode.conf"
#define CFG2 "/etc/leafnode/util-leafnode.conf"
#endif

int readcfg()
{
    char *path_cfg_file;
    FILE *cfg;
    char tmp[512];

    if (access(CFG2, F_OK)) {
	if (access(CFG1, F_OK)) {
            myerror("Can't find config file %s", strerror(errno));
            fprintf(stderr, "Can't find config file (%s)\n", strerror(errno));
            exit(1);
	} else path_cfg_file = CFG1;
    } else path_cfg_file = CFG2;

    if ((cfg = fopen(path_cfg_file, "r")) == NULL) {
	myerror("Can't open config file %s", path_cfg_file);
        fprintf(stderr, "Can't open config file \'%s\'\n", path_cfg_file);
        exit(1);
    }
    if(getIniInt(cfg, "[Config]", "port", &port_newsserv))
        port_newsserv = PORT_NEWSSERV;

    if(getIniStr(cfg, "[Config]", "leafnode owner", leafnode_owner))
        strcpy(leafnode_owner, LEAFNODE_OWNER);
    if(getIniStr(cfg, "[Config]", "logfile", logfile))
        strcpy(logfile, LOGFILE);
    if(getIniStr(cfg, "[Config]", "util logdir", util_logdir))
        strcpy(util_logdir, UTIL_LOGDIR);
    if(getIniStr(cfg, "[Config]", "incoming", incoming))
        strcpy(incoming, INCOMING);
    if(getIniStr(cfg, "[Config]", "outgoing", outgoing))
        strcpy(outgoing, OUTGOING);
    if(getIniStr(cfg, "[Config]", "failed posting", failed_posting))
        strcpy(failed_posting, FAILED_POSTING);
    if(getIniStr(cfg, "[Config]", "dupe post", dupe_post))
        strcpy(dupe_post, DUPE_POST);
    if(getIniStr(cfg, "[Config]", "interesting groups", interesting_groups))
        strcpy(interesting_groups, INTERESTING_GROUPS);
    if(getIniStr(cfg, "[Config]", "groupinfo", groupinfo))
        strcpy(groupinfo, GROUPINFO);
    if(getIniStr(cfg, "[Config]", "local groupinfo", local_groupinfo))
        strcpy(local_groupinfo, LOCAL_GROUPINFO);
    if(getIniStr(cfg, "[Config]", "active", active))
        strcpy(active, ACTIVE);
    if(getIniStr(cfg, "[Config]", "rfc2ftn", rfc2ftn))
        strcpy(rfc2ftn, RFC2FTN);
    if(getIniStr(cfg, "[Config]", "delete_ctrl_d", delete_ctrl_d))
        strcpy(delete_ctrl_d, DELETE_CTRL_D);
    if(getIniStr(cfg, "[Config]", "fido groups", ffgroups))
        ffgroups[0] = '\0';;

    fclose(cfg);
    return 0;
}

