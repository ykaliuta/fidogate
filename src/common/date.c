/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * $Id: date.c,v 4.11 2004/08/22 20:19:11 n0ll Exp $
 *
 * date() date/time print function
 *
 *****************************************************************************
 * Copyright (C) 1990-2004
 *  _____ _____
 * |     |___  |   Martin Junius             <mj.at.n0ll.dot.net>
 * | | | |   | |   Radiumstr. 18
 * |_|_|_|@home|   D-51069 Koeln, Germany
 *
 * This file is part of FIDOGATE.
 *
 * FIDOGATE is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * FIDOGATE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FIDOGATE; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/

#include "fidogate.h"



#define DST_OFFSET	1

static char *get_tz_name (struct tm *);



/*
 * Get name of current time zone
 */
static char *get_tz_name(struct tm *tm)
{
#ifdef HAS_STRFTIME
    static char buf[32];
    
    strftime(buf, sizeof(buf), "%Z", tm);
    return buf;
#endif

#ifdef HAS_TM_ZONE
    return tm->tm_zone;
#endif

#ifdef HAS_TZNAME
    return tm->tm_isdst > 0 ? tzname[1] : tzname[0];
#endif
}
    


/*
 * Format date/time according to strftime() format string
 */
char *date(char *fmt, time_t *t)
{
    static char buf[128];

    return date_buf(buf, sizeof(buf), fmt, t);
}


char *date_buf(char *buf, size_t len, char *fmt, time_t *t)
{
    TIMEINFO ti;
    struct tm *tm;

    /* names for weekdays */
    static char *weekdays[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
    };
    /* names for months */
    static char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };
    char *p = buf;
    char *s, sbuf[16];
    int hour, min, off;
    char cc;

    /* Check for invalid time (-1) */
    if(fmt==NULL && t && *t==-1)
	return "INVALID";
    
    GetTimeInfo(&ti);
    tm = localtime(&ti.time);
    if(tm->tm_isdst)
	ti.tzone += DST_OFFSET * 60;

    if(t)
	ti.time = *t;
    tm = localtime(&ti.time);
    if(tm->tm_isdst)
	ti.tzone -= DST_OFFSET * 60;

    /* Default format string */
    if(!fmt)
	fmt = DATE_DEFAULT;

    /*
     * Partial strftime() format implementation with additional
     *   %O    time difference to UTC, format [+-]hhmm,
     *         e.g. +0100 for MET, +0200 for MET DST
     */
    *p = 0;
    while(*fmt)
    {
	if(*fmt == '%')
	{
	    fmt++;
	    switch (*fmt)
	    {
	    case 'a':					/* Abbr. weekday */
		s = weekdays[tm->tm_wday];	break;
	    /* A not implemented */
	    case 'b':					/* Abbr. month */
		s = months[tm->tm_mon];		break;
	    /* B not implemented */
	    /* c not implemented */
	    case 'd':					/* Day of month */
		str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_mday);
		s = sbuf;			break;
	    case 'H':					/* Hour (24h) */
		str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_hour);
		s = sbuf;			break;
	    /* I not implemented */
	    case 'j':					/* Day of year */
		str_printf(sbuf, sizeof(sbuf), "%03d", tm->tm_yday);
		s = sbuf;			break;
	    case 'm':					/* Month */
		str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_mon + 1);
		s = sbuf;			break;
	    case 'M':					/* Minutes */
		str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_min);
		s = sbuf;			break;
	    /* p not implemented */
	    case 'S':					/* Seconds */
		str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_sec);
		s = sbuf;			break;
	    /* U not implemented */
	    case 'w':					/* Day of week */
		str_printf(sbuf, sizeof(sbuf), "%d", tm->tm_wday);
		s = sbuf;			break;
	    /* W not implemented */
	    case 'x':					/* Date */
		str_printf(sbuf, sizeof(sbuf), "%s %2d %4d",
			   months[tm->tm_mon],
			   tm->tm_mday, tm->tm_year+1900);
		s = sbuf;			break;
	    case 'X':					/* Time */
		str_printf(sbuf, sizeof(sbuf), "%02d:%02d:%02d",
			   tm->tm_hour, tm->tm_min, tm->tm_sec);
		s = sbuf;			break;
	    case 'y':					/* Year 00-99 */
		str_printf(sbuf, sizeof(sbuf), "%02d", (tm->tm_year % 100) );
		s = sbuf;			break;
	    case 'Y':					/* Year 1900 ... */
		str_printf(sbuf, sizeof(sbuf), "%4d", 1900 + tm->tm_year);
		s = sbuf;			break;
	    case 'Z':					/* Time zone */
		s = get_tz_name(tm);		break;
	    /***** Additional %O format *****/
	    case 'O':					/* Time diff to UTC */
		off  = - ti.tzone;
		cc   = off >= 0 ? '+'  : '-';
		off  = off <  0 ? -off : off;
		hour = off / 60;
		min  = off % 60;
		str_printf(sbuf, sizeof(sbuf), "%c%02d%02d", cc, hour, min);
		s    = sbuf;
		break;
	    default:
		sbuf[0] = *fmt;
		sbuf[1] = 0;
		s       = sbuf;
		break;
	    }
	}
	else 
	{
	    sbuf[0] = *fmt;
	    sbuf[1] = 0;
	    s       = sbuf;
	}

        str_append(buf, len, s);
	fmt++;
    }

    return buf;
}
