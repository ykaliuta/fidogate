/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Reading and processing FTN text body
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
 * Read one "line" from FTN text body. A line comprises arbitrary
 * characters terminated with a CR '\r'. None, one, or many LFs '\n'
 * may follow:
 *
 *    x x x x x x x x x \r [\n...]
 *
 * This function checks for orphan 0-bytes in the message body and
 * recognizes grunged messages produced by SQUISH 1.01.
 *
 * Return values:
 *     -1  ERROR     an error occured
 *      1            next line follows
 *      0  MSG_END   end of message, end of packet
 *      2  MSG_TYPE  end of message, next message header follows
 */
int pkt_get_line(FILE * fp, char *buf, int size)
{
    char *p;
    int c, c1, c2;
    char read_lf = FALSE;
    long pos;

    p = buf;

    while (size > 3) {          /* Room for current + 2 extra chars */
        c = getc(fp);

        if (read_lf && c != '\n') { /* No more LFs, this is end of line */
            ungetc(c, fp);
            *p = 0;
            return 1;
        }

        switch (c) {
        case EOF:              /* premature EOF */
            return ERROR;

        case 0:                /* end of message or orphan */
            c1 = getc(fp);
            c2 = getc(fp);
            if (c1 == EOF || c2 == EOF)
                return ERROR;
            if (c2 == 0) {      /* end of message */
                if (c1 == 2) {
                    *p = 0;
                    return MSG_TYPE;
                }
                if (c1 == 0) {  /* end of packet */
                    *p = 0;
                    return MSG_END;
                }
            }
            /* orphan 0-byte, skip */
            pos = ftell(fp);
            if (pos == ERROR)
                fglog("pkt_get_line(): orphan 0-char (can't determine offset)");
            else
                fglog("pkt_get_line(): orphan 0-char (offset=%ld)", pos);
            if (c1) {
                size--;
                *p++ = c1;
            }
            if (c2) {
                size--;
                *p++ = c2;
            }
            continue;
            break;

#if 1 /***** Work around for a SQUISH bug **********************************/
        case 2:                /* May be grunged packet: start of */
            c1 = getc(fp);      /* new message                     */
            if (c1 == EOF)
                return ERROR;
            if (c1 == 0) {      /* Looks like it is ... */
                *p = 0;
                fglog("pkt_get_line(): grunged packet");
                return MSG_TYPE;
            }
            *p++ = c;
            *p++ = c1;
            size--;
            size--;
            break;

#endif /********************************************************************/

        case '\r':             /* End of line */
            read_lf = TRUE;
        /**fall thru**/

        default:
            *p++ = c;
            size--;
            break;
        }
    }

    /* buf too small */
    *p = 0;
    return 1;
}

/*
 * Read text body from packet into Textlist
 *
 * Return values:
 *     -1  ERROR     an error occured
 *      0  MSG_END   end of packet
 *      2  MSG_TYPE  next message header follows
 */
int pkt_get_body(FILE * fp, Textlist * tl)
{
    int type;

    tl_clear(tl);

    /* Read lines and put into textlist */
    while ((type = pkt_get_line(fp, buffer, sizeof(buffer))) == 1)
        tl_append(tl, buffer);
    /* Put incomplete last line into textlist, if any */
    if ((type == MSG_END || type == MSG_TYPE) && buffer[0]) {
        /* Make sure that this line is terminated by \r\n */
        BUF_APPEND(buffer, "\r\n");

        tl_append(tl, buffer);
    }

    return type;
}

/*
 * Initialize MsgBody
 */
void msg_body_init(MsgBody * body)
{
    body->area = NULL;
    tl_init(&body->kludge);
    tl_init(&body->rfc);
    tl_init(&body->body);
    body->tear = NULL;
    body->origin = NULL;
    tl_init(&body->seenby);
    tl_init(&body->path);
    tl_init(&body->via);
}

/*
 * Clear MsgBody
 */
void msg_body_clear(MsgBody * body)
{
    xfree(body->area);
    body->area = NULL;
    tl_clear(&body->kludge);
    tl_clear(&body->rfc);
    tl_clear(&body->body);
    xfree(body->tear);
    body->tear = NULL;
    xfree(body->origin);
    body->origin = NULL;
    tl_clear(&body->seenby);
    tl_clear(&body->path);
    tl_clear(&body->via);
}

