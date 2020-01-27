/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * NEW charset.c code using charset.bin mapping file
 *
 *****************************************************************************
 * Copyright (C) 1990-2001
 *  _____ _____
 * |     |___  |   Martin Junius             FIDO:      2:2452/110
 * | | | |   | |   Radiumstr. 18             Internet:  mj@fido.de
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

/* New code reuses original structures */

/* fidogate.conf aliases to canonical fsc charset name */
static CharsetAlias *fsc_aliases;
static CharsetAlias *charset_name_map;

static char *orig_in;
static char *orig_out;

/*
 * Translate string
 */
char *xlat_s(char *s1, char *s2)
{
    char *dst = NULL;
    size_t src_len;
    size_t dst_len;
    int rc;

    if (s2 != NULL)
        free(s2);

    if (s1 == NULL)
        return NULL;

    src_len = strlen(s1);
    src_len++;                  /* recode also final \0 */

    rc = charset_recode_buf(&dst, &dst_len, s1, src_len, orig_in, orig_out);
    if (rc == OK)
        return dst;

    free(dst);
    return NULL;
}

/*
 * Set character mapping table
 */
void charset_set_in_out(char *in, char *out)
{
    if (!in || !out)
        return;

    debug(5, "charset: in=%s out=%s", in, out);

    orig_in = in;
    orig_out = out;
}

static void charset_fsc_aliases_add(char **list)
{
    char *name = *list++;
    char *alias;
    CharsetAlias *p;

    for (alias = *list; alias; alias = *++list) {
        p = xmalloc(sizeof(*p));
        snprintf(p->name, sizeof(p->name), "%s", name);
        snprintf(p->alias, sizeof(p->alias), "%s", alias);

        p->next = fsc_aliases;
        fsc_aliases = p;

        debug(15, "Adding FSC alias %s -> %s\n", alias, name);
    }
}

char *charset_fsc_canonize(char *chrs)
{
    CharsetAlias *p;

    for (p = fsc_aliases; p; p = p->next) {
        if (streq(chrs, p->alias))
            return p->name;
    }
    return chrs;
}

static void charset_fsc_aliases_init(void)
{
    int first = TRUE, next = FALSE;
    char *p;
    char **list = NULL;

    for (p = cf_get_string("CharsetAliasesFSC", first);
         p; p = cf_get_string("CharsetAliasesFSC", next)) {

        list_init(&list, p);

        if (*list == NULL) {
            fglog("ERROR: CharsetAliasesFSC requires <name> <list>\n");
            continue;
        }

        charset_fsc_aliases_add(list);
    }

    list_free(list);
}

char *charset_name_rfc2ftn(char *chrs)
{
    CharsetAlias *p;

    for (p = charset_name_map; p; p = p->next) {
        if (streq(chrs, p->name))
            return p->alias;
    }
    return chrs;
}

static void charset_name_map_add(char **list)
{
    char *rfc = list[0];
    char *fsc = list[1];
    CharsetAlias *p;

    p = xmalloc(sizeof(*p));
    snprintf(p->name, sizeof(p->name), "%s", rfc);
    snprintf(p->alias, sizeof(p->alias), "%s", fsc);

    p->next = charset_name_map;
    charset_name_map = p;

    debug(15, "Adding charset name map %s -> %s\n", rfc, fsc);
}

static void charset_name_map_init(void)
{
    int first = TRUE, next = FALSE;
    char *p;
    char **list = NULL;

    for (p = cf_get_string("CharsetNameMap", first);
         p; p = cf_get_string("CharsetNameMap", next)) {

        list_init(&list, p);

        if ((list[0] == NULL) || (list[1] == 0)) {
            fglog("ERROR: Syntax CharsetNameMap <RFC name> <FSC name>\n");
            continue;
        }

        charset_name_map_add(list);
    }

    list_free(list);
}

/*
 * Initialize charset mapping
 */
void charset_init(void)
{
    charset_fsc_aliases_init();
    charset_name_map_init();
}

struct str_to_str {
    char *key;
    char *val;
};

