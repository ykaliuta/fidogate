/*
 * (C) Maint Laboratory 2003
 * Author: Elohin Igor
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
	snprintf(buf, sizeof(buf), "%s%d.%s%d.%s%d %s%d:%s%d:%s%d",
		tm->tm_mday > 9 ? "":"0",
		tm->tm_mday,
		tm->tm_mon > 9 ? "":"0",
		tm->tm_mon,
		tm->tm_year > 9 ? "":"0",
		tm->tm_year,
		tm->tm_hour > 9 ? "":"0",
		tm->tm_hour,
		tm->tm_min > 9 ? "":"0",
		tm->tm_min,
		tm->tm_sec > 9 ? "":"0",
		tm->tm_sec);
	return buf;
}
