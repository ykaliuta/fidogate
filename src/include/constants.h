/*****************************************************************************
 * FIDOGATE --- Gateway software UNIX <-> FIDO
 *
 *  September 27, 2016 Yauheni Kaliuta
 *
 * Constants, moved from old config.h and fidogate.h
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

#ifndef FIDOGATE_CONSTANTS_H
#define FIDOGATE_CONSTANTS_H

#include "config.h"

#ifdef HAVE_SYSEXITS_H

#include <sysexits.h>           /* EX_* defines */

#else

/* BSD error codes (used by sendmail */
#define EX_OK		 0          /* successful termination */
#define EX_USAGE	64          /* command line usage error */
#define EX_DATAERR	65          /* data format error */
#define EX_NOINPUT	66          /* cannot open input */
#define EX_NOHOST	68          /* host name unknown */
#define EX_UNAVAILABLE	69      /* service unavailable */
#define EX_SOFTWARE	70          /* internal software error */
#define EX_OSERR	71          /* system error (e.g., can't fork) */
#define EX_OSFILE	72          /* critical OS file missing */
#define EX_CANTCREAT	73      /* can't create (user) output file */
#define EX_IOERR	74          /* input/output error */

#endif                          /* HAVE_SYSEXITS_H */

/*
 * Exit codes used by FIDOGATE
 */
#define EXIT_OK		0           /* successful */
#define EXIT_ERROR	1           /* error */
#define EXIT_BUSY	2           /* program already running */
#define EXIT_CONTINUE	3       /* still work to do */

#define EXIT_KILL	32          /* killed: exit code = 32 + signum */

/*
 * Values
 */
#define TRUE		1
#define FALSE		0
#define FIRST		1
#define NEXT		0
#define OK		0
#define ERROR		(-1)
#define EMPTY		(-1)
#define INVALID		(-1)
#define WILDCARD	(-2)

#define MAX_LINE_LENGTH		32768

/*
 * Standard FIDONET domain for Z1-6 Message-IDs
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!! DON'T TOUCH THIS, IF YOU'RE NOT ABSOLUTELY SURE WHAT YOU'RE DOING !!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */
#define MSGID_FIDONET_DOMAIN	".fidonet.org"

/****************************************************************************
 *                                                                          *
 *              Think twice before changing anything below!!!               *
 *                                                                          *
 ****************************************************************************/

/*
 * Permissions
 */
#define PACKET_MODE	0660        /* Mode for outbound packets */
#define BSY_MODE	0664        /* Mode for BSY files */
#define FLO_MODE	0664        /* Mode for FLO files */
#define DIR_MODE	0775        /* Mode for directories */
#define FILE_DIR_MODE	0770    /* Mode for create fileecho directories */
#define CONF_MODE	0660        /* Mode for written config files */

/*
 * RFC headers recognized at beginning of FTN message body
 */
#define FTN_RFC_HEADERS \
    "From:", "Reply-To:", "UUCPFROM:", "To:", "Cc:", "Bcc:", \
    "Newsgroups:", "Sender:", "Content-Transfer-Encoding:", \
    "Header-To:", "Header-Cc:", "Subject:", "User-Agent:", \
    "X-Mailer:", "X-Newsreader:", "In-Reply-To:", "References:"

/*
 * RFC headers output for ^ARFC level 1 (partial RFC headers)
 */
#define RFC_LVL_1_HEADERS \
    "Reply-To", "Message-ID", "References"

/*
 * RFC headers output for ^ARFC level 3 (all RFC headers, excluding some)
 */
#define RFC_LVL_3_HEADERS \
    "NNTP-Posting-Date", \
    "NNTP-Posting-Host", \
    "X-Trace", \
    "X-Complaints-To", \
    "Path", \
    "Cache-Post-Path", \
    "X-Cache", \
    "Distribution", \
    "Received", \
    "Supersedes", \
    "X-Flags", \
    "X-MimeOLE", \
    "X-Priority", \
    "X-MSMail-Priority", \
    "X-Accept-Language", \
    "X-Mailer", \
    "X-Newsreader", \
    "X-Gateway", \
    "User-Agent", \
    "X-GateSoftware", \
    "In-Reply-To", \
    "From", \
    "Reply-To", \
    "Sender", \
    "To", \
    "CC", \
    "X-Comment-To", \
    "Newsgroups", \
    "Subject", \
    "Date", \
    "Organization", \
    "Lines", \
    "Message-ID", \
    "References", \
    "Mime-Version", \
    "Content-Type", \
    "Content-Transfer-Encoding", \
    "Xref", \
    "X-Original-Date", \
    "X-FTN-Kludge", \
    "X-FTN-Tearline", \
    "X-FTN-Origin", \
    "X-FTN-Seen-By", \
    "X-FTN-Path"

