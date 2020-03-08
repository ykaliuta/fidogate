/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * date() date/time print function
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
 *  _____ _____
 * |     |___  |   Martin Junius             <mj@fidogate.org>
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

static char *get_tz_name(struct tm *);

/*
 * Get name of current time zone
 */
static char *get_tz_name(struct tm *tm)
{
#ifdef HAVE_STRFTIME
    static char buf[32];

    strftime(buf, sizeof(buf), "%Z", tm);
    return buf;
#else
#ifdef HAVE_TM_ZONE
    return tm->tm_zone;
#else
#ifdef HAVE_TZNAME
    return tm->tm_isdst > 0 ? tzname[1] : tzname[0];
#endif
#endif
#endif
}

/* Format date/time with TZ info from ^ATZUTC kludge */
char *date_tz(char *fmt, time_t * t, char *tz_str)
{
    static char buf[128];
    long tz = -1;
    unsigned hours;
    unsigned mins;
    long mult = 1;               /* or -1 */
    int rc;

    if (tz_str == NULL)
        goto out;

    if (tz_str[0] == '+') {     /* should not be */
        tz_str++;
    } else if (tz_str[0] == '-') {
        mult = -1;
        tz_str++;
    }

    rc = sscanf(tz_str, "%02u%02u", &hours, &mins);
    if (rc != 2) {
        fglog("WARNING: could not parse TZUTC: %s", tz_str);
        goto out;
    }

    /*
     * minus since TIMEINFO keeps difference = UTC - local,
     * while local = UTC + TZUTC => TZUTC = local - UTC
     */
    tz = -mult * (hours * 60 + mins);
 out:
    return date_buf(buf, sizeof(buf), fmt, t, tz);
}

/*
 * Format date/time according to strftime() format string
 */
char *date(char *fmt, time_t * t)
{
    return date_tz(fmt, t, NULL);
}

/* @tz in minutes */
char *date_buf(char *buf, size_t len, char *fmt, time_t * t, long tz)
{
    TIMEINFO ti;
    struct tm *tm;
    time_t adjusted_time;

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
    if (fmt == NULL && t && *t == -1)
        return "INVALID";

    if (tz != -1)
	timezone = tz * 60;

    GetTimeInfo(&ti);

    if (t)
        ti.time = *t;
    if (tz != -1)
        ti.tzone = tz;

    adjusted_time = ti.time - (ti.tzone * 60);
    tm = gmtime(&adjusted_time);

    /* Default format string */
    if (!fmt)
        fmt = DATE_DEFAULT;

    /*
     * Partial strftime() format implementation with additional
     *   %O    time difference to UTC, format [+-]hhmm,
     *         e.g. +0100 for MET, +0200 for MET DST
     */
    *p = 0;
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case 'a':          /* Abbr. weekday */
                s = weekdays[tm->tm_wday];
                break;
                /* A not implemented */
            case 'b':          /* Abbr. month */
                s = months[tm->tm_mon];
                break;
                /* B not implemented */
                /* c not implemented */
            case 'd':          /* Day of month */
                str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_mday);
                s = sbuf;
                break;
            case 'H':          /* Hour (24h) */
                str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_hour);
                s = sbuf;
                break;
#ifdef FTS_VIA
            case 'G':          /* Hour (24h) UTC */
                off = -ti.tzone;
                off = off < 0 ? -off : off;
                hour = off / 60;
                str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_hour - hour);
                s = sbuf;
                break;
            case 'V':          /* Hour (24h) UTC */
                off = -ti.tzone;
                off = off < 0 ? -off : off;
                min = off % 60;
                str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_min - min);
                s = sbuf;
                break;
            case 'U':          /* UTC */
                s = "UTC";
                break;
