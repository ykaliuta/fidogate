/*
 * (C) Maint Laboratory 2003-2004
 * Author: Elohin Igor'
 * e-mail: maint@unona.ru
 * fido  : 2:5070/222.52
 * URL   : http://maint.unona.ru
 */
#include <stdio.h>
#include <time.h>

char *diagtime()
{
	struct tm *tm;
	time_t clock;
        static char buf[1028];
	
	tzset();
	clock = time(NULL);
	tm = localtime(&clock);
	tm->tm_mon++;
        tm->tm_year -= 100;
	snprintf(buf, sizeof(buf), "%.2d.%.2d.%.2d %.2d:%.2d:%.2d",
		tm->tm_mday,
		tm->tm_mon,
		tm->tm_year,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);
	return buf;
}