/*
 * Open modes for fopen(), binary for system requiring this.
 */
#ifdef DO_BINARY
#define R_MODE		"rb"
#define W_MODE		"wb"
#define A_MODE		"ab"
#define RP_MODE	"r+b"
#define WP_MODE	"w+b"
#define AP_MODE	"a+b"
#else
#define R_MODE		"r"
#define W_MODE		"w"
#define A_MODE		"a"
#define RP_MODE	"r+"
#define WP_MODE	"w+"
#define AP_MODE	"a+"
#endif /**DO_BINARY**/
#define R_MODE_T	"r"
#define W_MODE_T	"w"
#define A_MODE_T	"a"
#define RP_MODE_T	"r+"
#define WP_MODE_T	"w+"
#define AP_MODE_T	"a+"

/*
 * Format strings for dates
 */
#define DATE_LOG	"%b %d %H:%M:%S"

#define DATE_DEFAULT	"%a, %d %b %Y %H:%M:%S %O"
#define DATE_NEWS	"%a, %d %b %Y %H:%M:%S %O"
#define DATE_MAIL	"%a, %d %b %Y %H:%M:%S %O (%Z)"
#define DATE_FROM	"%a %b %d %H:%M:%S %Y"

#define DATE_FTS_0001	"%d %b %y  %H:%M:%S"
#define DATE_TICK_PATH	"%a %b %d %H:%M:%S %Y %Z"
#ifndef FTS_VIA
#define DATE_VIA	"%a %b %d %Y at %H:%M:%S %Z"
#else
//
//  %a  Abbr. weekly
//  %b  Abbr. Month
//  %d  Day of month
//  %H  Hour (local 24h)
//  %G  Hour (UTC 24h)
//  %V  Minute (UTC)
//  %U  "UTC" word
//  %j  Day of year
//  %m  Month
//  %M  Minutes (local)
//  %S  Second
//  %w  Day of week
//  %x  Date (Mon DD YYYY)
//  %X  Time (HH:MM:SS)
//  %y  Year (00-99)
//  %Y  Year (1900...)
//  %Z  Timezone (example: MSK)
//  %N  Time diff to TZUTC (if diff +03:30 - "+0330")
//  %O  Time diff to UTC (if diff +03:30 - "0330")
#define DATE_VIA	"%Y%m%d.%H%M%S.%N"
#endif                          /* FTS_VIA */
#define DATE_SPLIT	"%d %b %y %H:%M:%S"

/*
 * Product code for packets generated by FIDOGATE, 0xfe is used because
 * this code is reserved for new products, when the code numbers ran out.
 */
#define PRODUCT_CODE	0xfe

/*
 * Hard limits compiled into FIDOGATE
 */
#define MAXADDRESS	32          /* Max. # of FTN address in CONFIG */

#define MAXDOSDRIVE	16          /* Max. # of DOS drives in CONFIG */

#ifndef MAXPATH                 /* Already defined by DJGPP */
#define MAXPATH	128             /* Max. size of path names */
#endif

#define MAXINETADDR	128         /* Max. size of an Internet address */

#define MAXUSERNAME	128         /* Max. size of an user name */

#define MAXOPENFILES	10      /* Max. # of open packet files used
                                 * by ftntoss/ftnroute, this value
                                 * should work on all supported
                                 * systems, it can be incremented with
                                 * ftntoss/ftnroute's -M option */

#ifdef HAVE_ICONV
#define INTERNAL_CHARSET "utf-8"
#else
#define INTERNAL_CHARSET "cp866"
#endif                          /* HAVE_ICONV */

#define INTERNAL_ENCODING "8bit"

#endif                          /* FIDOGATE_CONSTANTS_H */
