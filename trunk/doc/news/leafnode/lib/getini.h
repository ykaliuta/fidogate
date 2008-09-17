/*
 * (C) Maint Laboratory 2003-2004
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */

int getIniInt(FILE *, char *, char *, int *);
int getIniBool(FILE *, char *, char *, unsigned char *);
int getIniString(FILE *, char *, char *, char *, unsigned char);
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define getIniStr(p1, p2, p3, p4) getIniString((p1), (p2), (p3), (p4), 0)
#define getIniStrNext(p1, p2, p3, p4) getIniString(p1, p2, p3, p4, 1)

