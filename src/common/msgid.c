/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * MSGID <-> Message-ID conversion handling. See also ../doc/msgid.doc
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
 *****************************************************************************
 * As an exception to this rule you may freely use the functions
 * contained in THIS module to implement the FIDO-Gatebau '94 specs
 * for FIDO-Internet gateways without making the resulting program
 * subject to the GNU General Public License.
 *****************************************************************************/

#include "fidogate.h"

/*
 * Prototypes
 */
#ifndef FIDO_STYLE_MSGID
static void msgid_fts9_quote(char *, char *, int);
#endif                          /* FIDO_STYLE_MSGID */
static void msgid_mime_quote(char *, char *, int);
static char *msgid_domain(int);

/*
 * Quote string containing <SPACE> according to FTS-0009
 */
#ifndef FIDO_STYLE_MSGID
static void msgid_fts9_quote(char *d, char *s, int n)
{
    int i = 0;
    int must_quote = FALSE;

    must_quote = strchr(s, ' ') || strchr(s, '\"'); /* " */

    if (must_quote)
        d[i++] = '\"';          /* " */
    for (; i < n - 3 && *s; i++, s++) {
        if (*s == '\"')         /* " */
            d[i++] = '\"';      /* " */
        d[i] = *s;
    }
    if (must_quote)
        d[i++] = '\"';          /* " */
    d[i] = 0;
}
#endif                          /* FIDO_STYLE_MSGID */

/*
 * Quote ^AMSGID string using MIME-style quoted-printable =XX
 */
static void msgid_mime_quote(char *d, char *s, int n)
{
    int i, c;

    for (i = 0; i < n - 4 && *s && *s != '\r'; i++, s++) {
        c = *s & 0xff;
        if (c == ' ')
            d[i] = '_';
        else if (strchr("()<>@,;:\\\"[]/=_", c) || c >= 0x7f || c < 0x20) {
            str_printf(d + i, 4, "=%02X", c);
            i += 2;
        } else
            d[i] = c;
    }
    d[i] = 0;
}

/*
 * Return Message-ID domain for FTN zone
 */
static char *msgid_domain(int zone)
{
    char *d;

    d = zone >= 1
        && zone <= 6 ? MSGID_FIDONET_DOMAIN : cf_zones_inet_domain(zone);
    if (*d == '.')
        d++;

    return d;
}

/*
 * Convert FIDO ^AMSGID/REPLY to RFC Message-ID/References
 */
char *s_msgid_fido_to_rfc(char *msgid, int *pzone, short mail, char *ref_line)
{
    char *save;
    char *origaddr, *serialno;
    char *p, *s;
    Node idnode;
    int zone;
    TmpS *tmps;

    save = strsave(msgid);

    /*
     * Retrieve `origaddr' part
     */
    if (*save == '\"') {        /* " */
        /*
         * Quoted: "abc""def" -> abc"def
         */
        origaddr = save;
        p = save;
        s = save + 1;
        while (*s) {
            if (*s == '\"') {   /* " */
                if (*(s + 1) == '\"')   /* " */
                    s++;
                else
                    break;
            }
            *p++ = *s++;
        }
        if (*s == '\"')         /* " */
            s++;
        while (*s && is_space(*s))
            s++;
        *p = 0;
        serialno = s;
    } else {
        /*
         * Not quoted
         */
        origaddr = save;
        for (p = save; *p && !is_space(*p); p++) ;
        s = p;
        while (*s && is_space(*s))
            s++;
        *p = 0;
        serialno = s;
    }

    /*
     * Retrieve `serialno' part
     */
    for (p = serialno; *p && !is_space(*p); p++) ;
    *p = 0;

#if defined(DBC_HISTORY) && defined(FIDO_STYLE_MSGID)
    /***** Parse dbc for msgid ******/
    if (!pzone && mail && strchr(msgid, ' ')) {
        if (lock_program(cf_p_lock_history(), FALSE) == ERROR)
            return NULL;
        if (hi_init_dbc() == ERROR)
            return NULL;
        s = hi_fetch(msgid, 0);
        hi_close();
        unlock_program(cf_p_lock_history());
        if (s) {
            return s;
        } else {
#endif                          /* DBC_HISTORY && FIDO_STYLE_MSGID */
            if (ref_line)
                return ref_line;
#if defined(DBC_HISTORY) && defined(FIDO_STYLE_MSGID)
        }
    }
#endif
    /***** New-style converted RFC Message-ID *****/
    if (wildmat(origaddr, "<*@*>")) {
        tmps = tmps_copy(origaddr);
        xfree(save);
        if (pzone)
            *pzone = -2;
        return tmps->s;
    }

       /***** FTN-style *****/

    /*
     * Search for parsable FTN address in origaddr
     */
    for (p = origaddr; *p && !is_digit(*p); p++) ;
    for (s = p; *s && (is_digit(*s) || *s == ':' || *s == '/' || *s == '.');
         s++) ;
    *s = 0;
    if (asc_to_node(p, &idnode, TRUE) != ERROR) {   /* Address found */
        zone = idnode.zone;
        if (pzone)
            *pzone = zone;
    } else {
        zone = cf_zone();
        if (pzone)
            *pzone = -1;
    }

    /*
     * New-style FTN Message-IDs using MIME quoted-printable
     */
    tmps = tmps_alloc(2 * MAXINETADDR);

    str_copy(tmps->s, tmps->len, "<MSGID_");
    msgid_mime_quote(tmps->s + strlen(tmps->s), msgid,
                     tmps->len - strlen(tmps->s));
    str_append(tmps->s, tmps->len, "@");
    str_append(tmps->s, tmps->len, msgid_domain(zone));
    str_append(tmps->s, tmps->len, ">");

    xfree(save);
    return tmps->s;
}