/*
 * Convert message body from Textlist to MsgBody struct
 *
 * Return: -1  error during parsing, but still valid control info
 *         -2  fatal error, can't process message
 */

static char *rfc_headers[] = {
    FTN_RFC_HEADERS, NULL
};

#ifdef OLD_TOSS
static int msg_body_parse_echomail(MsgBody * body)
{
    Textline *p, *pp, *ps, *pn;
    char i = FALSE;

    /*
     * Work our way backwards from the end of the body to the tear line
     */
    /* Search for last ^APath or SEEN-BY line */
    for (p = body->body.last;
         p && strncmp(p->line, "\001PATH", 5) && strncmp(p->line, "SEEN-BY", 7);
         p = p->prev)
        // Delete any following lines */
        tl_delete(&body->body, p);

    if (p == NULL) {
        fglog("ERROR: parsing echomail message: no ^APATH or SEEN-BY line");
        return -2;
    }

    /* ^APATH */
    pp = p;
    for (; p && !strncmp(p->line, "\001PATH", 5); p = p->prev)
        i = TRUE;
    pn = p->next;

    if (i) {
        do {
            ps = pn;
            pn = ps->next;
            tl_remove(&body->body, ps);
            tl_add(&body->path, ps);
        }
        while (pp != ps);
        i = FALSE;
        pp = p;
    }

    /* SEEN-BY */
    for (; p && !strncmp(p->line, "SEEN-BY", 7);) {
        i = TRUE;

        if (p->prev)
            p = p->prev;
        else
            break;
    }
    pn = p->next;

    if (i) {
        do {
            ps = pn;
            pn = ps->next;
            tl_remove(&body->body, ps);
            tl_add(&body->seenby, ps);
        }
        while (pp != ps);
    }

    /* Some systems generate empty line[s] between Origin and SEEN-BY :-( */
    for (; p->line && *p->line == '\r'; p = p->prev) {
        pp = p->prev;
        tl_delete(&body->body, p);
        p = pp;
    }

    /*  * Origin: */

    if (p->line && !strncmp(p->line, " * Origin:", 10)) {
        pp = p->prev;
        body->origin = strsave(p->line);
        tl_delete(&body->body, p);
        p = pp;
    }

    /* --- Tear line */

    if (p->line && (!strncmp(p->line, "---\r", 4) ||
                    !strncmp(p->line, "--- ", 4))) {
        body->tear = strsave(p->line);
        tl_delete(&body->body, p);
    }

    if (body->seenby.n == 0) {
        fglog("ERROR: parsing echomail message: no SEEN-BY line");
        return -2;
    }
    if (body->tear == NULL || body->origin == NULL)
        return -1;

    return OK;
}

static int msg_body_parse_netmail(MsgBody * body)
{
    Textline *p, *pn;

    /*
     * Work our way backwards from the end of the body to the tear line
     */
    /* ^AVia, there may be empty lines within the ^AVia lines */
    for (p = body->body.last;
         p && (!strncmp(p->line, "\001Via", 4) ||
               !strncmp(p->line, "\001Recd", 5) || *p->line == '\r');
         p = p->prev) ;
    /*  * Origin: */
    if (p && !strncmp(p->line, " * Origin:", 10))
        p = p->prev;
    /* --- Tear line */
    if (p && (!strncmp(p->line, "---\r", 4) || !strncmp(p->line, "--- ", 4)))
        p = p->prev;
    /* Move back */
    p = p ? p->next : body->body.first;

    /*
     * Copy to MsgBody
     */
    /* --- Tear line */
    if (p && (!strncmp(p->line, "---\r", 4) || !strncmp(p->line, "--- ", 4))) {
        pn = p->next;
        body->tear = strsave(p->line);
        tl_delete(&body->body, p);
        p = pn;
    }
    /*  * Origin: */
    if (p && !strncmp(p->line, " * Origin:", 10)) {
        pn = p->next;
        body->origin = strsave(p->line);
        tl_delete(&body->body, p);
        p = pn;
    }
    /* ^AVia */
    while (p) {
        pn = p->next;

        if (!strncmp(p->line, "\001Via", 4) || !strncmp(p->line, "\001Recd", 5)) {
            tl_remove(&body->body, p);
            tl_add(&body->via, p);
            p = pn;
        } else if (*p->line == '\r') {
            tl_remove(&body->body, p);
            p = pn;
        } else
            break;
    }
    /* Delete any following lines */
    while (p) {
        pn = p->next;
        tl_delete(&body->body, p);
        p = pn;
    }

    return OK;
}
#endif                          /* OLD_TOSS */