static struct str_to_str level1_map[] = {
    {"ASCII", "ASCII"},
    {"DUTCH", "ISO646-DK"},
    {"FINNISH", "ISO646-FI"},
    {"FRENCH", "ISO646-FR"},
    {"CANADIAN", "ISO646-CA"},
    {"GERMAN", "ISO646-DE"},
    {"ITALIAN", "ISO646-IT"},
    {"NORWEIG", "ISO646-NO"},
    {"PORTU", "ISO646-PT"},
    {"SPANISH", "ISO646-ES"},
    {"SWEDISH", "ISO646-SE"},
    {"SWISS", "ISO646-CN"},
    {"UK", "ISO646-GB"},
};

static char *charset_level1_to_iconv(char *charset)
{
    int i;

    for (i = 0; i < sizeof(level1_map) / sizeof(level1_map[0]); i++)
        if (stricmp(level1_map[i].key, charset) == 0)
            return level1_map[i].val;
    return NULL;
}

/*
 * Get charset name from ^ACHRS kludge line
 */
char *charset_chrs_name(char *s)
{
    static char name[MAXPATH];
    char *p;
    int level;

    while (is_space(*s))
        s++;
    debug(5, "FSC-0054 ^ACHRS/CHARSET: %s", s);

    BUF_COPY(name, s);
    p = strtok(name, " \t");
    if (!p)
        return NULL;

    p = strtok(NULL, " \t");
    if (!p)
        /* In this case it's an FSC-0050 kludge without the class code.
         * Treat it like FSC-0054 level 2. */
        level = 2;
    else
        level = atoi(p);

    switch (level) {
    case 1:
        p = charset_level1_to_iconv(name);
        debug(5, "FSC-0054 level 1 charset=%s (level 2: %s)", name, p);
        return p;

    default:
        debug(5, "FSC-0054 level %d charset=%s", level, name);
        return name;
    }

    return NULL;
}

static void charset_fsc_aliases_free(void)
{
    CharsetAlias *pa, *pa1;

    for (pa = fsc_aliases; pa; pa = pa1) {
        pa1 = pa->next;
        free(pa);
    }
}

static void charset_name_map_free(void)
{
    CharsetAlias *pa, *pa1;

    for (pa = charset_name_map; pa; pa = pa1) {
        pa1 = pa->next;
        free(pa);
    }
}

void charset_free(void)
{
    charset_fsc_aliases_free();
    charset_name_map_free();
}

static int _charset_recode_iconv(char **res, size_t *res_len,
                                 char *src, size_t src_len,
                                 char *from, char *_to)
{
    int rc;
    iconv_t desc;
    size_t size;
    char *to;
    char *dst;
    size_t dst_size;
    size_t inc = src_len;
    char *cur;
    size_t cur_size;
    size_t dst_len;             /* successfuly converted to dst */

    debug(6, "Using ICONV");

    size = strlen(_to) + sizeof("//TRANSLIT");
    to = xmalloc(size);
    sprintf(to, "%s//TRANSLIT", _to);

    desc = iconv_open(to, from);
    if (desc == (iconv_t) - 1) {
        debug(6, "WARNING: iconv cannot convert from %s to %s", from, to);
        return ERROR;
    }

    dst_size = src_len;
    dst = xmalloc(dst_size);
    cur = dst;
    cur_size = dst_size;

    while (src_len > 0) {
        rc = iconv(desc, &src, &src_len, &cur, &cur_size);
        if (rc != -1)
            break;

        if (errno != E2BIG) {
            src++;
            src_len--;
            *cur++ = '?';
            cur_size--;
            continue;
        }

        /* after iconv call cur_size contains size of unused space */
        dst_len = dst_size - cur_size;
        dst = xrealloc(dst, dst_size + inc);
        dst_size += inc;
        cur = dst + dst_len;
        /* unused + new */
        cur_size += inc;
    }

    /*
     * write sequence to get to the initial state if needed
     * https://www.gnu.org/software/libc/manual/html_node/iconv-Examples.html
     */
    iconv(desc, NULL, NULL, &cur, &cur_size);
    dst_len = dst_size - cur_size;
    iconv_close(desc);
    free(to);

    *res = dst;
    *res_len = dst_len;

    return OK;
}

