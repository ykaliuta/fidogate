/*
 * (C) Maint Laboratory 2003-2004
 * Author: Elohin Igor'
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#define BUFSIZE 512
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define VERSION " v 0.6.2"
#define MSGBUF 512

void loginit(char *, char *);
void logclose(void);
void error(const char *fmt, ...);
void message(const char *fmt, ...);
void notice(const char *fmt, ...);
void dolog(int flag, const char *fmt, ...);
void die(int);
void put_server(char *);
int get_server(char *);
char *get_line(char *);
void putaline(const char *fmt, ...);
char *diagtime(void);

#endif