int is_blank_line(char *s)
{
    if (!s)
        return TRUE;
    while (*s) {
        if (!is_space(*s))
            return FALSE;
        s++;
    }
    return TRUE;
}

#ifdef OLD_TOSS
int msg_body_parse(Textlist * text, MsgBody * body)
{
    Textline *p, *pn;
    int i;
    int look_for_AREA;

    msg_body_clear(body);

    p = text->first;

    /*
     * 1st, look for ^A kludges and AREA: line
     */
    look_for_AREA = TRUE;

    while (p) {
        pn = p->next;

        if (p->line[0] == '\001') { /* ^A kludge,    */
            tl_remove(text, p);
#if 0
            if (!strncmp(p->line, "\001Via", 4))
                tl_add(&body->via, p);
            else
#endif
                tl_add(&body->kludge, p);
            p = pn;
            continue;
        }

        if (look_for_AREA &&    /* Only 1st AREA line */
            !strncmp(p->line, "AREA:", 5)) {    /* AREA:XXX */
            look_for_AREA = FALSE;
            body->area = strsave(p->line);
            tl_delete(text, p);
            p = pn;
            continue;
        }

        break;
    }

    /*
     * Next, look for supported RFC header lines. Up to 3 blank
     * lines before the RFC headers are allowed
     */
    if (p && is_blank_line(p->line))
        p = p->next;
    if (p && is_blank_line(p->line))
        p = p->next;
    if (p && is_blank_line(p->line))
        p = p->next;

    while (p) {
        int found;
        pn = p->next;

        for (found = FALSE, i = 0; rfc_headers[i]; i++)
            if (!strnicmp(p->line, rfc_headers[i], strlen(rfc_headers[i]))) {
                tl_remove(text, p);
                tl_add(&body->rfc, p);
                p = pn;
                found = TRUE;
                break;
            }

        if (!found)
            break;
    }

    /*
     * Now, text contains just the message body after kludges
     * and RFC headers, copy to body->body.
     */
    body->body = *text;
    tl_init(text);

    /*
     * Call function for EchoMail and NetMail, respectively, parsing
     * control info at end of text body.
     */
    return body->area ? msg_body_parse_echomail(body)
        : msg_body_parse_netmail(body);
}
#endif                          /* OLD_TOSS */

#define HI_KLUDGE		1
#define PARSE_BLANK		2
#define SEARCH_RFC		3
#define PARSE_BODY_NETMAIL	4
#define PARSE_BODY_ECHOMAIL	5
#define FOUND_KLUDGE		0x0001
#define FOUND_ORIGIN		0x0002
#define FOUND_TEARLINE		0x0004

/*
 * Read text body from packet into Textlist & parse it.
 *
 * Return values:
 *     -1  ERROR     an error occured
 *      0  MSG_END   end of packet
 *      2  MSG_TYPE  next message header follows
 */