static int charset_recode_iconv(char **dst, size_t *dstlen,
                                char *src, size_t srclen, char *from, char *to)
{
    int rc;
    char *p;
    char *buf;
    size_t len;
    size_t off;

    rc = _charset_recode_iconv(dst, dstlen, src, srclen, from, to);
    if (rc == OK)
        return OK;

    /* Heuristic, LATIN-1 -> LATIN1 */
    p = strchr(from, '-');
    if (p == NULL)
        return ERROR;

    off = p - from;
    len = strlen(from);

    buf = xmalloc(len + 1);
    memcpy(buf, from, off);
    memcpy(buf + off, p + 1, len - off - 1);
    buf[len - 1] = '\0';

    rc = _charset_recode_iconv(dst, dstlen, src, srclen, buf, to);
    free(buf);

    return rc;
}

/*
 * Gets source buffer, lenght of it, allocates buffer for the result.
 * Return dst -- allocated buffer
 *        dstlen -- number of used bytes in it
 * The argument's order is like in str/mem functions
 *
 * Adjusts given length to string's length
 */
int charset_recode_buf(char **dst, size_t *dstlen,
                       char *src, size_t srclen, char *from, char *to)
{
    if (src == NULL || dst == NULL)
        return ERROR;

    if (srclen == 0)
        return ERROR;

    debug(6, "mime charset: recoding from %s to %s", from, to);

    if (strieq(from, to)) {
        *dst = xmalloc(srclen);
        memcpy(*dst, src, srclen);
        *dstlen = srclen;
        return OK;
    }

    return charset_recode_iconv(dst, dstlen, src, srclen, from, to);
}

int charset_is_7bit(char *buffer, size_t len)
{
    int i;

    if (buffer == NULL)
        return TRUE;

    for (i = 0; i < len; i++)
        if (buffer[i] & 0x80)
            return FALSE;
    return TRUE;
}

enum utf8_state {
    START_SEQ,
    PROCESS_SEQ,
    FINISH,
    ERR,
};

static enum utf8_state utf8_check_start(unsigned char c, size_t *n)
{
    size_t num;

    if ((c & 0x80) == 0)
        num = 1;
    else if (((c & 0xc0) == 0xc0) && ((c & 0x20) == 0))
        num = 2;
    else if (((c & 0xe0) == 0xe0) && ((c & 0x10) == 0))
        num = 3;
    else if (((c & 0xf0) == 0xf0) && ((c & 0x08) == 0))
        num = 4;
    else
        return ERR;

    *n = num;
    return PROCESS_SEQ;
}

static bool utf8_check_rest_byte(unsigned char c)
{
    return ((c & 0x80) == 0x80) && ((c & 0x40) == 0);
}

static bool utf8_check_rest_bytes(char *s, size_t len, size_t i, size_t num)
{
    while (num--) {
        if (s[i] == '\0' || i == len)
            return false;
        if (!utf8_check_rest_byte(s[i]))
            return false;
        i++;
    }
    return true;
}

bool charset_is_valid_utf8(char *s, size_t len)
{
    enum utf8_state state = START_SEQ;
    size_t i;
    size_t num;
    static const void *const states[] = {
        [START_SEQ] = &&START_SEQ,
        [PROCESS_SEQ] = &&PROCESS_SEQ,
        [FINISH] = &&FINISH,
        [ERR] = &&ERR,
    };

    i = 0;
    goto START_SEQ;

 START_SEQ:
    if (s[i] == '\0' || i == len)
        goto FINISH;
    state = utf8_check_start(s[i], &num);
    goto *states[state];

 PROCESS_SEQ:
    i++;
    num--;
    if (!utf8_check_rest_bytes(s, len, i, num))
        goto ERR;
    i += num;
    goto START_SEQ;

 FINISH:
    return true;
 ERR:
    return false;
}
