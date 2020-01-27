/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Configuration data and functions
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
#include <assert.h>

/*
 * Hostname / domainname
 */
static char scf_hostname[MAXPATH];
static char scf_domainname[MAXPATH];
static char scf_hostsdomain[MAXPATH];
static char scf_fqdn[MAXPATH];

/*
 * Configured zones and addresses
 */
struct st_addr {
    int zone;                   /* Zone for this address set */
    Node addr;                  /* Our own main address */
    Node uplink;                /* Uplink address */
    Node gateaddr;              /* For new-style GateAddress config */
};

static struct st_addr scf_addr[MAXADDRESS];
static int scf_naddr = 0;       /* # of addresses */
static int scf_zone = 0;        /* Current zone */
static int scf_index = 0;       /* Index to current zone scf_addr[] */

static Node scf_c_addr = { -1, -1, -1, -1, "" };    /* Current address */
static Node scf_c_uplink = { -1, -1, -1, -1, "" };  /* Current uplink  */

static int scf_ia = 0;          /* Index for Address */
static int scf_ir = 0;          /* Index for Uplink */
static int scf_ig = 0;          /* Index for GateAddress */

/*
 * Zones, domains, and outbound directories
 */
struct st_zones {
    int zone;                   /* Zone */
    char *inet_domain;          /* Internet domain */
    char *ftn_domain;           /* FTN domain */
    char *out;                  /* Outbound subdir */
};

static struct st_zones scf_zones[MAXADDRESS];
static int scf_nzones = 0;      /* # of zones */
static int scf_izones = 0;      /* Index for cf_zones_trav() */

/*
 * Configured DOS drive -> UNIX path translation
 */
struct st_dos {
    char *drive;                /* MSDOS drive */
    char *path;                 /* UNIX path */
};

static struct st_dos scf_dos[MAXDOSDRIVE];
static int scf_ndos = 0;        /* # of DOS drives */

/*
 * FTN-Internet gateway
 */
static Node scf_gateway;

/*
 * All other, unknown config lines stored in linked list
 */
static cflist *scf_list_first = NULL;
static cflist *scf_list_last = NULL;

/*
 * Set config for gateway programs (ftn2rfc, rfc2ftn, ftnin, ftnmail),
 * basically reshuffling addresses a bit (using GateAddress as the main
 * address).
 */
void cf_i_am_a_gateway_prog(void)
{
    int i;

    if (scf_ig) {
        /* GateAddress used, new-style config, shuffle addresses */
        debug(8, "config: switching to gateway, using GateAddress");
        for (i = 0; i < scf_ig; i++) {
            scf_addr[i].uplink = scf_addr[i].addr;
            scf_addr[i].addr = scf_addr[i].gateaddr;
        }
        for (i = 0; i < scf_ig; i++)
            debug(8, "config: address Z%-4d: GATE addr=%s uplink=%s",
                  scf_addr[i].zone, znfp1(&scf_addr[i].addr),
                  znfp2(&scf_addr[i].uplink));
        return;
    }

    if (scf_ir == 0) {
        /* No Uplink used, same address for gateway and tosser, copy addr */
        debug(8, "config: no explicit uplink, using Address");
        for (i = 0; i < scf_ia; i++)
            scf_addr[i].uplink = scf_addr[i].addr;
        scf_ir = scf_ia;
        for (i = 0; i < scf_ia; i++)
            debug(8, "config: address Z%-4d: GATE addr=%s uplink=%s",
                  scf_addr[i].zone, znfp1(&scf_addr[i].addr),
                  znfp2(&scf_addr[i].uplink));
        return;
    }
}

/*
 * Check for corresponding "Address" and "Uplink" entries (config.gate)
 */
void cf_check_gate(void)
{
    if (scf_ia == 0) {
        fglog("ERROR: config: no Address");
        if (!verbose)
            fprintf(stderr, "ERROR: config: no Address\n");
        exit(EX_USAGE);
    }
#if 0
    if (scf_ir == 0 && scf_ig == 0) {
        fglog("ERROR: config: no Uplink or GateAddress");
        if (!verbose)
            fprintf(stderr, "ERROR: config: no Uplink or GateAddress\n");
        exit(EX_USAGE);
    }
#endif

    if (scf_ir && scf_ia != scf_ir)
        fglog("WARNING: config: #Address (%d) != #Uplink (%d)", scf_ia, scf_ir);
    if (scf_ig && scf_ia != scf_ig)
        fglog("WARNING: config: #Address (%d) != #GateAddress (%d)",
              scf_ia, scf_ig);
}

