/*
 */
#include <stdarg.h>
#include "buffered-printf.h"

#define DEBUG_BUF 0
#define LOG_BUF 1

int verbose = 0;

void debug_set_verbose(int v)
{
    verbose = v;
}

void debug(int lvl, const char *fmt, ...)
{
    int buffer_id = DEBUG_BUF;
    va_list ap;

    va_start(ap, fmt);
    bp_vprintf(buffer_id, fmt, ap);
    bp_printf(buffer_id, "\n");
    va_end(ap);
}

char *debug_buffer(void)
{
    return bp_buffer_content(DEBUG_BUF);
}

void debug_buffer_release(void)
{
    bp_buffer_release(DEBUG_BUF);
}

void fglog(const char *fmt, ...)
{
    int buffer_id = LOG_BUF;
    va_list ap;

    va_start(ap, fmt);
    bp_vprintf(buffer_id, fmt, ap);
    bp_printf(buffer_id, "\n");
    va_end(ap);
}

char *log_buffer(void)
{
    return bp_buffer_content(LOG_BUF);
}

void log_buffer_release(void)
{
    bp_buffer_release(LOG_BUF);
}

void log_program(char *name)
{
}
