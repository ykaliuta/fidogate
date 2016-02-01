/*
 *
 *  buffer2c.c
 *
 *  January 30, 2016 Yauheni Kaliuta
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define DEFAULT_LIMIT 68

struct ctx {
    FILE *f;
    size_t cur_len;
    size_t limit;
    char *b;
    size_t buf_size;
};

int printed_len;

static struct ctx *open_stream(size_t limit)
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
    ctx->limit = limit ? limit : DEFAULT_LIMIT;

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
#define make_cr_if_needed(ctx, chunk_size) do {			\
	if ((ctx->limit > 0) &&					\
	    ctx->cur_len + chunk_size + 1 > ctx->limit)		\
	    out_cr(ctx);					\
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

char *buffer2c_limit(char *buf, size_t limit)
{
    struct ctx *ctx;
    char *b;

    if (buf == NULL)
	return NULL;

    ctx = open_stream(limit);
    if (ctx == NULL)
	return NULL;

    out_header(ctx);

    for (; *buf != '\0'; buf++) {
	if (*buf == '"') {
	    out_quote(ctx);
	    continue;
	}
	if (*buf == '\n') {
	    out_eol(ctx);
	    continue;
	}
	if (!isprint(*buf)) {
	    out_hex(ctx, *buf);
	    continue;
	}
	out_char(ctx, *buf);
    }

    out_footer(ctx);
    b = close_stream(ctx);
    return b;
}

char *buffer2c(char *buf)
{
    return buffer2c_limit(buf, 0);
}

