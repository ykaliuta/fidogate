/*
 * (C) Maint Laboratory 2003-2004
 * Author: Elohin Igor'
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#define READ_CONFIG
#include <stdio.h>
#include <stdlib.h>
#include "getini.h"
#define INIBUF 512

int getIniString(FILE *fd, char *section, char *name, char *string, unsigned char flagNEXT)
{
    char buf[INIBUF];
    char *s;
    unsigned char flagFOUND;

    if(flagNEXT) goto next;
    rewind(fd);

    flagFOUND = 0;
    while(fgets(buf, INIBUF - 1, fd) != NULL){
	s = strchr(buf, '\n');
        if(s != NULL) *s = '\0';
	alltrim(buf);
	if(buf[0] == '\0' || buf[0] == '#') continue;
	if(strcmp(buf, section) == 0){
            flagFOUND = 1;
	    break;
	}
    }
    if(!flagFOUND) return 1;
next:
    while(fgets(buf, INIBUF - 1, fd) != NULL){
	s = strchr(buf, '\n');
	if(s != NULL) *s = '\0';
        alltrim(buf);
	if(buf[0] == '\0' || buf[0] == '#') continue;
	if(buf[0] == '[' && buf[strlen(buf) - 1] == ']') return 1;
        s = strchr(buf, '#');
        if(s != NULL) *s = '\0';
        alltrim(buf);
	s = strchr(buf, '=');
        if(s == NULL) return 1;
	*s = '\0';
        s++;
	alltrim(buf);
	if(strcmp(buf, name) == 0){
	    alltrim(s);
	    strcpy(string, s);
            return 0;
	}
    }
    return 1;
}
int getIniInt(FILE *fd, char *section, char *name, int *value)
{
    char buf[INIBUF];

    if(getIniString(fd, section, name, buf, 0)) return 1;
    if(!isnum(buf)) return 1;
    *value = atoi(buf);
    return 0;
}
int getIniBool(FILE *fd, char *section, char *name, unsigned char *value)
{
    char buf[INIBUF];
    if(getIniString(fd, section, name, buf, 0)) return 1;
    if(strcmp(buf, "true") == 0) *value = TRUE;
    else if(strcmp(buf, "false") == 0) *value = FALSE;
    return 1;
}

