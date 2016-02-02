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

struct ctx {
    FILE *f;
    size_t cur_len;
    size_t str_len;
    char *b;
    size_t buf_size;
    char *input;
    size_t input_size;
    char *end;
};

int printed_len;

static struct ctx *open_stream(char *buf, size_t size, size_t str_len)
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
    ctx->str_len = str_len ? str_len : DEFAULT_LIMIT;
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

static char last_char(struct ctx *ctx)
{
    fflush(ctx->f);
    return ctx->b[ctx->buf_size - 1];
}

static void out_cr(struct ctx *ctx)
{
    char quote = '"';

    if (last_char(ctx) != quote)
	putc(quote, ctx->f);

    putc('\n', ctx->f);
    putc(quote, ctx->f);

    ctx->cur_len = 1;
}

/* +1 for the last " */
#define make_cr_if_needed(ctx, chunk_size) do {		  \
	if ((ctx->str_len > 0) &&			  \
	    ctx->cur_len + chunk_size + 1 > ctx->str_len) \
	    out_cr(ctx);				  \
    } while (0)

static void out_quote(struct ctx *ctx)
{
    make_cr_if_needed(ctx, 2);
    fprintf(ctx->f, "\\\"");
    ctx->cur_len += 2;
}

static void out_header(struct ctx *ctx)
{
    fprintf(ctx->f, "\"");
    ctx->cur_len += 1;
}

static void out_footer(struct ctx *ctx)
{
    make_cr_if_needed(ctx, 1);
    fprintf(ctx->f, "\"");
    ctx->cur_len += 1;
}

static void out_char(struct ctx *ctx, char c)
{
    make_cr_if_needed(ctx, 1);
    putc(c, ctx->f);
    ctx->cur_len += 1;
}

/* finish and start ", from new line if needed */
static void retrigger_literal(struct ctx *ctx)
{
    make_cr_if_needed(ctx, 2);
    if (ctx->cur_len > 1) {
	putc('"', ctx->f);
	putc('"', ctx->f);
    }
}

static void out_hex(struct ctx *ctx, char c)
{
    make_cr_if_needed(ctx, 4); /* \x01 */
    printed_len = fprintf(ctx->f, "\\x%02x", c);
    ctx->cur_len += printed_len;
    retrigger_literal(ctx);
}

static void out_eol(struct ctx *ctx)
{
    make_cr_if_needed(ctx, 2);
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

char *buffer2c_size_limit(char *buf, size_t size, size_t str_len)
{
    struct ctx *ctx;
    char *b;

    if (buf == NULL)
	return NULL;

    ctx = open_stream(buf, size, str_len);
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

char *buffer2c_limit(char *buf, size_t str_len)
{
    return buffer2c_size_limit(buf, 0, str_len);
}

char *buffer2c(char *buf)
{
    return buffer2c_limit(buf, 0);
}

