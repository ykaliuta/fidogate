/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FTN NetMail/EchoMail
 *
 *
 * MIME stuff
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

static int is_qpx(int);
static int x2toi(char *);
void mime_free(void);

static MIMEInfo *mime_list = NULL;

static char *mime_get_main_charset()
{
    MIMEInfo *mime;
    char *charset;
    mime = get_mime(s_header_getcomplete("MIME-Version"),
                    s_header_getcomplete("Content-Type"),
                    s_header_getcomplete("Content-Transfer-Encoding"));
    charset = strsave(mime->type_charset);
    mime_free();
    return charset;
}

static int is_qpx(int c)
{
    return isxdigit(c) /**is_digit(c) || (c>='A' && c<='F')**/ ;
}

static int x2toi(char *s)
{
    int val = 0;
    int n;

    n = toupper(*s) - (isalpha(*s) ? 'A' - 10 : '0');
    val = val * 16 + n;
    s++;
    n = toupper(*s) - (isalpha(*s) ? 'A' - 10 : '0');
    val = val * 16 + n;

    return val;
}

/*
 * Dequote string with MIME-style quoted-printable =XX
 */
char *mime_dequote(char *d, size_t n, char *s)
{
    int i, c = 0;

    for (i = 0; i < n - 1 && *s; i++, s++) {
        if (s[0] == '=') {      /* MIME quoted printable */
            if (is_qpx(s[1]) && is_qpx(s[2])) { /* =XX */
                c = x2toi(s + 1);
                s += 2;
            } else if (s[1] == '\n' ||  /* =<LF> and */
                       (s[1] == '\r' && s[2] == '\n')) {    /* =<CR><LF> */
                break;
            }
        } else if (s[0] == '_') {   /* Underscore */
            c = ' ';
        } else {                /* Nothing special to do */
            c = *s;
        }
        d[i] = c;
    }
    d[i] = 0;

    return d;
}

/*
 * Decode MIME RFC1522 header
 *
 **FIXME: currently always assumes ISO-8859-1 char set
 **FIXME: optional flag for conversion to 7bit ASCII replacements
 */
#define MIME_HEADER_CODE_START	"=?"
#define MIME_HEADER_CODE_MIDDLE_QP	"?Q?"
#define MIME_HEADER_CODE_MIDDLE_B64	"?B?"
#define MIME_HEADER_CODE_END	"?="
/*
 * no \r, see
 * https://github.com/ykaliuta/fidogate/commit/a727f4bae629905392e8e174c003bfa38bcbf386
 */
#define MIME_HEADER_STR_DELIM	"\n"
#define MIME_ENC_STRING_LIMIT 80
#define MIME_MAX_ENC_LEN 31

#define MAX_UTF8_LEN 4

/* A..Z -- 0x00..0x19
 * a..z -- 0x1a..0x33
 * 0..9 -- 0x34..0x3d
 * +    -- 0x3e
 * /    -- 0x3f
 * =    -- special
 */
int mime_b64toint(char c)
{
    if (('A' <= c) && ('Z' >= c))
        return (c - 'A');
    else if (('a' <= c) && ('z' >= c))
        return (0x1a + c - 'a');
    else if (('0' <= c) && ('9' >= c))
        return (0x34 + c - '0');
    else if ('+' == c)
        return 0x3e;
    else if ('/' == c)
        return 0x3f;
    else if ('=' == c)
        return 0x40;
    else
        return ERROR;
}

char mime_inttob64(int a)
{
    char c = (char)(a & 0x0000003f);
    if (c <= 0x19)
        return c + 'A';
    if (c <= 0x33)
        return c - 0x1a + 'a';
    if (c <= 0x3d)
        return c - 0x34 + '0';
    if (c == 0x3e)
        return '+';
    else
        return '/';
}

int mime_qptoint(char c)
{
    if (('0' <= c) && ('9' >= c))
        return (c - '0');
    else if (('A' <= c) && ('F' >= c))
        return (0x0a + c - 'A');
    else if (('a' <= c) && ('f' >= c))
        return (0x0a + c - 'a');
    else
        return ERROR;
}

/* rfc 2045 */
static bool mime_qp_is_plain(int c)
{
    return ((c >= '%') && (c < '@') && (c != '='))
        || ((c >= 'A') && (c <= 'Z'))
        || ((c >= 'a') && (c <= 'z'));
}

#define B64_ENC_CHUNK 3
#define B64_NLET_PER_CHUNK 4

/*
 * dst must have space for at least 4 octets
 * dst will not be NUL-terminated
 * Max 3 source octets encoded
 */
static size_t mime_b64_encode_chunk(char *dst, unsigned char *src, unsigned len)
{
    int padding;

    if (len == 0)
        return 0;

    if (len > B64_ENC_CHUNK)
        len = B64_ENC_CHUNK;

    padding = B64_ENC_CHUNK - len;

    dst[0] = mime_inttob64(src[0] >> 2);
    dst[1] = mime_inttob64(src[0] << 4);

    if (len < 2)
        goto out;

    dst[1] = mime_inttob64((src[0] << 4) | (src[1] >> 4));
    dst[2] = mime_inttob64(src[1] << 2);

    if (len < 3)
        goto out;

    dst[2] = mime_inttob64((src[1] << 2) | (src[2] >> 6));
    dst[3] = mime_inttob64(src[2]);

 out:
    for (; padding > 0; padding--)
        dst[B64_NLET_PER_CHUNK - padding] = '=';

    return len;
}

struct mime_word_enc_state {
    char *encoded_line;
    char *charset;
    size_t charset_len;
    size_t size;
    size_t inc;
    size_t pos;                 /* position in the string */
    size_t vpos;                /* visual position in the line */
    size_t limit;               /* in 7 bit chars, no line endings */
    size_t rem_len;
    char *encoding;
    size_t max_mb_chunks;       /* max decoder's chunks to flush mb seq */
    size_t (*calc_len)(char *token, size_t len);
    size_t (*encode)(struct mime_word_enc_state * state, char *p, size_t len);
    char rem[B64_ENC_CHUNK];
    bool is_mime;               /* previous word is not plain */
};