/*
 * debug output of configuration
 */
void cf_debug(void)
{
    int i;

    if (verbose < 8)
        return;

    debug(8, "config: fqdn=%s", scf_fqdn);

    for (i = 0; i < scf_naddr; i++)
        debug(8, "config: address Z%-4d: addr=%s uplink=%s gateaddr=%s",
              scf_addr[i].zone, znfp1(&scf_addr[i].addr),
              znfp2(&scf_addr[i].uplink), znfp3(&scf_addr[i].gateaddr));

    for (i = 0; i < scf_nzones; i++)
        debug(8, "config: zone %-4d: %s  %s  %s", scf_zones[i].zone,
              scf_zones[i].inet_domain, scf_zones[i].ftn_domain,
              scf_zones[i].out);

    debug(8, "config: gateway=%s", znfp1(&scf_gateway));
}

/*
 * Return main AKA (independent of current zone setting)
 */
Node *cf_main_addr(void)
{
    return &scf_addr[0].addr;
}

Node cf_n_main_addr(void)
{
    return scf_addr[0].addr;
}

/*
 * Return current main/fakenet/uplink FTN address
 */
Node *cf_addr(void)
{
    return &scf_c_addr;
}

Node *cf_uplink(void)
{
    return &scf_c_uplink;
}

Node cf_n_addr(void)
{
    return scf_c_addr;
}

Node cf_n_uplink(void)
{
    return scf_c_uplink;
}

#ifdef BEST_AKA
/*
 * Select best AKA
 */
void cf_set_best(int zone, int net, int node)
{
    int i;

    if (scf_naddr == 0) {
        fprintf(stderr, "No FTN addresses configured.\n");
        exit(1);
    }

    scf_zone = zone;
/* Z:N/F.x */
    for (i = 0; i < scf_naddr; i++)
        if ((zone == scf_addr[i].zone) && (net == scf_addr[i].addr.net)
            && (node == scf_addr[i].addr.node)) {
            scf_index = i;
            scf_c_addr = scf_addr[i].addr;
            scf_c_uplink = scf_addr[i].uplink;
            debug(9, "Select best AKA: %s  Uplink: %s",
                  znfp1(&scf_addr[i].addr), znfp2(&scf_addr[i].uplink));
            return;
        }
/* Z:N/x.x */
    for (i = 0; i < scf_naddr; i++)
        if ((zone == scf_addr[i].zone) && (net == scf_addr[i].addr.net)) {
            scf_index = i;
            scf_c_addr = scf_addr[i].addr;
            scf_c_uplink = scf_addr[i].uplink;
            debug(9, "Select best AKA: %s  Uplink: %s",
                  znfp1(&scf_addr[i].addr), znfp2(&scf_addr[i].uplink));
            return;
        }
/* Z:x/x.x */
    for (i = 0; i < scf_naddr; i++)
        if ((zone == scf_addr[i].zone)) {
            scf_index = i;
            scf_c_addr = scf_addr[i].addr;
            scf_c_uplink = scf_addr[i].uplink;
            debug(9, "Select best AKA: %s  Uplink: %s",
                  znfp1(&scf_addr[i].addr), znfp2(&scf_addr[i].uplink));
            return;
        }

    scf_index = i = 0;
    scf_c_addr = scf_addr[i].addr;
    scf_c_uplink = scf_addr[i].uplink;
    debug(9, "Select default AKA: %s  Uplink: %s",
          znfp1(&scf_addr[i].addr), znfp2(&scf_addr[i].uplink));
}
#else
void cf_set_best(int zone, int net, int node)
{
    cf_set_zone(zone);
}
#endif                          /* BEST_AKA */

/*
 * Set current zone
 */