#endif                          /* FTS_VIA */
                /* I not implemented */
            case 'j':          /* Day of year */
                str_printf(sbuf, sizeof(sbuf), "%03d", tm->tm_yday);
                s = sbuf;
                break;
            case 'm':          /* Month */
                str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_mon + 1);
                s = sbuf;
                break;
            case 'M':          /* Minutes */
                str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_min);
                s = sbuf;
                break;
                /* p not implemented */
            case 'S':          /* Seconds */
                str_printf(sbuf, sizeof(sbuf), "%02d", tm->tm_sec);
                s = sbuf;
                break;
                /* U not implemented */
            case 'w':          /* Day of week */
                str_printf(sbuf, sizeof(sbuf), "%d", tm->tm_wday);
                s = sbuf;
                break;
                /* W not implemented */
            case 'x':          /* Date */
                str_printf(sbuf, sizeof(sbuf), "%s %2d %4d",
                           months[tm->tm_mon], tm->tm_mday, tm->tm_year + 1900);
                s = sbuf;
                break;
            case 'X':          /* Time */
                str_printf(sbuf, sizeof(sbuf), "%02d:%02d:%02d",
                           tm->tm_hour, tm->tm_min, tm->tm_sec);
                s = sbuf;
                break;
            case 'y':          /* Year 00-99 */
                str_printf(sbuf, sizeof(sbuf), "%02d", (tm->tm_year % 100));
                s = sbuf;
                break;
            case 'Y':          /* Year 1900 ... */
                str_printf(sbuf, sizeof(sbuf), "%4d", 1900 + tm->tm_year);
                s = sbuf;
                break;
            case 'Z':          /* Time zone */
                s = get_tz_name(tm);
                break;
        /***** Additional %O format *****/
            case 'N':          /* Time diff to TZUTC */
                off = -ti.tzone;
                cc = off >= 0 ? '+' : '-';
                off = off < 0 ? -off : off;
                hour = off / 60;
                min = off % 60;
                if (cc != '+')
                    str_printf(sbuf, sizeof(sbuf), "%c%02d%02d", cc, hour, min);
                else
                    str_printf(sbuf, sizeof(sbuf), "%02d%02d", hour, min);
                s = sbuf;
                break;
            case 'O':          /* Time diff to UTC */
                off = -ti.tzone;
                cc = off >= 0 ? '+' : '-';
                off = off < 0 ? -off : off;
                hour = off / 60;
                min = off % 60;
                str_printf(sbuf, sizeof(sbuf), "%c%02d%02d", cc, hour, min);
                s = sbuf;
                break;
            default:
                sbuf[0] = *fmt;
                sbuf[1] = 0;
                s = sbuf;
                break;
            }
        } else {
            sbuf[0] = *fmt;
            sbuf[1] = 0;
            s = sbuf;
        }

        str_append(buf, len, s);
        fmt++;
    }

    return buf;
}

/*
 * Returns tz tail of rfc822 date or +0000 for UTC/GMT
 * Sun, 8 Mar 2020 17:00:23 -0600         -> -0600
 * Mon,  9 Mar 2020 00:05:20 +0100 (CET)  -> +0100 (CET)
 * Sun, 8 Mar 2020 16:19:22 -0700 (PDT)   -> -0700 (PDT)
 * 8 Mar 2020 23:34:03 GMT                -> +0000
 * Sun, 08 Mar 2020 22:30:55 UTC          -> +0000
 */
char *date_rfc_tz(char *rfc_date)
{
    char *p;

    if (rfc_date == NULL)
	return NULL;

    p = strrchr(rfc_date, ' ');
    if (p == NULL)
	return NULL;

    if ((p[1] == '+') || (p[1] == '-'))
	return p + 1;

    if (strneq(p + 1, "GMT", sizeof("GMT") - 1)
	|| strneq(p + 1, "UTC", sizeof("UTC") - 1))
	return "+0000";

    /* second word */
    for (p--; p > rfc_date; p--) {
	if (*p  == ' ')
	    break;
    }

    if (*p != ' ')
	return NULL;

    if ((p[1] == '+') || (p[1] == '-'))
	return p + 1;

    return NULL;
}
