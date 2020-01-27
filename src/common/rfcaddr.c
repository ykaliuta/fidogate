/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * RFCAddr struct handling
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
 * DotNames option from config file: User.Name instead of User_Name
 */
static int dot_names = FALSE;
static char *fallback_username;

void rfcaddr_dot_names(void)
{
    dot_names = TRUE;
}

/*
 * RfcAddrMode option from config file: () <> or none;
 */
static int addr_mode = 0;       /* 0 = user@do.main (Real Name)
                                 * 1 = Real Name <user@do.main>
                                 * 2 = user@do.main             */

void rfcaddr_mode(int m)
{
    addr_mode = m;
}

void rfcaddr_fallback_username(char *name)
{
    fallback_username = name;
}

/*
 * rfcaddr_from_ftn() --- Generate RFCAddr struct from FTN name and node
 */
#define NOT_ALLOWED_ATOMS	"()<>@,;::\\\"[]"
#define NOT_ALLOWED_QTEXT	"\"\\"
/*#define NOT_ALLOWED_FULLNAME	"()<>"*/
#define NOT_ALLOWED_FULLNAME	"<>"

RFCAddr rfcaddr_from_ftn(char *name, Node * node)
{
    RFCAddr rfc;
    char buf[MAXUSERNAME];
    int i, c;
    char *p;
    int must_quote;

//    rfc.flags = 0;

    /*
     * Internet address part
     */
#ifdef LOCAL_FTN_ADDRESSES
    if (!hosts_lookup(node, NULL))
        BUF_COPY(rfc.addr, cf_fqdn());
    else
#endif
    if (node->zone == -1)
        BUF_COPY(rfc.addr, FTN_INVALID_DOMAIN);
    else
        str_ftn_to_inet(rfc.addr, sizeof(rfc.addr), node, FALSE);

    /* RFC name part must be 7 bit only */
    if (!charset_is_7bit(name, strlen(name)) && fallback_username) {
        strncpy(rfc.user, fallback_username, sizeof(rfc.user) - 1);
        rfc.user[sizeof(rfc.user) - 1] = '\0';
        goto skip_name_check;
    }

    /*
     * Translate special chars and removed ctrl chars
     */
    for (i = 0, p = name; *p && i < MAXUSERNAME - 1;) {
        c = (unsigned char)(*p++);
        if (c < ' ')
            c = '_';
        buf[i++] = c;
    }
    buf[i] = 0;

    /*
     * Check name for characters not allowed in RFC 822 atoms,
     * requiring that name must be quoted with "...". Copying name
     * to rfc.user[].
     */
    must_quote = FALSE;
    for (p = buf; *p; p++)
        if (strchr(NOT_ALLOWED_ATOMS, *p))
            must_quote = TRUE;

    i = 0;
    if (must_quote)
        rfc.user[i++] = '\"';   /* " makes C-mode happy */
    for (p = buf; i < MAXUSERNAME - 2 && *p; p++)
        if (!strchr(NOT_ALLOWED_QTEXT, *p)) {
            rfc.user[i] = *p;
            if (rfc.user[i] == ' ')
                rfc.user[i] = dot_names ? '.' : '_';
            i++;
        }
    if (must_quote)
        rfc.user[i++] = '\"';   /* " makes C-mode happy */
    rfc.user[i] = 0;

 skip_name_check:

#ifdef LOCAL_FTN_ADDRESSES
    if (!hosts_lookup(node, NULL))
        /* Add "%p.f.n.z" to user name */
        BUF_APPEND2(rfc.user, "%", node_to_pfnz(node));
#endif

    /*
     * Copy ftn name to real name field
     */
    i = 0;
    for (p = name; i < (sizeof(rfc.real) - 1) && *p; p++)
        if (!strchr(NOT_ALLOWED_FULLNAME, *p))
            rfc.real[i++] = *p;
    rfc.real[i] = 0;

    /* Remove trailing spaces */
    for (; i >= 0 && rfc.real[i] == ' '; i--)
        rfc.real[i] = '\0';

    return rfc;
}

/*
 * rfcaddr_from_rfc() --- Generate RFCAddr struct from RFC address
 *
 * Supported formats:
 *     user@domain
 *     user@domain (Full Name)
 *     Full Name <user@domain>
 */
