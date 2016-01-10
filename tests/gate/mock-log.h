/*
 */

#ifndef MOCK_LOG_H_
#define MOCK_LOG_H_

void debug_set_verbose(int v);
void debug(int lvl, const char *fmt, ...);
char *debug_buffer(void);
void debug_buffer_release(void);
void fglog(const char *fmt, ...);
char *log_buffer(void);
void log_buffer_release(void);
void log_program(char *name);

#endif