/*
 * Generate ID for FIDO messages without ^AMSGID, using date and CRC over
 * From, To and Subject.
 */
char *s_msgid_default(Message * msg)
{
    /*
     * Compute CRC for strings from, to, subject
     */
    crc32_init();
    crc32_compute((unsigned char *)msg->name_from, strlen(msg->name_from));
    crc32_compute((unsigned char *)msg->name_to, strlen(msg->name_to));
    crc32_compute((unsigned char *)msg->subject, strlen(msg->subject));

    return s_printf("<NOMSGID_%d=3A%d=2F%d.%d_%s_%08lx@%s>",
                    msg->node_orig.zone, msg->node_orig.net,
                    msg->node_orig.node, msg->node_orig.point,
                    date("%y%m%d_%H%M%S", &msg->date), crc32_value(),
                    msgid_domain(msg->node_orig.zone));
}

/*
 * Convert RFC Message-ID/References to FIDO ^AMSGID/^AREPLY
 */
#ifdef FIDO_STYLE_MSGID
char *s_msgid_rfc_to_fido(int *origid_flag, char *message_id,
                          int part, int split, char *area, short int dont_flush,
                          int for_reply)
#else
char *s_msgid_rfc_to_fido(int *origid_flag, char *message_id,
                          int part, int split, char *area, short int x_flags_m,
                          int for_reply)
