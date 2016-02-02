/*
 *
 *  buffer2c.c
 *
 *  January 30, 2016 Yauheni Kaliuta
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define DEFAULT_LIMIT 68

enum state {
    QUOTE_CLOSED,
    QUOTE_OPENED,
};

struct ctx {
    FILE *f;
    size_t cur_len;
    size_t str_limit;
    enum state state;
    char *b;
    size_t buf_size;
    char *input;
    size_t input_size;
    char *end;
};

int printed_len;

static struct ctx *open_stream(char *buf, size_t size, size_t str_limit)
{
    struct ctx *ctx;
    FILE *f;

    ctx = malloc(sizeof(*ctx));
    if (ctx == NULL)
	return NULL;

    f = open_memstream(&ctx->b, &ctx->buf_size);
    if (f == NULL)
	goto err;

    ctx->f = f;
    ctx->cur_len = 0;
    ctx->state = QUOTE_CLOSED;
    ctx->str_limit = str_limit ? str_limit : DEFAULT_LIMIT;
    ctx->input = buf;
    ctx->input_size = size;
    ctx->end = buf + size;

    return ctx;
err:
    free(ctx);
    return NULL;
}

static char *close_stream(struct ctx *ctx)
{
    char *b;

    fclose(ctx->f);
    b = ctx->b;
    free(ctx);

    return b;
}

static void trigger_quote_state(struct ctx *ctx)
{
    ctx->state =
	ctx->state == QUOTE_OPENED ? QUOTE_CLOSED : QUOTE_OPENED;
}

static void out_cr(struct ctx *ctx)
{
    char quote = '"';

    if (ctx->state == QUOTE_OPENED)
	putc(quote, ctx->f);

    putc('\n', ctx->f);
    putc(quote, ctx->f);
    ctx->state = QUOTE_OPENED;

    ctx->cur_len = 1;
}

/* +1 for the last " */
static void make_cr_if_needed(struct ctx *ctx, size_t chunk_size)
{
    if ((ctx->str_limit > 0) &&
	ctx->cur_len + chunk_size + 1 > ctx->str_limit)
	out_cr(ctx);
}

static void open_quote_if_needed(struct ctx *ctx)
{
    if (ctx->state == QUOTE_CLOSED) {
	putc('"', ctx->f);
	ctx->cur_len++;
    }
    ctx->state = QUOTE_OPENED;
}

static void out_header(struct ctx *ctx)
{
    fprintf(ctx->f, "\"");
    ctx->cur_len += 1;
    trigger_quote_state(ctx);
}

static void out_footer(struct ctx *ctx)
{
    if (ctx->state == QUOTE_CLOSED)
	return;

    make_cr_if_needed(ctx, 1);
    putc('"', ctx->f);
    ctx->cur_len += 1;
    ctx->state = QUOTE_CLOSED;
}

static void out_quote(struct ctx *ctx)
{
    make_cr_if_needed(ctx, 2);
    open_quote_if_needed(ctx);
    fprintf(ctx->f, "\\\"");
    ctx->cur_len += 2;
}

static void out_char(struct ctx *ctx, char c)
{
    make_cr_if_needed(ctx, 1);
    open_quote_if_needed(ctx);
    putc(c, ctx->f);
    ctx->cur_len += 1;
}

static void out_hex(struct ctx *ctx, unsigned char c)
{
    make_cr_if_needed(ctx, 5); /* \x01 */
    open_quote_if_needed(ctx);
    printed_len = fprintf(ctx->f, "\\x%02x\"", c);
    ctx->cur_len += printed_len;
    ctx->state = QUOTE_CLOSED;
}

static void out_eol(struct ctx *ctx)
{
    make_cr_if_needed(ctx, 2);
    open_quote_if_needed(ctx);
    fprintf(ctx->f, "\\n\"\n\"");
    ctx->cur_len = 1;
}

static bool end_of_input(struct ctx *ctx)
{
    bool r;

    if (ctx->input_size == 0)
	r = *ctx->input == '\0';
    else
	r = ctx->input == ctx->end;
    return r;
}

static char cur_input(struct ctx *ctx)
{
    return *ctx->input;
}

static void progress_input(struct ctx *ctx)
{
    ++ctx->input;
}

char *buffer2c_size_limit(char *buf, size_t size, size_t str_limit)
{
    struct ctx *ctx;
    char *b;

    if (buf == NULL)
	return NULL;

    ctx = open_stream(buf, size, str_limit);
    if (ctx == NULL)
	return NULL;

    out_header(ctx);

    for (; !end_of_input(ctx); progress_input(ctx)) {
	if (cur_input(ctx) == '"') {
	    out_quote(ctx);
	    continue;
	}
	if (cur_input(ctx) == '\n') {
	    out_eol(ctx);
	    continue;
	}
	if (!isprint(cur_input(ctx))) {
	    out_hex(ctx, cur_input(ctx));
	    continue;
	}
	out_char(ctx, cur_input(ctx));
    }

    out_footer(ctx);
    b = close_stream(ctx);
    return b;
}

char *buffer2c_limit(char *buf, size_t str_limit)
{
    return buffer2c_size_limit(buf, 0, str_limit);
}

char *buffer2c(char *buf)
{
    return buffer2c_limit(buf, 0);
}