void cf_set_zone(int zone)
{
    int i;

    if (scf_naddr == 0) {
        fprintf(stderr, "No FTN addresses configured.\n");
        exit(1);
    }

    scf_zone = zone;
    for (i = 0; i < scf_naddr; i++)
        if (zone == scf_addr[i].zone) {
            scf_index = i;
            scf_c_addr = scf_addr[i].addr;
            scf_c_uplink = scf_addr[i].uplink;
            debug(9, "Select Z%d AKA: %s  Uplink: %s",
                  scf_addr[i].zone,
                  znfp1(&scf_addr[i].addr), znfp2(&scf_addr[i].uplink));
            return;
        }

    scf_index = i = 0;
    scf_c_addr = scf_addr[i].addr;
    scf_c_uplink = scf_addr[i].uplink;
    debug(9, "Select default AKA: %s  Uplink: %s",
          znfp1(&scf_addr[i].addr), znfp2(&scf_addr[i].uplink));
}

/*
 * Set current address
 */
void cf_set_curr(Node * node)
{
    cf_set_best(node->zone, node->net, node->node);
    scf_c_addr = *node;
}

/*
 * Return current/default zone
 */
int cf_zone(void)
{
    return scf_zone;
}

int cf_defzone(void)
{
    return scf_addr[0].zone;
}

/*
 * Read line from config file. Strip `\n', leading spaces,
 * comments (starting with `#'), and empty lines. cf_getline() returns
 * a pointer to the first non-whitespace in buffer.
 */
static long cf_lineno = 0;

long cf_lineno_get(void)
{
    return cf_lineno;
}

long cf_lineno_set(long n)
{
    long old;

    old = cf_lineno;
    cf_lineno = n;

    return old;
}

char *cf_getline(char *buffer, int len, FILE * fp)
{
    char *p;

    while (fgets(buffer, len, fp)) {
        cf_lineno++;
        strip_crlf(buffer);
        for (p = buffer; *p && is_space(*p); p++) ; /* Skip white spaces */
        if (*p != '#')
            return p;
    }
    return NULL;
}

/*
 * Process line from config file
 */