short int pkt_get_body_parse(FILE * fp, MsgBody * body, Node * from, Node * to)
{
    short int type;
    char look_for_AREA = TRUE;
    char parse_blank = 3;
    char do_flag = HI_KLUDGE;
    int line = 0;
    int ret = ERROR;
    int len;

    msg_body_clear(body);

    /* Read lines and put into textlist */
    while ((type = pkt_get_line(fp, buffer, sizeof(buffer))) == 1) {
        if (do_flag == HI_KLUDGE) {
            if (buffer[0] == '\001') {
                tl_append(&body->kludge, buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "kludge_hi: %s", buffer);
                }
                ret = OK;
                continue;
            }
            if (look_for_AREA &&    /* Only 1st AREA line */
                !strncmp(buffer, "AREA:", 5)) { /* AREA:XXX */
                body->area = strsave(buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "area: %s", buffer);
                }
                look_for_AREA = FALSE;
                ret = OK;
                continue;
            }
            if (ret == OK)
                do_flag = PARSE_BLANK;
            else {
                fglog("ERROR: no found HI kludge in message");
                return ERROR;
            }

        }
        if (do_flag == PARSE_BLANK) {
            parse_blank--;
            if (!is_blank_line(buffer) || !parse_blank)
                do_flag = SEARCH_RFC;
            else {
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "parse_blank: %s", buffer);
                }
                continue;
            }
        }
        if (do_flag == SEARCH_RFC) {
            int i;
            char found;

            for (found = FALSE, i = 0; rfc_headers[i]; i++)
                if (!strnicmp(buffer, rfc_headers[i], strlen(rfc_headers[i]))) {
                    tl_append(&body->rfc, buffer);
                    if (verbose) {
                        len = strlen(buffer);
                        if (buffer[len - 1] == '\n')
                            buffer[len - 1] = '\0';
                        debug(9, "rfc: %s", buffer);
                    }
                    found = TRUE;
                    continue;
                }
            if (found)
                continue;
            else if (body->area)
                do_flag = PARSE_BODY_ECHOMAIL;
            else {
                do_flag = PARSE_BODY_NETMAIL;
                ret = OK;
            }
        }
        if (do_flag == PARSE_BODY_NETMAIL) {
            if (!strncmp(buffer, "---", 3) &&
                (buffer[3] == ' ' || buffer[3] == '\r') && !body->tear) {
                body->tear = strsave(buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "tearline: %s", buffer);
                }
                line |= FOUND_TEARLINE;
                continue;
            }
            if (!strncmp(buffer, " * Origin:", 10) && !body->origin) {
                body->origin = strsave(buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "origin: %s", buffer);
                }
                line |= FOUND_ORIGIN;
                continue;
            }

            if (*buffer == '\001' && (!strncmp(buffer, "\001Via", 4) ||
                                      !strncmp(buffer, "\001Recd", 5))) {
                tl_append(&body->via, buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "via/recd: %s", buffer);
                }
                line |= FOUND_KLUDGE;
                continue;
            }
            /* control duplicate origin or tearline */
            if (line == 0) {
                tl_append(&body->body, buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "body: %s", buffer);
                }
            } else if (line & FOUND_TEARLINE) {
                tl_append(&body->body, body->tear);
                tl_append(&body->body, buffer);
                xfree(body->tear);
                body->tear = NULL;
                line &= ~FOUND_TEARLINE;
            } else if (line & FOUND_ORIGIN) {
                tl_append(&body->body, body->origin);
                tl_append(&body->body, buffer);
                xfree(body->origin);
                body->origin = NULL;
                line &= ~FOUND_ORIGIN;
            }
        } else if (do_flag == PARSE_BODY_ECHOMAIL) {
            if (!strncmp(buffer, "---", 3) &&
                (buffer[3] == ' ' || buffer[3] == '\r') && !body->tear) {
                body->tear = strsave(buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "tearline: %s", buffer);
                }
                line |= FOUND_TEARLINE;
                continue;
            }
            if (!strncmp(buffer, " * Origin:", 10) && !body->origin) {
                body->origin = strsave(buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "origin: %s", buffer);
                }
                line |= FOUND_ORIGIN;
                continue;
            }

            if (!strncmp(buffer, "\001PATH", 5)) {
                tl_append(&body->path, buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "path: %s", buffer);
                }
                line |= FOUND_KLUDGE;
                continue;
            }
            if (!strncmp(buffer, "SEEN-BY", 7)) {
                tl_append(&body->seenby, buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "seen-by: %s", buffer);
                }
                line |= FOUND_KLUDGE;
                continue;
            }
            /* control duplicate origin or tearline */
            if (line == 0) {
                tl_append(&body->body, buffer);
                if (verbose) {
                    len = strlen(buffer);
                    if (buffer[len - 1] == '\n')
                        buffer[len - 1] = '\0';
                    debug(9, "body: %s", buffer);
                }
            } else if (line & FOUND_TEARLINE) {
                tl_append(&body->body, body->tear);
                tl_append(&body->body, buffer);
                xfree(body->tear);
                body->tear = NULL;
                line &= ~FOUND_TEARLINE;
            } else if (line & FOUND_ORIGIN) {
                tl_append(&body->body, body->origin);
                tl_append(&body->body, buffer);
                xfree(body->origin);
                body->origin = NULL;
                line &= ~FOUND_ORIGIN;
            }
        }
    }
    if (type == ERROR) {
        if (feof(fp)) {
            fglog("WARNING: premature EOF reading input packet");
        } else {
            fglog("ERROR: reading input packet");
            TMPS_RETURN(ERROR);
        }
    }
    if (body->area) {
        if (body->tear == NULL) {
            debug(9, "WARNING: no Tearline!");
            body->tear = strsave("---\r");
        }
        if (body->origin == NULL) {
            debug(9, "WARNING: no ' * Origin:' line!");
#ifdef INSERT_ORIGIN
            sprintf(buffer, " * Origin: (%s)\r", znfp1(from));
#endif
            body->origin = strsave(buffer);
        }
        if (body->seenby.n == 0) {
            debug(9, "WARNING: parsing echomail message: no SEEN-BY line!");
            sprintf(buffer, "SEEN-BY: %d/%d", to->net, to->node);
            tl_append(&body->seenby, buffer);
        }
        if (body->path.n == 0) {
            debug(9, "WARNING: parsing echomail message: no ^APATH line!");
            sprintf(buffer, "\001PATH: %d/%d", to->net, to->node);
            tl_append(&body->path, buffer);
        }
    }

    /* Put incomplete last line into textlist, if any */

    return ret;
}

