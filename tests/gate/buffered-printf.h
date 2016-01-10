/*
 */

#ifndef BUFFERED_PRINTF_H_
#define BUFFERED_PRINTF_H_

#include <stdarg.h>

char *bp_buffer_content(int id);
int bp_vprintf(int id, const char *fmt, va_list ap);
int bp_printf(int id, const char *fmt, ...);
void bp_buffer_release(int id);

#endif