static size_t mime_b64_calc_len(char *t, size_t len)
{
    size_t res;

    /* only b64 now */

    res = len / 3 * 4;
    res += len % 3 ? 4 : 0;

    return res;
}

static void mime_strcat(struct mime_word_enc_state *state, char *str)
{
    size_t len;

    strcpy(state->encoded_line + state->pos, str);
    len = strlen(str);
    state->pos += len;
    state->vpos += len;
}

static void mime_strlcpy(struct mime_word_enc_state *state,
                         char *str, size_t len)
{
    strncpy(state->encoded_line + state->pos, str, len);

    state->pos += len;
    state->vpos += len;

    *(state->encoded_line + state->pos) = '\0';
}

/*
 * Calculates length of the word if it have been encoded.
 * Takes into account the previous state:
 * - if previous state mime-encoded and adding 7 bit, that must be
 *   closed;
 * - if previous state mime-encoded and adding mime, that is just
 *   connected to the code, space encoded, no mime-start;
 * - if previous state is 7 bit and adding 7 bit -- just connect as
 *   well;
 * - if previous state is 7 bit and adding mime, it should start with
 *   new mime-start;
 * If current last word is mime, account 2 chars for mime-end before
 * the new line.
 */
static size_t mime_word_calc_len(struct mime_word_enc_state *state,
                                 char *token, size_t len, char **start)
{                               /* first non-space */
    size_t encoded_len;
    char *p;
    bool prev_mime = state->is_mime;
    size_t charset_len = state->charset_len;
    bool is_7bit;

    for (p = token; p - token < len; p++) {
        if (!isspace(*p))
            break;
    }

    is_7bit = charset_is_7bit(token, len);

    if (prev_mime) {
        if (is_7bit) {
            encoded_len = strlen(MIME_HEADER_CODE_END) + len;
        } else {
            encoded_len = state->calc_len(token, len);  /* with spaces */
        }
    } else {
        if (is_7bit) {
            encoded_len = len;
        } else {
            encoded_len = 1 +   /* space */
                strlen(MIME_HEADER_CODE_START) + charset_len + strlen(MIME_HEADER_CODE_MIDDLE_B64) + state->calc_len(p, len);   /* without spaces */
        }
    }

    encoded_len += state->rem_len ? B64_NLET_PER_CHUNK : 0;

    *start = p;
    return encoded_len;
}

static void mime_flush_reminder(struct mime_word_enc_state *state)
{
    if (state->rem_len == 0)
        return;

    mime_b64_encode_chunk(state->encoded_line + state->pos,
                          (unsigned char *)state->rem, state->rem_len);

    state->pos += B64_NLET_PER_CHUNK;
    state->vpos += B64_NLET_PER_CHUNK;
    state->rem_len = 0;
}

static size_t mime_add_reminder(struct mime_word_enc_state *state,
                                char **p, size_t *len, size_t limit)
{
    size_t ret;
    size_t rem_left;

    if (state->rem_len == 0)
        return 0;

    if (state->rem_len + *len < B64_ENC_CHUNK) {
        memcpy(&state->rem[state->rem_len], *p, *len);

        state->rem_len += *len;
        (*p) += *len;

        ret = *len;
        *len = 0;
        return ret;
    }

    if (state->vpos + B64_NLET_PER_CHUNK > limit)
        return 0;

    rem_left = sizeof(state->rem) - state->rem_len;

    memcpy(&state->rem[state->rem_len], *p, rem_left);
    (*p) += rem_left;
    *len -= rem_left;
    state->rem_len += rem_left;

    mime_flush_reminder(state);

    return rem_left;
}

static size_t mime_save_reminder(struct mime_word_enc_state *state,
                                 char *p, size_t len)
{
    if (len > sizeof(state->rem)) {
        fprintf(stderr, "ERROR: too big reminder: %zu\n", len);
        abort();
    }

    memcpy(state->rem, p, len);
    state->rem_len = len;

    return len;
}

static void mime_word_end(struct mime_word_enc_state *state)
{
    if (!state->is_mime)
        return;

    mime_strcat(state, MIME_HEADER_CODE_END);

    state->is_mime = false;
}

static void mime_line_break(struct mime_word_enc_state *state)
{
    state->size += state->inc;
    state->encoded_line = xrealloc(state->encoded_line, state->size);

    mime_word_end(state);

    mime_strcat(state, MIME_HEADER_STR_DELIM);

    state->vpos = 0;
    state->is_mime = false;
}

static void mime_word_start(struct mime_word_enc_state *state)
{
    size_t len;

    len = 1 + strlen(MIME_HEADER_CODE_START)
        + state->charset_len + strlen(MIME_HEADER_CODE_MIDDLE_B64)
        + B64_NLET_PER_CHUNK    /* at least one */
        + strlen(MIME_HEADER_CODE_END);

    if (state->vpos + len > state->limit)
        mime_line_break(state);

    mime_strcat(state, " ");
    mime_strcat(state, MIME_HEADER_CODE_START);
    mime_strcat(state, state->charset);
    mime_strcat(state, state->encoding);

    state->is_mime = true;
}

static void mime_switch_to_plain(struct mime_word_enc_state *state)
{
    size_t end_len = strlen(MIME_HEADER_CODE_END);
    size_t extra_len = state->rem_len ? B64_NLET_PER_CHUNK : 0;

    if (!state->is_mime)
        return;

    if (state->vpos + extra_len + end_len > state->limit) {
        mime_line_break(state);
        mime_word_start(state);
    }

    mime_flush_reminder(state);
    mime_word_end(state);
}

/* Get size of unfinished multibyte sequence. Only utf-8 supported */
static size_t mime_get_mb_tail(struct mime_word_enc_state *state, char *p)
{
    unsigned char c;
    char *save_p;

    if (state->max_mb_chunks == 0)
	return 0;

    if (!strieq(state->charset, "utf-8"))
	return 0;

    save_p = p;
    for (c = *p;
	 ((c & 0x80) == 0x80) && ((c & 0x40) == 0);
	 c = *++p)
	;
    return p - save_p;
}

