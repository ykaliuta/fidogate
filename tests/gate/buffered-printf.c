/*
 */

#define _GNU_SOURCE

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#define DEFAULT_SIZE 128

static void
_fatal(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, fmt, ap);
    va_end(ap);
    abort();
}

#define fatal(fmt, ...) _fatal("[%s:%d]: " fmt,			\
			       __func__, __LINE__, ##__VA_ARGS__)

/* prefix bp_ == buffered printf */
struct bp_buffer {
    int id;
    size_t size;
    size_t len;
    char *buf;
    LIST_ENTRY(bp_buffer) entry;
};
LIST_HEAD(bp_buffer_list, bp_buffer);

static struct bp_buffer_list buffers = LIST_HEAD_INITIALIZER(buffers);

static void
bp_buffer_list_add(struct bp_buffer *b)
{
    struct bp_buffer_list *list = &buffers;

    LIST_INSERT_HEAD(list, b, entry);
}

static void
bp_buffer_list_remove(struct bp_buffer *b)
{
    LIST_REMOVE(b, entry);
}

static struct bp_buffer *
bp_buffer_find(int id)
{
    struct bp_buffer *b;
    struct bp_buffer_list *list  = &buffers;

    LIST_FOREACH(b, list, entry) {
	if (b->id == id)
	    return b;
    }
    return NULL;
}

static struct bp_buffer *
bp_buffer_new(int id)
{
    struct bp_buffer *b;
    char *p;
    size_t size = DEFAULT_SIZE;

    b = malloc(sizeof(*b));
    if (b == NULL)
	return NULL;

    p = malloc(size);
    if (p == NULL)
	goto err;
    p[0] = '\0';

    b->id = id;
    b->size = size;
    b->len = 0;
    b->buf = p;

    return b;
err:
    free(b);
    return NULL;
}

static void
bp_buffer_destroy(struct bp_buffer *b)
{
    if (b == NULL)
	return;
    free(b->buf);
    free(b);
}

static struct bp_buffer *
_bp_buffer_get_or_create(int id)
{
    struct bp_buffer *b;

    b = bp_buffer_find(id);
    if (b != NULL)
	return b;

    b = bp_buffer_new(id);
    if (b == NULL)
	return NULL;

    bp_buffer_list_add(b);

    return b;
}

static struct bp_buffer *
bp_buffer_get_or_create(int id)
{
    struct bp_buffer *b;

    b = _bp_buffer_get_or_create(id);
    if (b == NULL)
	fatal("Could not find buffer %d", id);

    return b;
}

static size_t
bp_buffer_calculate_new_size(struct bp_buffer *b, size_t len)
{
    size_t size = b->size;
    size_t required = b->len + len + 1;

    while (required > size)
	size *= 2;
    return size;
}

static void
bp_buffer_extend(struct bp_buffer *b, size_t len)
{
    size_t new_size;
    char *p;

    new_size = bp_buffer_calculate_new_size(b, len);

    p = realloc(b->buf, new_size);
    if (p == NULL)
	fatal("Could not realloc buffer");

    b->buf = p;
    b->size = new_size;
}

static void
_bp_buffer_add(struct bp_buffer *b, char *str, size_t len)
{
    char *p;

    if (b->len + len + 1 > b->size)
	bp_buffer_extend(b, len);

    p = b->buf + b->len;
    memcpy(p, str, len);
    b->len += len;
    b->buf[b->len] = '\0';
}

static void
bp_buffer_add(int id, char *str)
{
    struct bp_buffer *b;
    size_t len;

    b = bp_buffer_get_or_create(id);
    len = strlen(str);
    _bp_buffer_add(b, str, len);
}

char *
bp_buffer_content(int id)
{
    struct bp_buffer *b;

    b = bp_buffer_get_or_create(id);

    return b->buf;
}

int
bp_vprintf(int id, const char *fmt, va_list ap)
{
    char *p;
    int r;

    r = vasprintf(&p, fmt, ap);
    if (r == -1)
	fatal("Could create print string");

    bp_buffer_add(id, p);
    free(p);
    return r;
}

int
bp_printf(int id, const char *fmt, ...)
{
    va_list ap;
    int r;

    va_start(ap, fmt);
    r = bp_vprintf(id, fmt, ap);
    va_end(ap);
    return r;
}

void
bp_buffer_release(int id)
{
    struct bp_buffer *b;

    b = bp_buffer_find(id);
    if (b == NULL)
	return;

    bp_buffer_list_remove(b);
    bp_buffer_destroy(b);
}
