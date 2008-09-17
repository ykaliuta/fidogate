/*
 * (C) Maint Laboratory 2000-2004
 * Author: Elohin Igor
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
/*
 * Проверка вся ли строка состоит только из цифр
 */
#include <ctype.h>
int isnum(char *s)
{
    register char *c;

    c = s;
    while (*c) {
	if (!isdigit(*c)) return 0;
	c++;
    }
    return 1;
}