/*
 * Encodes part of word, that fits the current line
 */
static size_t mime_word_enc_b64(struct mime_word_enc_state *state,
                                char *p, size_t len)
{
    size_t limit;
    size_t left;
    size_t done;
    size_t chunk;
    size_t to_encode;
    bool end_of_line = false;

    limit = state->limit - strlen(MIME_HEADER_CODE_END);

    done = mime_add_reminder(state, &p, &len, limit);

    left = len;

    while ((left >= B64_ENC_CHUNK) && !end_of_line) {

	/* space to flush multibyte and encode current */
	if (state->vpos
	    + (state->max_mb_chunks + 1) * B64_NLET_PER_CHUNK > limit) {

	    end_of_line = true;

            /* flush multibyte sequence */
	    to_encode = mime_get_mb_tail(state, p);
	} else {
	    to_encode = left;
	}

        chunk = mime_b64_encode_chunk(state->encoded_line + state->pos,
                                      (unsigned char *)p, to_encode);

        left -= chunk;
        done += chunk;
        p += chunk;

	if (chunk > 0) {
	    state->pos += B64_NLET_PER_CHUNK;
	    state->vpos += B64_NLET_PER_CHUNK;
	}

    }

    *(state->encoded_line + state->pos) = '\0';

    if (left < B64_ENC_CHUNK)
        done += mime_save_reminder(state, p, left);

    return done;
}

static size_t mime_max_mb_chunks_b64(char *charset)
{
    if (!strieq(charset, "utf-8"))
	return 0;

    return ((MAX_UTF8_LEN - 1) + (B64_ENC_CHUNK - 1)) / B64_ENC_CHUNK;
}

static int mime_7bit_try(struct mime_word_enc_state *state,
                         char *token, size_t len)
{
    size_t encoded_len;
    char *p = NULL;
    int i;

    /* flushes reminder */
    mime_switch_to_plain(state);

    if (len > state->limit)
        return ERROR;

    encoded_len = mime_word_calc_len(state, token, len, &p);

    if (state->vpos + encoded_len > state->limit)
        mime_line_break(state);

    for (i = 0; i < p - token; i++) {
        if (token[i] != '\n')
            continue;

        /* replace original spaces if there is \n */
        mime_strcat(state, " ");
        len -= p - token;
        token = p;
    }

    mime_strlcpy(state, token, len);
    return OK;
}

#define QP_NLET_MAX 3           /* =XX */

static size_t mime_qp_calc_len(char *s, size_t len)
{
    size_t sum = 0;

    for (; len > 0; --len, s++) {
        if (mime_qp_is_plain(*s))
            sum += 1;
        else
            sum += QP_NLET_MAX;
    }

    return sum;
}

/*
 * Encodes one octet, @out should point to a buffer enough to store 3
 * bytes;
 *
 * Returns amount of bytes written
 */
static size_t _mime_qp_encode_octet(char *out, unsigned char in)
{
    int len;
    char buf[QP_NLET_MAX + 1];  /* max encoded length =XX\0 */

    if (mime_qp_is_plain(in)) {
        *out = in;
        return sizeof(*out);
    }

    len = snprintf(buf, sizeof(buf), "=%02X", in);
    memcpy(out, buf, len);
    return len;
}

static void mime_qp_encode_octet(struct mime_word_enc_state *state,
				 unsigned char in)
{
    size_t produced;

    produced = _mime_qp_encode_octet(state->encoded_line + state->pos, in);
    state->pos += produced;
    state->vpos += produced;
}

static size_t mime_word_enc_qp_flush_mb(struct mime_word_enc_state *state,
					char *p)
{
    size_t to_encode = mime_get_mb_tail(state, p);
    int i;

    for (i = 0; i < to_encode; i++)
        mime_qp_encode_octet(state, p[i]);

    return to_encode;
}

/*
 * Encodes the word, extending the line if necessary
 */
static size_t mime_word_enc_qp(struct mime_word_enc_state *state,
                               char *p, size_t len)
{
    size_t limit;
    size_t left;

    limit = state->limit - strlen(MIME_HEADER_CODE_END);

    for (left = len; left > 0; left--, p++) {
	/* space to flush multibyte and encode current */
        if (state->vpos + (state->max_mb_chunks + 1) * QP_NLET_MAX > limit) {
	    /* flush multibyte sequence */
	    left -= mime_word_enc_qp_flush_mb(state, p);
            break;
	}

        mime_qp_encode_octet(state, *p);
    }

    *(state->encoded_line + state->pos) = '\0';

    return len - left;
}

static size_t mime_max_mb_chunks_qp(char *charset)
{
    if (!strieq(charset, "utf-8"))
	return 0;

    /* QP encoding chunk is one byte, encoded is QP_NLET_MAX */
    return MAX_UTF8_LEN - 1;
}

static size_t mime_8bit_calc_len(char *s, size_t len)
{
    return len;
}

/*
 * Encodes word if necessary (non-7bit) and adds to the header line.
 * Makes extented line if needed.
 * Takes token with the leading spaces when exist
 */
static void mime_word_enc(struct mime_word_enc_state *state,
                          char *token, size_t len)
{
    char *p;
    size_t left;
    size_t consumed;

    /* simple case, no encoding */
    if (charset_is_7bit(token, len)) {

        /* switches to plain mode with flushing reminder */
        if (mime_7bit_try(state, token, len) == OK)
            return;
    }

    /* encode if non-7bit or very long */
    mime_word_calc_len(state, token, len, &p);

    left = len;
    if (!state->is_mime)
        left -= (p - token);    /* skip spaces */
    else
        p = token;              /* encode with the spaces */

    for (;;) {
        if (!state->is_mime) {
            mime_word_start(state);
        }

        consumed = state->encode(state, p, left);

        p += consumed;
        left -= consumed;
        if (left == 0)
            break;

        mime_line_break(state);
    }
}