/*
 * Write single line to packet file, checking for NULL
 */
int msg_put_line(FILE * fp, char *line)
{
    if (line)
        fputs(line, fp);
    return ferror(fp);
}

/*
 * Write MsgBody to packet file
 */
int msg_put_msgbody(FILE * fp, MsgBody * body)
{
    msg_put_line(fp, body->area);
    tl_fput(fp, &body->kludge);
    tl_fput(fp, &body->rfc);
    tl_fput(fp, &body->body);
    msg_put_line(fp, body->tear);
    msg_put_line(fp, body->origin);
    tl_fput(fp, &body->seenby);
    tl_fput(fp, &body->path);
    tl_fput(fp, &body->via);

    putc(0, fp);                /* Terminating 0-byte */

    return ferror(fp);
}

/*
 * Convert text line read from FTN message body
 */
char *msg_xlate_line(char *buf, int n, char *line, int ignore_soft_cr)
{
    char *s, *p;
    int c;

    n--;                        /* Room for \0 char */

    for (s = line, p = buf; *s; s++) {
        c = *s & 0xff;

        /*
         * Special chars require special treatment ...
         */
        if (c == '\n' || (ignore_soft_cr && c == 0x8d)) /* Ignore \n and soft \r */
            continue;
        if (c == '\r')
            c = '\n';
        else if (c < ' ') {
            /* Translate control chars to ^X */
            if (c != '\t' && c != '\f') {
                if (!n--)
                    break;
                *p++ = '^';
                c = c + '@';
            }
        }
        /*
         * Put normal char into buf
         */
        if (!n--)
            break;
        *p++ = c;
    }

    *p = 0;

    return OK;
}

/*
 * check_origin() --- Analyse ` * Origin: ...' line in FIDO message
 *		      body and parse address in ()s into Node structure
 *
 * Origin line is checked for the rightmost occurence of
 * ([text] z:n/n.p).
 */
int msg_parse_origin(char *buffer, Node * node)
{
    char *left, *right;
    char *buf;

    if (buffer == NULL)
        return ERROR;

    buf = strsave(buffer);
    right = strrchr(buf, ')');
    if (!right) {
        xfree(buf);
        return ERROR;
    }
    left = strrchr(buf, '(');
    if (!left) {
        xfree(buf);
        return ERROR;
    }

    *right = 0;
    *left++ = 0;

    /* Parse node info */
    while (*left && !is_digit(*left))
        left++;
    if (asc_to_node(left, node, FALSE) != OK)
        /* Not a valid FIDO address */
        node_invalid(node);

    xfree(buf);
    return node->zone != -1 ? OK : ERROR;
}

/*
 * msg_parse_msgid("2:5020/9999 1234abcd");
 */
int msg_parse_msgid(char *str, Node * node)
{
    char *p;
    char *buf;
    int rc = OK;

    if (NULL == str)
        return ERROR;

    do {
        buf = strsave(str);
        p = strchr(buf, ' ');
        if (NULL == p) {
            rc = ERROR;
            break;
        }
        *p = '\0';
        if (OK != asc_to_node(buf, node, FALSE)) {
            node_invalid(node);
            rc = ERROR;
            break;
        }
    }
    while (0);

    xfree(buf);
    return rc;
}