RFCAddr rfcaddr_from_rfc(char *addr)
{
    RFCAddr rfc;
    char bufn[MAXUSERNAME];
    char bufa[MAXINETADDR];
    char *p, *s, *r;
    int i;

//    rfc.flags = 0;

    /*
     * Full name <user@domain>
     */
    if ((p = strchr(addr, '<')) && (r = strrchr(p + 1, '>'))) {
        /* Full name */
        for (s = addr; is_space(*s); s++) ;
        for (i = 0; i < MAXUSERNAME - 1 && *s && *s != '<'; i++, s++)
            bufn[i] = *s;
        bufn[i] = 0;
        /* Address */
        for (i = 0, s = p + 1; i < MAXINETADDR - 1 && *s && s < r; i++, s++)
            bufa[i] = *s;
        bufa[i] = 0;
    }
    /*
     * user@domain (Full Name)
     */
    else if ((p = strchr(addr, '(')) && (r = strrchr(p + 1, ')'))) {
        /* Full name */
        for (i = 0, s = p + 1; i < MAXUSERNAME - 1 && *s && s < r; i++, s++)
            bufn[i] = *s;
        bufn[i] = 0;
        /* Address */
        for (s = addr; is_space(*s); s++) ;
        for (i = 0; i < MAXINETADDR - 1 && *s && *s != '('; i++, s++)
            bufa[i] = *s;
        bufa[i] = 0;
    }
    /*
     * user@domain
     */
    else {
        /* No full name */
        bufn[0] = 0;
        /* Address */
        for (s = addr; is_space(*s); s++) ;
        for (i = 0; i < MAXINETADDR - 1 && *s; i++, s++)
            bufa[i] = *s;
        bufa[i] = 0;
    }

    /*
     * Copy full name to RFCAddr struct, remove surrounding "..."
     * and leading/trailing spaces
     */
    p = bufn;
    if (*p == '\"')             /* " makes C-mode happy */
        p++;
    while (is_space(*p))
        p++;
    i = strlen(p) - 1;
    while (i >= 0 && p[i] == ' ')
        p[i--] = 0;
    if (i >= 0 && p[i] == '\"') /* " makes C-mode happy */
        p[i--] = 0;
    while (i >= 0 && p[i] == ' ')
        p[i--] = 0;
    BUF_COPY(rfc.real, p);

    /*
     * Removed leading/trailing spaces from address
     */
    for (p = bufa; is_space(*p); p++) ;
    for (i = strlen(p) - 1; i >= 0 && is_space(p[i]); p[i--] = 0) ;

    /*
     * Address type  user@domain
     */
    if ((r = strrchr(p, '@'))) {
        /* User name */
        for (i = 0, s = p; i < MAXUSERNAME - 1 && *s && s < r; i++, s++)
            rfc.user[i] = *s;
        rfc.user[i] = 0;
        /* Internet address */
        for (i = 0, s = r + 1; i < MAXINETADDR - 1 && *s; i++, s++)
            rfc.addr[i] = *s;
        rfc.addr[i] = 0;
    }
    /*
     * Address type  host!user
     */
    else if ((r = strchr(p, '!'))) {
        /* User name */
        for (i = 0, s = r + 1; i < MAXUSERNAME - 1 && *s; i++, s++)
            rfc.user[i] = *s;
        rfc.user[i] = 0;
        /* Internet address */
        for (i = 0, s = p; i < MAXINETADDR - 1 && *s && s < r; i++, s++)
            rfc.addr[i] = *s;
        rfc.addr[i] = 0;
    }
    /*
     * Address type  user%domain
     */
    else if ((r = strrchr(p, '%'))) {
        /* User name */
        for (i = 0, s = p; i < MAXUSERNAME - 1 && *s && s < r; i++, s++)
            rfc.user[i] = *s;
        rfc.user[i] = 0;
        /* Internet address */
        for (i = 0, s = r + 1; i < MAXINETADDR - 1 && *s; i++, s++)
            rfc.addr[i] = *s;
        rfc.addr[i] = 0;
    }
    /*
     * Adress type  user
     */
    else {
        /* User name */
        BUF_COPY(rfc.user, p);
        /* Internet address */
        rfc.addr[0] = 0;
    }

    return rfc;
}

/*
 * s_rfcaddr_to_asc() --- Text representation of RFCAddr
 */
char *s_rfcaddr_to_asc(RFCAddr * rfc, int real_flag)
    /* TRUE=with real name, FALSE=without */
{
    if (real_flag && rfc->real[0]) {
        if (addr_mode == 0) {
            /* user@do.main (Real Name) */
            return s_printf("%s%s%s (%s)",
                            rfc->user, rfc->addr[0] ? "@" : "",
                            rfc->addr, rfc->real);
        }
        if (addr_mode == 1) {
            /* Real Name <user@do.main> */
            return s_printf("%s <%s%s%s>",
                            rfc->real,
                            rfc->user, rfc->addr[0] ? "@" : "", rfc->addr);
        }
    }

    /* Default, no real name: user@do.main */
    return s_printf("%s%s%s", rfc->user, rfc->addr[0] ? "@" : "", rfc->addr);
}

#ifdef TEST /****************************************************************/

int main(int argc, char *argv[])
{
#if 0
    Node node;
    char *name;
    RFCAddr rfc;

    cf_initialize();
    cf_read_config_file(CONFIG_GATE);
    hosts_init();

    if (argc != 3) {
        fprintf(stderr, "usage: testrfc name Z:N/F.P\n");
        exit(1);
    }

    name = argv[1];
    if (asc_to_node(argv[2], &node, FALSE) == ERROR) {
        fprintf(stderr, "testrfc: invalid FTN address\n");
        exit(1);
    }

    rfc = rfcaddr_from_ftn(name, &node);

    printf("RFCAddr: %s@%s (%s)\n", rfc.user, rfc.addr, rfc.real);
#endif

#if 1
    char *addr;
    RFCAddr rfc;

    cf_initialize();
    cf_read_config_file(CONFIG_GATE);
    hosts_init();

    if (argc != 2) {
        fprintf(stderr, "usage: testrfc addr");
        exit(1);
    }

    addr = argv[1];

    rfc = rfcaddr_from_rfc(addr);
    printf("RFCAddr: %s@%s (%s)\n", rfc.user, rfc.addr, rfc.real);
#endif

    exit(0);
}

#endif /**TEST***************************************************************/