static void mime_header_enc_start(struct mime_word_enc_state *state,
                                  char *charset, int type)
{
    size_t size = MIME_STRING_LIMIT + sizeof(MIME_HEADER_STR_DELIM);    /* \0 counted */
    char *p;

    p = xmalloc(size);
    *p = '\0';
    state->size = size;
    state->encoded_line = p;
    state->charset = charset;
    state->charset_len = strlen(charset);
    state->inc = size - 1;      /* only one \0 */
    state->pos = 0;
    state->vpos = 0;
    state->limit = MIME_STRING_LIMIT;
    state->rem_len = 0;
    state->is_mime = false;

    switch (type) {
    case MIME_QP:
        state->encoding = MIME_HEADER_CODE_MIDDLE_QP;
        state->calc_len = mime_qp_calc_len;
        state->encode = mime_word_enc_qp;
	state->max_mb_chunks = mime_max_mb_chunks_qp(charset);
        break;
    case MIME_B64:
        state->encoding = MIME_HEADER_CODE_MIDDLE_B64;
        state->calc_len = mime_b64_calc_len;
        state->encode = mime_word_enc_b64;
	state->max_mb_chunks = mime_max_mb_chunks_b64(charset);
        break;
    default:
        state->encoding = NULL;
        state->calc_len = mime_8bit_calc_len;
        state->encode = NULL;
    }
}

static char *mime_header_enc_end(struct mime_word_enc_state *state)
{
    size_t len;

    if (state->is_mime) {
        len = strlen(MIME_HEADER_CODE_END);
        if (state->rem_len)
            len += B64_NLET_PER_CHUNK;

        if (state->vpos + len > state->limit) {
            mime_line_break(state);
            mime_word_start(state);
        }

        mime_flush_reminder(state);
        mime_word_end(state);
    }

    if (state->encoded_line[state->pos - 1] != '\n')
        strcat(state->encoded_line, MIME_HEADER_STR_DELIM);

    return state->encoded_line;
}

/*
 * Mime encodes 8bit header, splitting lines when necessary.
 * src must be NUL-terminated
 *
 * Result is \n terminated
 *
 * Encoding is done word by word, 7bit words are not encoded
 */
int mime_header_enc(char **dst, char *src, char *charset, int enc)
{
    struct mime_word_enc_state state;
    char *token;
    char *token_end;

    debug(6, "MIME: %s: to encode (%s): %s", __func__, charset, src);

    mime_header_enc_start(&state, charset, enc);

    /* strtok does not work on RO strings */

    token = src;
    token_end = strchr(src, ' ');
    if (token_end == NULL) {
        fglog("ERROR: misformatted header: %s\n", src);
        return ERROR;
    }

    while (token != NULL) {
        mime_word_enc(&state, token, token_end - token);

        token = token_end;

        /* first skip spaces */
        while ((*token_end == ' ' || *token_end == '\t' || *token == '\n')
               && *token_end != '\0')
            token_end++;

        if (*token_end == '\0') {
            token = NULL;
            continue;
        }

        /* then find next space or EOL */
        while (*token_end != ' ' && *token_end != '\t' && *token != '\n'
               && *token_end != '\0')
            token_end++;
    }

    mime_header_enc_end(&state);
    *dst = state.encoded_line;

    debug(6, "MIME: %s: encoded: %s", __func__, *dst);

    return OK;
}

void mime_b64_encode_tl(Textlist * in, Textlist * out)
{
    TextlistIterator iter;
    /* + \n\0, no \r needed */
    char buf[MIME_STRING_LIMIT + 2];
    char ibuf[B64_ENC_CHUNK];
    size_t len;
    size_t pos;

    tl_init(out);
    tl_iterator_start(&iter, in);
    pos = 0;

    len = tl_iterator_next(&iter, ibuf, sizeof(ibuf));

    while (len > 0) {
        if (pos + B64_NLET_PER_CHUNK > MIME_STRING_LIMIT) {
            buf[pos++] = '\n';
            buf[pos++] = '\0';

            tl_append(out, buf);
            pos = 0;
        }

        mime_b64_encode_chunk(buf + pos, (unsigned char *)ibuf, len);
        pos += B64_NLET_PER_CHUNK;
        len = tl_iterator_next(&iter, ibuf, sizeof(ibuf));
    }

    buf[pos++] = '\n';
    buf[pos++] = '\0';

    tl_append(out, buf);
}

int mime_b64_decode(char **dst, char *src, size_t len)
{
    char *buf;
    size_t buflen;
    size_t i;
    int v1, v2, v3, v4;
    char *d;
    int rc = OK;

    if (0 != (len % 4))
        return ERROR;

    buflen = ((len / 4) * 3) + 1;
    buf = xmalloc(buflen);
    memset(buf, 0, buflen);
    i = 0;
    d = buf;
    while (len > i) {
        v1 = mime_b64toint(src[i++]);
        if (ERROR == v1) {
            rc = ERROR;
            break;
        }

        v2 = mime_b64toint(src[i++]);
        if (ERROR == v2) {
            rc = ERROR;
            break;
        }

        v3 = mime_b64toint(src[i++]);
        if (ERROR == v3) {
            rc = ERROR;
            break;
        }

        v4 = mime_b64toint(src[i++]);
        if (ERROR == v4) {
            rc = ERROR;
            break;
        }

        *d++ = (v1 << 2) | (v2 >> 4);
        if (0x40 > v3) {
            *d++ = ((v2 << 4) & 0xf0) | (v3 >> 2);
            if (0x40 > v4)
                *d++ = ((v3 << 6) & 0xc0) | v4;
        } else if (0x40 > v4) {
            rc = ERROR;
            break;
        }
    }

    if (ERROR == rc)
        xfree(buf);
    else
        *dst = buf;
    return rc;
}

