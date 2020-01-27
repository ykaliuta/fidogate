/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Get system date/time. Taken from ifmail 1.7 / inn 1.4 and adopted
 * for FIDOGATE .
 *
 *****************************************************************************/
/*  Original %Revision: 1.4 %
**
*/

#include "fidogate.h"

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif                          /* HAVE_SYS_TIME_H */

void GetTimeInfo(TIMEINFO * Now)
{
    static time_t LastTime;
    static long LastTzone;
    struct tm *tm;
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tv;
#endif
#ifndef HAVE_TM_GMTOFF
    struct tm local;
    struct tm gmt;
#endif

    /* Get the basic time. */
#ifdef HAVE_GETTIMEOFDAY
    if (gettimeofday(&tv, (struct timezone *)NULL) == -1)
        return;
    Now->time = tv.tv_sec;
    Now->usec = tv.tv_usec;
#else
    /* Can't check for -1 since that might be a time, I guess. */
    (void)time(&Now->time);
    Now->usec = 0;
#endif

    /* Now get the timezone if it's been an hour since the last time. */
    if (Now->time - LastTime > 60 * 60) {
        LastTime = Now->time;
        if ((tm = localtime(&Now->time)) == NULL)
            return;
#ifndef HAVE_TM_GMTOFF
        /* To get the timezone, compare localtime with GMT. */
        local = *tm;
        if ((tm = gmtime(&Now->time)) == NULL)
            return;
        gmt = *tm;

        /* Assume we are never more than 24 hours away. */
        LastTzone = gmt.tm_yday - local.tm_yday;
        if (LastTzone > 1)
            LastTzone = -24;
        else if (LastTzone < -1)
            LastTzone = 24;
        else
            LastTzone *= 24;

        /* Scale in the hours and minutes; ignore seconds. */
        LastTzone += gmt.tm_hour - local.tm_hour;
        LastTzone *= 60;
        LastTzone += gmt.tm_min - local.tm_min;
#else
        LastTzone = (0 - tm->tm_gmtoff) / 60;
#endif
    }
    Now->tzone = LastTzone;
    return;
}