#endif
/* origid_flag - Flag for ^AORIGID       */
/* message_id  - Original RFC-ID         */
/* part        - part number             */
/* split       - != 0 # of parts         */
/* area        - FTN AREA                */
/* dont_flush  - Do'nt flush DBC History */
/* for_reply   - Id will be ^AREPLY      */
/* x_flags_m   - X-Flags: m              */
{
    char *id, *host, *p;
    char *savep;
    Node node, *n;
    int hexflag, i;
    char hexid[16];
    unsigned long crc32;
    TmpS *tmps;

    /****** Extract id and host from <id@host> *****/

    savep = strsave(message_id);
    /*
     * Format of message_id is "<identification@host.domain> ..."
     * We want the the last one in the chain, which is the message id
     * of the article replied to.
     */
    id = strrchr(savep, '<');
    if (!id) {
        xfree(savep);
        return NULL;
    }
    id++;
    host = strchr(id, '@');
    if (!host) {
        xfree(savep);
        return NULL;
    }
    *host++ = 0;
    p = strchr(host, '>');
    if (!p) {
        xfree(savep);
        return NULL;
    }
    *p = 0;

    /*
     * Don't convert <funpack....@...> and <NOMSGID-...@...> IDs
     * generated by FIDOGATE
     */
    if (!strncmp(id, "funpack", 7) || !strncmp(id, "NOMSGID_", 8)) {
        xfree(savep);
        return NULL;
    }

                                                  /***** Check for old style FTN Message-IDs <abcd1234%domain@p.f.n.z> *****/
    if (!split) {
        /*
         * First check ID. A FIDO Message-ID is a hex number with an
         * optional %Domain string. The hex number must not start with
         * `0' and it's length must be no more then 8 digits.
         */
        node.domain[0] = 0;
        p = id;
        hexflag = isxdigit(*p) && *p != '0';
        for (p++, i = 0; i < 7 && *p && *p != '%'; i++, p++)
            if (!isxdigit(*p))
                hexflag = FALSE;
        if (hexflag && *p == '%') { /* Domain part follows */
            *p++ = 0;
            BUF_COPY(node.domain, p);
        } else if (*p)
            hexflag = FALSE;
        if (hexflag) {
            /* Pad with leading 0's */
            str_copy(hexid, sizeof(hexid), "00000000");
            str_copy(hexid + 8 - strlen(id), sizeof(hexid) - 8 + strlen(id),
                     id);

            /* host must be an FTN address */
            if ((n = inet_to_ftn(host))) {
                /* YEP! This is an old-style FTN Message-ID!!! */
                node.zone = n->zone;
                node.net = n->net;
                node.node = n->node;
                node.point = n->point;
                tmps = tmps_printf("%s %s", znfp1(&node), hexid);
                xfree(savep);
                if (origid_flag)
                    *origid_flag = FALSE;
                return tmps->s;
            }
        }
    }   /**if(!split)**/

        /***** Check for new-style <MSGID_mimeanything@domain> *****/
    if (!strncmp(id, "MSGID_", 6)) {
        p = id + strlen("MSGID_");
        tmps = tmps_alloc(strlen(id) + 1);
        mime_dequote(tmps->s, tmps->len, p);
        xfree(savep);
        if (origid_flag)
            *origid_flag = FALSE;
        return tmps->s;
    }

        /***** Generate ^AMSGID according to msgid.doc specs *****/
    /*
     * New-style FIDO-Gatebau '94 ^AMSGID
     */
    xfree(savep);
    savep = strsave(message_id);
    id = strrchr(savep, '<');
    if (!id)
        id = savep;
    p = strchr(id, '>');
    if (!p)
        p = id;
    else
        p++;
    *p = 0;

    crc32_init();
    crc32_compute((unsigned char *)id, strlen(id));
    if (area)
        crc32_compute((unsigned char *)area, strlen(area));
    crc32 = crc32_value();
    if (split)
        crc32 += part - 1;

    tmps = tmps_alloc(strlen(id) + 1 + /**Extra**/ 20);
#ifndef FIDO_STYLE_MSGID
    if (!x_flags_m) {
        msgid_fts9_quote(tmps->s, id, tmps->len);
        str_printf(tmps->s + strlen(tmps->s), tmps->len - strlen(tmps->s),
                   " %08lx", crc32);
    } else {
        if (for_reply)
            str_printf(tmps->s, strlen(tmps->s) + strlen(id) + 2, "%s ", id);
        str_printf(tmps->s + strlen(tmps->s), tmps->len - strlen(tmps->s),
                   "%08lx", crc32);
    }
#else
    if (for_reply)
        str_printf(tmps->s, strlen(tmps->s) + strlen(id) + 2, "%s ", id);
    str_printf(tmps->s + strlen(tmps->s), tmps->len - strlen(tmps->s),
               "%08lx", crc32);
#endif                          /* !FIDO_STYLE_MSGID */
    xfree(savep);
    if (origid_flag)
        *origid_flag = TRUE;
#if defined(DBC_HISTORY) && defined(FIDO_STYLE_MSGID)
    if (area) {
        if (lock_program(cf_p_lock_history(), FALSE) == ERROR) {
            return tmps->s;
        }
        if (hi_init_dbc() == ERROR) {
            fglog("can't open dbc file");
        }
        if (hi_write_dbc(message_id, tmps->s, dont_flush) == ERROR)
            fglog("can't write to dbc file");
        hi_close();
        unlock_program(cf_p_lock_history());
    }
#endif                          /* DBC_HISTORY && FIDO_STYLE_MSGID */
    return tmps->s;
}

/*
 * Extract Message-ID from ^AORIGID/^AORIGREF with special handling for
 * split messages (appended " i/n"). Returns NULL for invalid ^AORIGID.
 */
char *s_msgid_convert_origid(char *origid)
{
    char *s, *p, *id;
    TmpS *tmps;

    s = strsave(origid);

    id = s;
    p = strrchr(s, '>');
    if (!p) {
        xfree(s);
        debug(1, "Invalid ^AORIGID: %s", origid);
        return NULL;
    }

    p++;
    if (is_space(*p)) {
        /*
         * Indication of splitted message " p/n" follows ...
         */
        *p++ = 0;
        while (is_space(*p))
            p++;
    }

    /*
     * Message-IDs must NOT contain white spaces
     */
    if (strchr(id, ' ') || strchr(id, '\t')) {
        xfree(s);
        debug(1, "Invalid ^AORIGID: %s", origid);
        return NULL;
    }

    tmps = tmps_copy(id);
    xfree(s);
    return tmps->s;
}