void mime_qp_encode_tl(Textlist * in, Textlist * out)
{
    TextlistIterator iter;
    /* + =\n\0 */
    char buf[MIME_STRING_LIMIT + 4];
    char ibuf[1];
    size_t len;
    size_t pos;
    size_t len_encoded;

    tl_init(out);
    tl_iterator_start(&iter, in);
    pos = 0;

    len = tl_iterator_next(&iter, ibuf, sizeof(ibuf));

    while (len > 0) {
        if (pos + QP_NLET_MAX > MIME_STRING_LIMIT) {
            buf[pos++] = '=';
            buf[pos++] = '\n';
            buf[pos++] = '\0';

            tl_append(out, buf);
            pos = 0;
        }

        len_encoded = _mime_qp_encode_octet(buf + pos, ibuf[0]);
        pos += len_encoded;
        len = tl_iterator_next(&iter, ibuf, sizeof(ibuf));
    }

    buf[pos++] = '\n';
    buf[pos++] = '\0';

    tl_append(out, buf);
}

int mime_qp_decode(char **dst, char *src, size_t len)
{
    char *buf;
    size_t i;
    int vh, vl;
    char *d;
    char *p1, *p2;
    int j;
    int rc = OK;

    p1 = src;
    j = 0;
    while (NULL != (p2 = strchr(p1, '_'))) {
        p1 = ++p2;
        j++;
    }

    buf = xmalloc(len + 1);
    memset(buf, 0, len + 1);

    i = 0;
    d = buf;
    while (len > i) {
        if (src[i] == '\n' || src[i] == '\r') {
            *d = '\n';
            break;
        }

        if ('_' == src[i]) {
            *d++ = ' ';
            i++;
            continue;
        }

        if ('=' != src[i]) {
            *d++ = src[i++] & 0x7F;
            continue;
        }

        i++;

        if (src[i] == '\n' || src[i] == '\r') {
            *d = '\n';
            break;
        }

        vh = mime_qptoint(src[i++]);
        if (ERROR == vh) {
            rc = ERROR;
            break;
        }

        vl = mime_qptoint(src[i++]);
        if (ERROR == vl) {
            rc = ERROR;
            break;
        }

        *d++ = (char)(((vh << 4) & 0xf0) | (vl & 0x0f));
    }

    if (ERROR == rc)
        xfree(buf);
    else
        *dst = buf;
    return rc;
}

/*
 * mime word is a mime encoded string, separated by space
 * from other mime words.
 * The function takes non-whitespace starting string
 * and decodes it from mime (quoted-printable or base64),
 * if it is properly encoded.
 * Leaves the string as is, if cannot recode.
 * Allocates buffer,
 * return the buffer,
 *        the byte length of the decoded string (no final '\0'),
 *        and the charset (from the mime info)
 */
static int mime_handle_mimed_word(char *s, char **out, size_t *out_len,
                                  bool *is_mime,
                                  char *charset, size_t ch_size, char *to)
{
    char *p;
    size_t len;
    size_t tmp_len;
    char *buf;
    char *beg;
    char *end;
    int (*decoder)(char **dst, char *src, size_t len) = NULL;

    /* May be MIME (b64 or qp) */
    p = strchr(s + strlen(MIME_HEADER_CODE_START), '?');
    if (p == NULL)
        goto fallback;

    tmp_len = p - (s + strlen(MIME_HEADER_CODE_START));
    if (tmp_len > MIME_MAX_ENC_LEN) {
        fglog("ERROR: charset name's length too long, %zd. Do not recode",
              tmp_len);
        goto fallback;
    }

    tmp_len = MIN(tmp_len + 1, ch_size);
    snprintf(charset, tmp_len, "%s", s + strlen(MIME_HEADER_CODE_START));

    /* Check if b64 or qp */
    if (strnieq(p,
                MIME_HEADER_CODE_MIDDLE_QP,
                strlen(MIME_HEADER_CODE_MIDDLE_QP))) {
        /* May be qp */
        decoder = mime_qp_decode;
        beg = p + strlen(MIME_HEADER_CODE_MIDDLE_QP);

    } else if (strnieq(p,
                       MIME_HEADER_CODE_MIDDLE_B64,
                       strlen(MIME_HEADER_CODE_MIDDLE_B64))) {
        /* May be b64 */
        decoder = mime_b64_decode;
        beg = p + strlen(MIME_HEADER_CODE_MIDDLE_B64);
    } else {
        fglog
            ("ERROR: subject looks like mime, but does not have proper header");
        goto fallback;
    }

    end = strchr(beg, '?');

    if (end == NULL ||
        !strnieq(end, MIME_HEADER_CODE_END, strlen(MIME_HEADER_CODE_END))) {
        fglog
            ("ERROR: subject looks like mime, but does not have proper ending");
        goto fallback;
    }
    if (decoder(&buf, beg, end - beg) == ERROR) {
        fglog("ERROR: subject mime deconding failed");
        goto fallback;
    }

    *out_len = strlen(buf);
    *out = buf;
    *is_mime = true;
    return end + strlen(MIME_HEADER_CODE_END) - s;

 fallback:
    /*
     * Keep it as is.
     * Will work correctly with utf-8 plain headers, iconv and utf-8
     * internal charset.
     */
    debug(6, "Could not unmime subject '%s', leaving as is", s);
    len = strlen(s);
    *out = xmalloc(len);
    memcpy(*out, s, len);       /* no '\0' */
    snprintf(charset, ch_size, "%s", to);
    *is_mime = false;
    *out_len = len;

    return len;
}