static void cf_do_line(char *line)
{
    char *p, *keyword;
    Node a;

    keyword = xstrtok(line, " \t");
    if (!keyword)
        return;

    /***** include ******************************************************/
    if (!stricmp(keyword, "include")) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("config: missing include file");
            return;
        }
        cf_read_config_file(p);
    }
    /***** hostname *****************************************************/
    else if (!stricmp(keyword, "hostname")) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("config: missing hostname");
            return;
        }
        BUF_COPY(scf_hostname, p);
    }
    /***** domain *******************************************************/
    else if (!stricmp(keyword, "domain")) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("config: missing domainname");
            return;
        }
        if (p[0] != '.')
            BUF_COPY2(scf_domainname, ".", p);
        else
            BUF_COPY(scf_domainname, p);
        /* This is also the default for "HostsDomain" */
        BUF_COPY(scf_hostsdomain, scf_domainname);
    }
    /***** hostsdomain **************************************************/
    else if (!stricmp(keyword, "hostsdomain")) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("config: missing domainname");
            return;
        }
        if (p[0] != '.')
            BUF_COPY2(scf_hostsdomain, ".", p);
        else
            BUF_COPY(scf_hostsdomain, p);
    }
    /***** address ******************************************************/
    else if (!stricmp(keyword, "address")) {
        /* address */
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("config: missing address");
            return;
        }
        if (asc_to_node(p, &a, FALSE) == ERROR) {
            fglog("config: illegal address %s", p);
            return;
        }

        if (scf_ia < MAXADDRESS) {
            scf_addr[scf_ia].zone = a.zone;
            scf_addr[scf_ia].addr = a;
            scf_ia++;
        } else
            fglog("config: too many addresses");
    }
    /***** uplink *******************************************************/
    else if (!stricmp(keyword, "uplink")) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("config: missing address");
            return;
        }
        if (asc_to_node(p, &a, FALSE) == ERROR) {
            fglog("config: illegal address %s", p);
            return;
        }

        if (scf_ir < MAXADDRESS) {
            scf_addr[scf_ir].uplink = a;
            scf_ir++;
        } else
            fglog("config: too many addresses");
    }
    /***** GateAddress **************************************************/
    else if (!stricmp(keyword, "gateaddress")) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("config: missing address");
            return;
        }
        if (asc_to_node(p, &a, FALSE) == ERROR) {
            fglog("config: illegal address %s", p);
            return;
        }

        if (scf_ig < MAXADDRESS) {
            scf_addr[scf_ig].gateaddr = a;
            scf_ig++;
        } else
            fglog("config: too many addresses");
    }
    /***** zone *********************************************************/
    else if (!stricmp(keyword, "zone")) {
        int zone;
        char *inet, *ftn, *out;

        if (scf_nzones >= MAXADDRESS) {
            fglog("config: too many zones");
            return;
        }

        if (!(p = xstrtok(NULL, " \t"))) {
            fglog("config: missing zone");
            return;
        }
        if (!stricmp(p, "default"))
            zone = 0;
        else {
            zone = atoi(p);
            if (!zone) {
                fglog("config: illegal zone value %s", p);
                return;
            }
        }

        if (!(inet = xstrtok(NULL, " \t"))) {
            fglog("config: missing Internet domain");
            return;
        }

        if (!(ftn = xstrtok(NULL, " \t"))) {
            fglog("config: missing FTN domain");
            return;
        }

        if (!(out = xstrtok(NULL, " \t"))) {
            fglog("config: missing outbound directory");
            return;
        }

        scf_zones[scf_nzones].zone = zone;
        scf_zones[scf_nzones].inet_domain = strsave(inet);
        scf_zones[scf_nzones].ftn_domain = strsave(ftn);
        scf_zones[scf_nzones].out = strsave(out);
        scf_nzones++;
    }
    /***** dosdrive *****************************************************/
    else if (!stricmp(keyword, "dosdrive")) {
        char *drive, *path;

        if (scf_ndos >= MAXDOSDRIVE) {
            fglog("config: too many DOS drives");
            return;
        }
        if (!(drive = xstrtok(NULL, " \t"))) {
            fglog("config: missing DOS drive");
            return;
        }
        if (!(path = xstrtok(NULL, " \t"))) {
            fglog("config: missing UNIX path");
            return;
        }

        scf_dos[scf_ndos].drive = strsave(drive);
        scf_dos[scf_ndos].path = strsave(path);
        scf_ndos++;
    }
    /***** gateway ******************************************************/
    else if (!stricmp(keyword, "gateway")) {
        p = xstrtok(NULL, " \t");
        if (!p) {
            fglog("config: missing address");
            return;
        }
        if (asc_to_node(p, &a, FALSE) == ERROR) {
            fglog("config: illegal address %s", p);
            return;
        }
        scf_gateway = a;
    }
    /***** U n k n o w n ************************************************/
    else {
        cflist *pl;

        p = xstrtok(NULL, "\n");
        if (p)
            while (is_blank(*p))
                p++;

        pl = (cflist *) xmalloc(sizeof(cflist));

        pl->key = strsave(keyword);
        pl->string = p ? strsave(p) : "";
        pl->next = NULL;

        if (scf_list_first)
            scf_list_last->next = pl;
        else
            scf_list_first = pl;
        scf_list_last = pl;
    }
}

/*
 * Read config file
 */
void cf_read_config_file(char *name)
{
    FILE *cf;
    char *line;

    if (!name || !*name)        /* Empty string -> no config file */
        return;

    cf = xfopen(name, R_MODE_T);

    while ((line = cf_getline(buffer, BUFFERSIZE, cf)))
        cf_do_line(line);

    scf_naddr = scf_ia;
    scf_zone = scf_addr[0].zone;
    scf_index = 0;
    scf_c_addr = scf_addr[0].addr;
    scf_c_uplink = scf_addr[0].uplink;

    BUF_COPY2(scf_fqdn, scf_hostname, scf_domainname);

    fclose(cf);
}

/*
 * Init configuration
 */
