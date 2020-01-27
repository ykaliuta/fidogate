/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway software UNIX <-> FIDO
 *
 *
 * Parsing and conversion for FIDO and RFC addresses
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
 *  _____ _____
 * |	 |___  |   Martin Junius	     FIDO:	2:2452/110
 * | | | |   | |   Radiumstr. 18  	     Internet:	mj@fidogate.org
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FIDOGATE; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/

#include "fidogate.h"

/*
 * Local prototypes
 */
static int try_pfnz(Node *, char *, char *, char *);

/*
 * Address parsing error message
 */
char address_error[ADDRESS_ERROR_SIZE];

static char **addr_is_local_pats = NULL;

/*
 * str_ftn_to_inet(): convert FTN address to Internet address
 *
 * force==TRUE: generate `pX.' point address even if `-p' is not
 *              specified in HOSTS.
 */
char *str_ftn_to_inet(char *buf, size_t len, Node * node, int force)
{

    Host *h;
    int point;

    h = hosts_lookup(node, NULL);

    if (h) {                    /* Address found in HOSTS */
        point = h->flags & HOST_POINT ? TRUE : force;
        if (h->name) {
            if (point && node->point > 0 && !h->node.point)
                str_printf(buf, len, "p%d.%s", node->point, h->name);
            else
                str_printf(buf, len, "%s", h->name);
        } else
            str_printf(buf, len, "%s%s", node_to_pfnz(node), cf_hostsdomain());
    } else
        str_printf(buf, len, "%s%s", node_to_pfnz(node),
                   cf_zones_inet_domain(node->zone));

    return buf;
}

/*
 * s_ftn_to_inet(): convert FTN address to Internet address
 */
char *s_ftn_to_inet(Node * node)
{
    TmpS *s;

    s = tmps_alloc(MAXINETADDR);
    str_ftn_to_inet(s->s, s->len, node, TRUE);
    return s->s;
}

#ifdef AI_1
/*
 * verify_host_flag(): Verify FTN host flag
 *
 */
int verify_host_flag(Node * node, int flag)
{
    Host *h;
    char HOST_ADDR;

    h = hosts_lookup(node, NULL);
    if (h && h->flags & flag)   /* Address found in HOSTS */
        return TRUE;
    return FALSE;
}
#endif

/*
 * inet_to_ftn(): convert Internet address to FTN address
 */
static int try_pfnz(Node * node, char *addr, char *dot, char *domain)
{
    char dom[MAXINETADDR];
    char adr[MAXINETADDR];

    int len = strlen(addr);
    int dlen = strlen(dot) + strlen(domain);

    BUF_COPY(adr, addr);
    BUF_COPY2(dom, dot, domain);

    if (len > dlen && !stricmp(adr + len - dlen, dom)) {
        adr[len - dlen] = 0;
        if (pfnz_to_node(adr, node) == OK)
            return TRUE;
    }

    return FALSE;
}