static int mime_handle_plain_word(char *s, char **out, size_t *out_len,
                                  char *charset, size_t ch_size, char *to)
{
    char *plain_charset = CHARSET_STDRFC;
    char *mime_charset = NULL;
    char *p;
    size_t len;
    char *res;

    /*
     * If there are several words, handle only one.
     * Reading code should combine splitted lines, so no \n.
     * TODO: \t can be checked as well
     */
    p = strchr(s, ' ');
    if (p != NULL)
        len = p - s;
    else
        len = strlen(s);

    /*
     * There must be no plain 8 bit headers except utf-8.
     * If there are, do some heuristics:
     * - if body charset supplied and is not 7 bit us-ascii, use that;
     * - othewise it's broken most probably, assume the precompiled
     *   default.
     */
    if (!charset_is_7bit(s, len) && charset_is_valid_utf8(s, len)) {
        plain_charset = "utf-8";
    } else {
        mime_charset = mime_get_main_charset();

        if ((mime_charset != NULL) &&
            (strcmp(mime_charset, CHARSET_STD7BIT) != 0)) {
            plain_charset = mime_charset;
        }
    }

    strncpy(charset, plain_charset, ch_size - 1);
    charset[ch_size - 1] = '\0';

    free(mime_charset);

    res = xmalloc(len + 1);
    str_copy(res, len + 1, s);

    *out_len = len;
    *out = res;

    return *out_len;
}

/*
 * Fetches the mime word from the string. It can be mime-encoded
 * or plain part, separated with "space".
 * _Must_ start with non-space.
 * For mime-encoded parts decode it and fetch the charset.
 * For plain parts copy them as is, take the global charset.
 * Allocates the final buffer.
 * @returns the number of bytes, handled in the source string.
 */
static int mime_handle_word(char *s, char **out, size_t *out_len,
                            bool *is_mime, char *charset, size_t ch_size,
                            char *to)
{
    if (strnieq(s, MIME_HEADER_CODE_START, strlen(MIME_HEADER_CODE_START))) {
        return mime_handle_mimed_word(s,
                                      out, out_len,
                                      is_mime, charset, ch_size, to);
    }

    *is_mime = false;
    return mime_handle_plain_word(s, out, out_len, charset, ch_size, to);
}

/* source @s must be '\0'-terminated */
char *mime_header_dec(char *d, size_t d_max, char *s, char *to)
{
    char *save_d = d;
    bool is_mime = false;
    bool is_prev_mime = false;
    char *buf;
    size_t len;
    char charset[MIME_MAX_ENC_LEN + 1];
    int rc;
    size_t d_left = d_max - 1;
    size_t s_handled;
    char *recoded;
    size_t recoded_len;

    while (d_left != 0 && *s != '\0') {
        if (isspace(*s)) {
            /* just skip spaces between mime words */
            if (!is_mime) {
                *d++ = *s;
                d_left--;
            }
            s++;
            continue;
        }

        is_prev_mime = is_mime;

        /* it means mime "word" -- encoded or plain part */
        s_handled = mime_handle_word(s, &buf, &len, &is_mime,
                                     charset, sizeof(charset), to);

        /* keep at least one space between mime and non-mime words */
        if (is_prev_mime && !is_mime) {
            *d++ = ' ';
            d_left--;

            if (d_left == 0)
                break;
        }

        debug(6, "header charset: %s", charset);
        rc = charset_recode_buf(&recoded, &recoded_len, buf, len, charset, to);
        if (rc != OK) {
            debug(6, "Could not recode header %s", buf);
            goto out;
        }

        free(buf);

        if (recoded_len > d_left) {
            fglog("ERROR: header buffer too small");
            recoded_len = d_left;
        }

        memcpy(d, recoded, recoded_len);
        d += recoded_len;
        d_left -= recoded_len;

        s += s_handled;
    }

 out:
    *d = '\0';
    return save_d;
}

/* takes space-stripped string */
/* Now is not used */
/*

static char* mime_fetch_attribute(char *str, char *attr)
{
     int attr_len = strlen(attr);
     char *tmp_str = NULL;

     do
     {
	  if(strnieq(str, attr, attr_len))
	  {
	       tmp_str = xmalloc(attr_len + 1);
	       strncpy(tmp_str, str, attr_len);
	       tmp_str[attr_len] = '\0';
	       break;
	  }
	  str = strchr(str, ';');
	  if(str != NULL)
	       str++;
     } while (str != NULL);

     return tmp_str;
}
*/

static int mime_parse_header(Textlist * line, char *str)
{
    char *p = NULL;

    if (line == NULL || str == NULL)
        return ERROR;

    debug(6, "Parsing header %s", str);
    for (p = strtok(str, ";"); p != NULL; p = strtok(NULL, ";")) {
        debug(6, "Recording header attribute %s", p);
        p = strip_space(p);
        tl_append(line, p);
    }
    return OK;
}

static char *mime_attr_value(char *str)
{
    char *p, *q = NULL;

    if (str == NULL)
        return NULL;

    p = strchr(str, '=');
    if (p != NULL) {
        if (*(++p) == '\"')
            p++;
        for (q = p; *q != '\0'; q++)
            if (*q == '\"' || is_space(*q))
                break;
        *q = 0;
        p = strsave(p);
    }
    return p;
}

/*
 * Return MIME header
 */