void cf_initialize(void)
{
    char *p;

    /*
     * Check for real uid != effective uid (setuid installed FIDOGATE
     * programs), disable debug() output (-v on command line) and
     * environment variables in this case.
     */
    if (getuid() != geteuid()) {
        return;
    }

    /* Env FIDOGATE is alias for FIDOGATE_CONFIGDIR */
    if ((p = getenv("FIDOGATE")))
        cf_s_configdir(p);

    /* Code for reading FIDOGATE_xxx env vars generated by subst.pl */
#include "cf_env.c"

}

/*
 * Set FIDO address */
void cf_set_addr(char *addr)
{
    Node node;

    if (asc_to_node(addr, &node, FALSE) == ERROR) {
        Node *n = inet_to_ftn(addr);
        if (!n) {
            fprintf(stderr, "Illegal FIDO address %s\n", addr);
            exit(EX_USAGE);
        }
        node = *n;
    }

    scf_naddr = 1;
    scf_ia = 1;
    scf_ig = 0;
    scf_ir = 0;
    scf_addr[0].zone = node.zone;
    scf_addr[0].addr = node;
    scf_zone = node.zone;
    scf_index = 0;
    scf_c_addr = scf_addr[0].addr;
    scf_c_uplink = scf_addr[0].uplink;
}

/*
 * Set uplink FIDO address
 */
void cf_set_uplink(char *addr)
{
    Node node;
    int i;

    if (asc_to_node(addr, &node, FALSE) == ERROR) {
        Node *n = inet_to_ftn(addr);
        if (!n) {
            fprintf(stderr, "Illegal FIDO address %s\n", addr);
            exit(EX_USAGE);
        }
        node = *n;
    }

    for (i = 0; i < scf_naddr; i++)
        scf_addr[i].uplink = node;

    scf_ir = 1;
    scf_addr[0].uplink = node;
    scf_zone = scf_addr[0].zone;
    scf_index = 0;
    scf_c_addr = scf_addr[0].addr;
    scf_c_uplink = scf_addr[0].uplink;
}

/*
 * Return hostname / domain name / fully qualified domain name
 */
char *cf_hostname(void)
{
    return scf_hostname;
}

char *cf_domainname(void)
{
    return scf_domainname;
}

char *cf_hostsdomain(void)
{
    return scf_hostsdomain;
}

char *cf_fqdn(void)
{
    return scf_fqdn;
}

/***** Stuff for processing zone info ****************************************/

/*
 * Return Internet domain for a FIDO zone
 */
char *cf_zones_inet_domain(int zone)
{
    int i;

    /*
     * Search zone
     */
    for (i = 0; i < scf_nzones; i++)
        if (scf_zones[i].zone == zone)
            return scf_zones[i].inet_domain;

    /*
     * Search default domain
     */
    for (i = 0; i < scf_nzones; i++)
        if (scf_zones[i].zone == 0)
            return scf_zones[i].inet_domain;

    return FTN_INVALID_DOMAIN;
}

/*
 * Check for valid zone
 */
int cf_zones_check(int zone)
{
    short i;

    /*
     * Search zone
     */
    for (i = 0; i < scf_nzones; i++)
        if (scf_zones[i].zone && scf_zones[i].zone == zone)
            return TRUE;

    return FALSE;
}

/*
 * Traverse Internet domains
 */
char *cf_zones_trav(int first)
{
    if (first)
        scf_izones = 0;

    return scf_izones < scf_nzones ? scf_zones[scf_izones++].inet_domain : NULL;
}

/*
 * Return outbound directory for a FIDO zone
 */
char *cf_zones_out(int zone)
{
    short i;

    /*
     * Search zone
     */
    for (i = 0; i < scf_nzones; i++)
        if (scf_zones[i].zone == zone)
            return scf_zones[i].out;

    return NULL;
}

/*
 * Return outbound directory
 */
char *cf_out_get(short i)
{
    if (i < scf_nzones)
        return scf_zones[i].out;
    else
        return NULL;
}

/*
 * Return FTN domain name for a FTN zone
 */
char *cf_zones_ftn_domain(int zone)
{
    int i;

    /*
     * Search zone
     */
    for (i = 0; i < scf_nzones; i++)
        if (scf_zones[i].zone == zone)
            return scf_zones[i].ftn_domain;

    /*
     * Search default domain
     */
    for (i = 0; i < scf_nzones; i++)
        if (scf_zones[i].zone == 0)
            return scf_zones[i].ftn_domain;

    return "fidonet";
}