Node *inet_to_ftn(char *addr)
{

    char buf[MAXINETADDR];
    static Node node;
    Host *host;
    char *p, *pn;
    int point = -1;

    /*
     * Look for optional point addressing in front of host name
     */
    p = addr;
    if (*p == 'p' || *p == 'P') {
        p++;
        pn = p;
        while (*p && is_digit(*p))
            p++;
        if (*p == '.') {        /* Must end with '.' */
            p++;
            point = atoi(pn);   /* Point number */
        } else
            p = addr;           /* No pX. */
    }

    /*
     * 1. Lookup in HOSTS
     */
    if ((host = hosts_lookup(NULL, p))) {
        node = host->node;
        if (!node.point && point != -1)
            node.point = point;
        return &node;
    }

    /*
     * 2. Add domainname and lookup in HOSTS
     */
    BUF_COPY2(buf, p, cf_domainname());
    if ((host = hosts_lookup(NULL, buf))) {
        node = host->node;
        if (!node.point && point != -1)
            node.point = point;
        return &node;
    }

    /*
     * 2a. Add hosts domainname and lookup in HOSTS
     */
    BUF_COPY2(buf, p, cf_hostsdomain());
    if ((host = hosts_lookup(NULL, buf))) {
        node = host->node;
        if (!node.point && point != -1)
            node.point = point;
        return &node;
    }

    BUF_COPY(buf, addr);

    /*
     * 3. Try p.f.n.z
     */
    if (try_pfnz(&node, buf, "", ""))
        return &node;

    /*
     * 4. Try p.f.n.z.HOSTNAME
     */
    if (try_pfnz(&node, buf, ".", cf_hostname()))
        return &node;

    /*
     * 5. Try p.f.n.z.HOSTNAME.DOMAIN
     */
    if (try_pfnz(&node, buf, ".", cf_fqdn()))
        return &node;

    /*
     * 6. Try p.f.n.z.DOMAIN
     */
    if (try_pfnz(&node, buf, "", cf_domainname()))
        return &node;

    /*
     * 6a. Try p.f.n.z.HOSTSDOMAIN
     */
    if (try_pfnz(&node, buf, "", cf_hostsdomain()))
        return &node;

    /*
     * 7. Try FTN domains from CONFIG
     */
    for (p = cf_zones_trav(TRUE); p; p = cf_zones_trav(FALSE))
        if (try_pfnz(&node, buf, "", p))
            return &node;

    /*
     * Everything failed - not an FTN address
     */
    return NULL;
}

/*
 * Check for local RFC address, i.e. "user@HOSTNAME.DOMAIN (Full Name)"
 * or "user (Full Name)"
 */
int addr_is_local(char *addr)
{

    RFCAddr rfc;

    if (!addr)
        return FALSE;

    rfc = rfcaddr_from_rfc(addr);

    debug(7, "addr_is_local(): From=%s FQDN=%s",
          s_rfcaddr_to_asc(&rfc, TRUE), cf_fqdn());
    return rfc.addr[0] == '\0' || stricmp(rfc.addr, cf_fqdn()) == 0;
}

void addr_is_local_xpost_init(char *addr)
{
    list_init(&addr_is_local_pats, addr);
}

int addr_is_local_xpost(char *addr)
{

    RFCAddr rfc;
    char ailx;
    static char **addr_list = NULL;

    if (!addr)
        return FALSE;

    rfc = rfcaddr_from_rfc(addr);

    debug(7, "addr_is_local_xpost(): From=%s FQDN=%s",
          s_rfcaddr_to_asc(&rfc, TRUE), cf_fqdn());
    ailx = rfc.addr[0] == '\0' || stricmp(rfc.addr, cf_fqdn()) == 0;
    if (!ailx) {
        list_init(&addr_list, addr);
        ailx = list_match(addr_is_local_pats, addr_list);
    }
    return ailx;
}

/*
 * Check for address in local domain, i.e. "user@*DOMAIN (Full Name)"
 * or "user (Full Name)"
 */
int addr_is_domain(char *addr)
{

    RFCAddr rfc;
    char *d;
    int l, ld, ret = TRUE;

    if (addr) {

        rfc = rfcaddr_from_rfc(addr);

        d = cf_domainname();
        ld = strlen(d);
        l = strlen(rfc.addr);

        debug(7, "addr_is_domain(): From=%s domain=%s",
              s_rfcaddr_to_asc(&rfc, TRUE), d);

        if (rfc.addr[0] == '\0') ;
        /* user@DOMAIN */
        else if (*d == '.' && stricmp(rfc.addr, d + 1) == 0) ;
        else if (stricmp(rfc.addr, d) == 0) ;
        /* user@*.DOMAIN */
        else if (ld > l)
            ret = FALSE;
        else
            ret = stricmp(rfc.addr + l - ld, d) == 0;
    }
    return ret;
}