static MIMEInfo *get_mime_disposition(char *ver, char *type, char *enc,
                                      char *disp)
{
    MIMEInfo *mime;

    Textlist header_line = { NULL, NULL };
    Textline *tmp_line;
    char *tmp_str = NULL;

    mime = (MIMEInfo *) s_alloc(sizeof(*mime));

    mime->version = ver;
    mime->type = type;
    mime->type_type = NULL;
    mime->type_charset = NULL;
    mime->type_boundary = NULL;
    mime->encoding = enc;
    mime->disposition = disp;
    mime->disposition_filename = NULL;

    if (type != NULL) {
        tmp_str = s_copy(type);
        mime_parse_header(&header_line, tmp_str);
        if (header_line.first != NULL && header_line.first->line != NULL)
            mime->type_type = s_copy(header_line.first->line);

        tmp_line = tl_get(&header_line, "charset", strlen("charset"));
        if (tmp_line != NULL) {
            tmp_str = mime_attr_value(tmp_line->line);
            mime->type_charset = s_copy(tmp_str);
            xfree(tmp_str);
        }
        tmp_line = tl_get(&header_line, "boundary", strlen("boundary"));
        if (tmp_line != NULL) {
            tmp_str = mime_attr_value(tmp_line->line);
            mime->type_boundary = s_copy(tmp_str);
            xfree(tmp_str);
        }
        tl_clear(&header_line);
    }

    if (disp != NULL) {
        tmp_str = s_copy(disp);
        mime_parse_header(&header_line, tmp_str);
        tmp_line = tl_get(&header_line, "filename", strlen("filename"));
        if (tmp_line != NULL)
            tmp_str = mime_attr_value(tmp_line->line);
        mime->disposition_filename = s_copy(tmp_str);
        tl_clear(&header_line);
    }

    debug(6, "RFC MIME-Version:              %s",
          mime->version ? mime->version : "-NONE-");
    debug(6, "RFC Content-Type:              %s",
          mime->type ? mime->type : "-NONE-");
    debug(6, "                        type = %s",
          mime->type_type ? mime->type_type : "");
    debug(6, "                     charset = %s",
          mime->type_charset ? mime->type_charset : "");
    debug(6, "                    boundary = %s",
          mime->type_boundary ? mime->type_boundary : "");
    debug(6, "RFC Content-Transfer-Encoding: %s",
          mime->encoding ? mime->encoding : "-NONE-");
    debug(6, "RFC Content-Disposition: %s",
          mime->disposition ? mime->disposition : "-NONE-");
    debug(6, "                    filename = %s",
          mime->disposition_filename ? mime->disposition_filename : "");

    return mime;
}

MIMEInfo *get_mime(char *ver, char *type, char *enc)
{
    return get_mime_disposition(ver, type, enc, NULL);
}

/* TODO smth like s_header_getcomplete() */

MIMEInfo *get_mime_from_header(Textlist * header)
{
    if (header == NULL)
        return get_mime_disposition(header_get("Mime-Version"),
                                    header_get("Content-Type"),
                                    header_get("Content-Transfer-Encoding"),
                                    header_get("Content-Disposition"));
    else
        return get_mime_disposition(rfcheader_get(header, "Mime-Version"),
                                    rfcheader_get(header, "Content-Type"),
                                    rfcheader_get(header,
                                                  "Content-Transfer-Encoding"),
                                    rfcheader_get(header,
                                                  "Content-Disposition"));
}

/* Garantee [^\r]\n\0 in the end of string since our
   output subroutine adds '\r'
   str should have space for 1 extra symbol */

static char *mime_debody_flush_str(Textlist * body, char *str)
{
    int len;
    if (str == NULL || body == NULL)
        return NULL;

    len = strlen(str);
    if (len < 2)
        return NULL;

    if (str[len - 2] == '\r') {
        str[len - 2] = '\n';
        str[len - 1] = '\0';
    } else if (str[len - 1] != '\n') {
        str[len] = '\n';
        str[len + 1] = '\0';
    }

    tl_append(body, str);
    str[0] = '\0';

    return str;
}

static Textlist *mime_debody_qp(Textlist * body)
{
    char *dec_str, *tmp_str;
    long len;

    Textlist *dec_body;
    Textlist *full_line;

    Textline *line, *subline;

    dec_body = xmalloc(sizeof(*dec_body));
    memset(dec_body, 0, sizeof(*dec_body));

    full_line = xmalloc(sizeof(*full_line));
    memset(full_line, 0, sizeof(*full_line));

    for (line = body->first; line != NULL; line = line->next) {
        len = strlen(line->line);
        if (mime_qp_decode(&dec_str, line->line, len) == ERROR)
            goto fail;

        strip_crlf(dec_str);
        tl_append(full_line, dec_str);
        xfree(dec_str);

        if (streq(line->line + (len - 2), "=\n")
            || streq(line->line + (len - 3), "=\r\n"))
            continue;

        len = tl_size(full_line);

        tmp_str = xmalloc(len + 2);
        memset(tmp_str, 0, len + 2);

        for (subline = full_line->first; subline != NULL;
             subline = subline->next)
            strcat(tmp_str, subline->line);

        tmp_str[len] = '\n';

        tl_append(dec_body, tmp_str);
        xfree(tmp_str);
        tl_clear(full_line);
    }

    tl_clear(full_line);
    xfree(full_line);
    return dec_body;

 fail:
    fglog("ERROR: decoding quoted-printable, skipping section");
    tl_clear(dec_body);
    xfree(dec_body);
    tl_clear(full_line);
    xfree(full_line);
    return NULL;
}

/* decode base64 body */

static Textlist *mime_debody_base64(Textlist * body)
{
    char *out_str;
    char *dec_str;
    char *out_ptr;
    char *dec_ptr, *dec_prev_ptr;

    Textlist *dec_body = NULL;
    Textline *line;

    int max_len = MAX_LINE_LENGTH;
    int left;
    int enc_len = 0, dec_str_len = 0;

    dec_body = xmalloc(sizeof(*dec_body));
    memset(dec_body, 0, sizeof(*dec_body));

    /* len + \n + \0 */
    out_ptr = out_str = xmalloc(max_len + 2);
    out_str[0] = '\0';
    left = max_len;

    for (line = body->first; line != NULL; line = line->next) {

        enc_len = (xstrnlen(line->line, MIME_ENC_STRING_LIMIT) / 4) * 4;
        if (mime_b64_decode(&dec_str, line->line, enc_len) == ERROR)
            goto exit;

        dec_prev_ptr = dec_str;
        while ((dec_ptr = strchr(dec_prev_ptr, '\n')) != NULL) {
            dec_str_len = dec_ptr - dec_prev_ptr + 1;
            if (dec_str_len <= left) {
                strncat(out_str, dec_prev_ptr, dec_str_len);
                dec_prev_ptr = dec_ptr + 1;
            } else {
                strncat(out_str, dec_prev_ptr, left);
                dec_prev_ptr += left;
            }

            mime_debody_flush_str(dec_body, out_str);
            left = max_len;
        }

        dec_str_len = strlen(dec_str) - (dec_prev_ptr - dec_str);
        do {
            if (dec_str_len < left) {
                strncat(out_str, dec_prev_ptr, dec_str_len);
                out_ptr += dec_str_len;
                left -= dec_str_len;
                dec_str_len = 0;
            } else {
                strncat(out_str, dec_prev_ptr, left);
                dec_prev_ptr += left;
                dec_str_len -= left;

                mime_debody_flush_str(dec_body, out_str);
                left = max_len;

            }
        } while (dec_str_len > 0);

    }
    if (left != max_len)
        mime_debody_flush_str(dec_body, out_str);

 exit:
    xfree(out_str);
    return dec_body;
}