/*
 * Traverse FTN addresses
 */
Node *cf_addr_trav(int first)
{
    static int iaddr;

    if (first)
        iaddr = 0;

    if (iaddr >= scf_naddr)     /* End of addresses */
        return NULL;

    return &scf_addr[iaddr++].addr;
}

/*
 * UNIX to MSDOS file name translation enabled
 */
int cf_dos(void)
{
    return scf_ndos != 0;
}

/*
 * Convert UNIX path on server to MSDOS path on client
 */
char *cf_dos_xlate(char *name)
{
    static char buf[MAXPATH];
    int i;
    char *s;
    int len;

    for (i = 0; i < scf_ndos; i++) {
        len = strlen(scf_dos[i].path);
        if (!strncmp(name, scf_dos[i].path, len)) {
            BUF_COPY2(buf, scf_dos[i].drive, name + len);
            for (s = buf; *s; s++)
                switch (*s) {
                case '/':
                    *s = '\\';
                    break;
                }
            return buf;
        }
    }

    return NULL;
}

/*
 * Convert MSDOS path back to UNIX path
 */
char *cf_unix_xlate(char *name)
{
    static char buf[MAXPATH];
    int i;
    char *s;
    int len;

    for (i = 0; i < scf_ndos; i++) {
        len = strlen(scf_dos[i].drive);
        if (!strnicmp(name, scf_dos[i].drive, len)) {
            BUF_COPY2(buf, scf_dos[i].path, name + len);
            for (s = buf; *s; s++)
                switch (*s) {
                case '\\':
                    *s = '/';
                    break;
                }
            return buf;
        }
    }

    return NULL;
}

/*
 * Address of FTN-Internet gateway
 */
Node cf_gateway(void)
{
    return scf_gateway;
}

static int is_empty_string(char *str)
{
    assert(str != NULL);

    return str[0] == '\0';
}

static void cf_string_debug(char *name, char *val)
{
    int level = 8;
    char *empty_fmt = "config: %s";
    char *nonempty_fmt = "config: %s %s";

    if (is_empty_string(val))
        debug(level, empty_fmt, name);
    else
        debug(level, nonempty_fmt, name, val);
}

/*
 * Get config statement(s) string(s)
 */
char *cf_get_string(char *name, int first)
{
    static cflist *last_listp = NULL;
    char *string;

    if (first)
        last_listp = scf_list_first;

    while (last_listp) {
        if (!stricmp(last_listp->key, name)) {  /* Found */
            string = last_listp->string;
            last_listp = last_listp->next;
            cf_string_debug(name, string);
            return string;
        }
        last_listp = last_listp->next;
    }

    return NULL;
}

/*
 * Get config values for Organization, Origin
 */
char *cf_p_organization(void)
{
    static char *pval;

    if (!pval) {
        if (!(pval = cf_get_string("Organization", TRUE)))
            pval = "FIDOGATE";
    }

    return pval;
}

char *cf_p_origin(void)
{
    static char *pval;

    if (!pval) {
        if (!(pval = cf_get_string("Origin", TRUE)))
            pval = "FIDOGATE";
    }

    return pval;
}

cflist *config_first(void)
{
    return scf_list_first;
}

void config_free(void)
{

    cflist *p, *n;
    int i;

    for (p = scf_list_first; p; p = n) {
        n = p->next;

        xfree(p->key);
        if (strlen(p->string) > 0)
            xfree(p->string);
        p->next = NULL;

        xfree(p);
    }

    for (i = 0; i < scf_nzones; i++) {
        if (scf_zones[i].inet_domain)
            xfree(scf_zones[i].inet_domain);
        if (scf_zones[i].ftn_domain)
            xfree(scf_zones[i].ftn_domain);
        if (scf_zones[i].out)
            xfree(scf_zones[i].out);
    }

    for (i = 0; i < scf_ndos; i++) {
        if (scf_dos[i].drive)
            xfree(scf_dos[i].drive);
        if (scf_dos[i].path)
            xfree(scf_dos[i].path);
    }

}