static Textlist *mime_debody_section(Textlist * body, Textlist * header,
                                     char *to);

static Textlist *mime_debody_multipart(Textlist * body, MIMEInfo * mime,
                                       char *to)
{
    Textlist header = { NULL, NULL };
    Textline *line;

    Textlist *dec_body, *ptr_body, tmp_body = { NULL, NULL };

    char *boundary = NULL;
    char *fin_boundary = NULL;

    if (mime->type_boundary == NULL)
        return NULL;

    dec_body = xmalloc(sizeof(*dec_body));
    dec_body->first = NULL;
    dec_body->last = NULL;

    /* --boundary\0 */
    boundary = xmalloc(strlen(mime->type_boundary) + 3);
    strcpy(boundary, "--");
    strcat(boundary, mime->type_boundary);

    /* --boundary--\0 */
    fin_boundary = xmalloc(strlen(boundary) + 3);
    strcpy(fin_boundary, boundary);
    strcat(fin_boundary, "--");

    for (line = body->first; line != NULL; line = line->next)
        if (strneq(line->line, boundary, strlen(boundary)))
            break;

    for (line = line->next; line != NULL; line = line->next) {
        if (!strneq(line->line, boundary, strlen(boundary))) {
            tl_append(&tmp_body, line->line);
        } else {
            header_read_list(&tmp_body, &header);
            header_delete_from_body(&tmp_body);

            ptr_body = mime_debody_section(&tmp_body, &header, to);
            if (ptr_body != NULL) {
                tl_addtl(dec_body, ptr_body);
                if (ptr_body != &tmp_body)
                    xfree(ptr_body);
            }
            tl_clear(&tmp_body);

            if (strneq(line->line, fin_boundary, strlen(fin_boundary)))
                break;
        }
    }

    xfree(boundary);
    xfree(fin_boundary);
    return dec_body;
}

static int mime_decharset_section(Textlist * body, MIMEInfo * mime, char *to)
{

    char *res_str = NULL;
    size_t res_len, src_len;
    int rc;
    Textline *line;
    char *tmp;

    if (body == NULL || mime == NULL)
        return ERROR;

    /* if no charset, us-ascii assumed */
    if (mime->type_charset == NULL || strieq(mime->type_charset, to))
        return OK;

    for (line = body->first; line != NULL; line = line->next) {
        src_len = strlen(line->line) + 1;   /* recode final \0 also */

        rc = charset_recode_buf(&res_str, &res_len, line->line, src_len,
                                mime->type_charset, to);

        if (rc == ERROR) {
            goto exit;          /* something really wrong */
        }

        tmp = line->line;
        line->line = res_str;
        free(tmp);
    }

    rc = OK;
 exit:
    return rc;
}

static Textlist *mime_debody_section(Textlist * body, Textlist * header,
                                     char *to)
{

    Textlist *dec_body;
    MIMEInfo *mime;

    if ((mime = get_mime_from_header(header)) == NULL)
        return NULL;

    if ((mime->type_type == NULL) || strieq(mime->type_type, "text/plain")
        || (mime->type == NULL)) {
        if ((mime->encoding == NULL) || strieq(mime->encoding, "8bit")
            || strieq(mime->encoding, "7bit")) {
            dec_body = body;
        } else if (strieq(mime->encoding, "base64")) {
            dec_body = mime_debody_base64(body);
        } else if (strieq(mime->encoding, "quoted-printable")) {
            dec_body = mime_debody_qp(body);
        } else {
            fglog("WARNING: Skipped unsupported transfer encoding %s",
                  mime->encoding);
            dec_body = NULL;
            goto exit;
        }
        mime_decharset_section(dec_body, mime, to);
    } else if(strnieq(mime->type_type,
		      "multipart/", sizeof("multipart/") - 1)) {
        dec_body = mime_debody_multipart(body, mime, to);
    } else {
        fglog("WARNING: Skipped unsupported mime type  %s", mime->type_type);
        dec_body = NULL;
        goto exit;
    }
 exit:
    mime_free();
    return dec_body;

}

int mime_body_dec(Textlist * body, char *to)
{
    Textlist *dec_body;
    Textlist *header;
    int len;
    char *buf;

    if ((header = header_get_list()) == NULL)
        return ERROR;

    if ((dec_body = mime_debody_section(body, header, to)) == NULL)
        return ERROR;

    if (dec_body->first == NULL) {
        fglog("ERROR: could not decode mime body");
        xfree(dec_body);
        return ERROR;
    }

    if (dec_body != body) {
        tl_clear(body);
        *body = *dec_body;
        xfree(dec_body);
    }

    len = snprintf(NULL, 0, "text/plain; charset=%s", to);
    buf = xmalloc(len + 1);
    sprintf(buf, "text/plain; charset=%s", to);

    header_alter(header, "Content-Type", buf);
    header_alter(header, "Content-Transfer-Encoding", INTERNAL_ENCODING);

    free(buf);

    return OK;
}

void mime_free(void)
{
    MIMEInfo *mime, *n;

    for (mime = mime_list; mime; mime = n) {
        n = mime->next;

        xfree(mime->version);
        xfree(mime->type);
        xfree(mime->type_type);
        xfree(mime->type_charset);
        xfree(mime->type_boundary);
        xfree(mime->encoding);
        xfree(mime->disposition);
        xfree(mime->disposition_filename);
        xfree(mime);
    }

}
