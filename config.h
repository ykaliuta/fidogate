/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway software UNIX <-> FIDO
 *
 * $Id: config.h,v 4.49 2004/08/22 20:19:09 n0ll Exp $
 *
 * Configuration header file
 *
 *****************************************************************************
 * Copyright (C) 1990-2002
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

/***** General configuration *************************************************/

/*
 * Generate local FTN addresses, e.g.
 *     user_name%p.f.n.z@host.domain
 * instead of
 *     user_name@p.f.n.z.domain
 */
/* #define LOCAL_FTN_ADDRESSES */

/*
 * Create Binkley-style BSY files for all outbound operations
 */
#define DO_BSY_FILES

/*
 * Create lock files / BSY files in an NFS-safe way (see man 2 open)
 */
#define NFS_SAFE_LOCK_FILES

/*
 * Create 4D outbound filenames for AmigaDOS mailers,
 * Z.N.F.P.flo / Z.N.F.P.mo0
 */
/* #define AMIGADOS_4D_OUTBOUND */

/*
 * Default max. message size for FIDO. Due to some more brain damage
 * in FIDONET programs we have to split larger messages into several
 * smaller ones. May be set with the -M option in AREAS or MaxMsgSize
 * in DEFAULT_CONFIG_GATE.
 */
/* < 16 K */
#define MAXMSGSIZE	14000
/* < 32 K */
/* #define MAXMSGSIZE	30000 */

/*
 * Domain for invalid FTN addresses
 */
#define FTN_INVALID_DOMAIN "INVALID_FTN_ADDRESS"

/*
 * syslog facility used for logging if logfile == "syslog"
 * (only for HAS_SYSLOG defined)
 */
#define FACILITY	LOG_LOCAL0

/*
 * Default assumed charset for Fido messages (see also DefaultCharset)
 */
#define CHARSET_STDFTN	"ibmpc"

/*
 * Default assumed charset for RFC messages if without MIME headers
 */
#define CHARSET_STDRFC	"iso-8859-1"

/*
 * Default charset for RFC messages with forced 7bit encoding
 */
#define CHARSET_STD7BIT	"us-ascii"



/***** ftn2rfc configuration ************************************************/

/*
 * Rewrite addresses found in ALIASES so that the sender's address is the
 * gateway address. The reverse direction requires suitable MTA aliases.
 */
/* #define ALIASES_ARE_LOCAL */



/***** rfc2ftn configuration ************************************************/

/** Passthru operation for NetMail: FIDO->Internet->FIDO **/
/* #define PASSTHRU_NETMAIL */
/** Passthru operation for EchoMail: FIDO->Internet->FIDO **/
/*
 * Implemented, but requires ftntoss run after rfc2ftn to sort SEEN-BY
 */
/* #define PASSTHRU_ECHOMAIL */

/*
 * Don't pass news control messages to FTN
 */
#define NO_CONTROL



/***** AI patches configuration (see README.ai) *****************************/
/*
 * Add -a option to HOSTS entries, useful only with PASSTHRU_NET/ECHOMAIL
 */
/* #define AI_1 */

/*
 * New `DeleteSeenBy' and `DeletePath' options for ftntoss
 */
/* #define AI_3 */

/*
 * Check on 8bit in subject, origin and tearline
 */
#define AI_5

/* 
 * New `AddressIsLocalForXPost' option in config.common file (see
 * example). In this variable one can specify wildcards to determine
 * which users are allowed to crosspost to areas marked with `-l' flag
 * in `areas' file.
 */
/* #define AI_6 */

/*
 * Added ACL support. This feature allows one to describe the
 * correspondence between sender's email address and newsgroups to
 * which this sender is allowed to write. One can use wildcards as
 * email and newsgroups. The newsgroups list is in INN like style. See
 * the comments in `acl' file in the `examples' directory for further
 * information.
 */
/* #define AI_8 */



/***** System dependend configuration ***************************************
 *
 *   HAS_FCNTL_LOCK		Do you have file locking with fcntl()
 *
 *   HAS_GETTIMEOFDAY	        Do you have gettimeofday()?
 *
 *   HAS_TM_GMTOFF		Does your (struct tm) have a tm_gmtoff field?
 *
 *   HAS_SYSEXITS_H             Do you have sysexits.h?
 *
 *
 * Define only one of HAS_TM_ZONE, HAS_STRFTIME, HAS_TZNAME!!!
 *
 *   HAS_TM_ZONE		Does your (struct tm) have a tm_zone field?
 *
 *   HAS_STRFTIME		Do you have strftime()?
 *
 *   HAS_TZNAME			Do you have extern char *tzname[2]?
 *
 *
 * Define only one of HAS_STRCASECMP, HAS_STRICMP!!!
 *
 *   HAS_STRCASECMP		Do you have strcasecmp(), strncasecmp()?
 *
 *   HAS_STRICMP		Do you have stricmp(), strnicmp()?
 *
 *
 *   HAS_STRERROR               Do you have strerror()?
 *
 *
 *   DO_BINARY			Open files in binary mode
 *
 *   DO_DOSIFY			DOSify program names for execution
 *
 *
 *   RECEIVED_BY_MAILER "Received: by NeXT.Mailer"
 *				Define this if your mail system always
 *				generates something like
 *				"Received: by NeXT.Mailer"
 *
 *   HAS_SYSLOG		        syslogd, syslog(), vsyslog() supported
 *
 *   HAS_SNPRINTF               snprintf(), vsnprintf() supported
 *
 *   HAS_HARDLINKS		hardlinks supported by link() and filesystem
 *
 *   HAS_POSIX_REGEX		POSIX regcomp(), regexec() etc. supported
 */

/***** Unix and derivates ***************************************************/
/* Standard config: POSIX UNIX */
# define HAS_FCNTL_LOCK
# undef  HAS_GETTIMEOFDAY
# undef  HAS_TM_GMTOFF
# undef  HAS_SYSEXITS_H
# undef  HAS_TM_ZONE
# define HAS_STRFTIME
# undef  HAS_TZNAME
# undef  HAS_STRCASECMP
# undef  HAS_STRICMP
# define HAS_STRERROR
# undef  DO_BINARY
# undef  DO_DOSIFY
# undef  HAS_SYSLOG		/* syslog(), vsyslog() not supported */
# undef  HAS_SNPRINTF		/* snprintf(), vsnprintf() not supported */
# define HAS_HARDLINKS
# undef  HAS_POSIX_REGEX

#ifdef __sun__
# ifdef __svr4__		/* Solaris 2.x aka SUNOS 5.x, GNU gcc */
#  define HAS_FCNTL_LOCK
#  define HAS_GETTIMEOFDAY
#  undef  HAS_TM_GMTOFF
#  define HAS_SYSEXITS_H
#  undef  HAS_TM_ZONE
#  define HAS_STRFTIME
#  undef  HAS_TZNAME
#  define HAS_STRCASECMP
#  undef  HAS_STRICMP
#  define HAS_STRERROR
#  undef  DO_BINARY
#  undef  DO_DOSIFY
#  define HAS_SYSLOG
#  define HAS_SNPRINTF
#  define HAS_HARDLINKS
#  define HAS_POSIX_REGEX
# else				/* SUNOS 4.1.x, GNU gcc */
#  define HAS_FCNTL_LOCK
#  define HAS_GETTIMEOFDAY
#  define HAS_TM_GMTOFF
#  define HAS_SYSEXITS_H
#  undef  HAS_TM_ZONE
#  define HAS_STRFTIME
#  undef  HAS_TZNAME
#  define HAS_STRCASECMP
#  undef  HAS_STRICMP
#  undef  HAS_STRERROR
#  undef  DO_BINARY
#  undef  DO_DOSIFY
#  define HAS_SYSLOG
#  undef  HAS_SNPRINTF
#  define HAS_HARDLINKS
#  undef  HAS_POSIX_REGEX
#  endif /**svr4**/
#endif

#ifdef __linux__		/* Linux */
# define HAS_FCNTL_LOCK
# define HAS_GETTIMEOFDAY
# undef  HAS_TM_GMTOFF
# define HAS_SYSEXITS_H
# undef  HAS_TM_ZONE
# define HAS_STRFTIME
# undef  HAS_TZNAME
# define HAS_STRCASECMP
# undef  HAS_STRICMP
# define HAS_STRERROR
# undef  DO_BINARY
# undef  DO_DOSIFY
# define HAS_SYSLOG
# define HAS_SNPRINTF
# define HAS_HARDLINKS
# define HAS_POSIX_REGEX
#endif

#ifdef __FreeBSD__		/* FreeBSD 2.1.6., GNU gcc */
# define HAS_FCNTL_LOCK
# define HAS_GETTIMEOFDAY
# define HAS_TM_GMTOFF
# define HAS_SYSEXITS_H
# define HAS_TM_ZONE
# define HAS_STRFTIME
# undef  HAS_TZNAME
# define HAS_STRCASECMP
# undef  HAS_STRICMP
# undef  HAS_STRERROR		/* ? */
# undef  DO_BINARY
# undef  DO_DOSIFY
# define HAS_SYSLOG
# define HAS_SNPRINTF		/* ? */
# define HAS_HARDLINKS
# undef  HAS_POSIX_REGEX	/* ? */
#endif

#ifdef ISC			/* ISC 3.x, GNU gcc, -DISC necessary */
# define HAS_FCNTL_LOCK
# define HAS_GETTIMEOFDAY
# undef  HAS_TM_GMTOFF
# undef  HAS_SYSEXITS_H		/* ? */
# undef  HAS_TM_ZONE
# undef  HAS_STRFTIME
# define HAS_TZNAME
# undef  HAS_STRCASECMP		/* ? */
# undef  HAS_STRICMP
# undef  HAS_STRERROR		/* ? */
# undef  DO_BINARY
# undef  DO_DOSIFY
# define HAS_SYSLOG
# undef  HAS_SNPRINTF
# define HAS_HARDLINKS
# undef  HAS_POSIX_REGEX
#endif

#ifdef __NeXT__                 /* NEXTSTEP 3.3 (Intel only?) */
# define HAS_GETTIMEOFDAY
# define HAS_TM_GMTOFF
# define HAS_SYSEXITS_H
# define HAS_TM_ZONE
# define HAS_STRFTIME
# define HAS_TZNAME
# undef  HAS_STRCASECMP
# undef  HAS_STRICMP
# undef  HAS_STRERROR
# undef  DO_BINARY
# define RECEIVED_BY_MAILER "Received: by NeXT.Mailer"
# undef  DO_DOSIFY
# define HAS_SYSLOG
# undef  HAS_SNPRINTF
# define HAS_HARDLINKS
# undef  HAS_POSIX_REGEX
#endif /* __NeXT__ */


/***** Other (MSDOS, OS/2, Windows) *****************************************/
#ifdef MSDOS			/* MSDOS, DJGPP GNU gcc */
# undef  HAS_FCNTL_LOCK
# define HAS_TM_GMTOFF
# undef  HAS_SYSEXITS_H		/* ? */
# undef  HAS_TM_ZONE
# define HAS_GETTIMEOFDAY
# define HAS_STRFTIME
# undef  HAS_STRCASECMP
# define HAS_STRICMP
# undef  HAS_STRERROR		/* ? */
# define DO_BINARY
# define DO_DOSIFY
# undef  HAS_SYSLOG		/* syslog(), vsyslog() not supported */
# undef  HAS_SNPRINTF
# undef  HAS_HARDLINKS
# undef  HAS_POSIX_REGEX
#endif

#ifdef __EMX__			/* OS/2, EMX GNU gcc */
# ifndef OS2
#  define OS2
# endif
#endif
#ifdef OS2
# undef  HAS_FCNTL_LOCK
# undef  HAS_SYSEXITS_H
# define HAS_GETTIMEOFDAY
# define HAS_STRFTIME
# undef  HAS_STRCASECMP
# define HAS_STRICMP
# define HAS_STRERROR
# define DO_BINARY
# define DO_DOSIFY
# undef  HAS_SYSLOG		/* syslog(), vsyslog() not supported */
# define HAS_SNPRINTF
# undef  HAS_HARDLINKS
# undef  HAS_POSIX_REGEX
#endif

#ifdef __CYGWIN32__		/* GNU-Win32 Beta 20.1 */
# undef  HAS_FCNTL_LOCK
# undef  HAS_GETTIMEOFDAY
# undef  HAS_TM_GMTOFF
# undef  HAS_SYSEXITS_H
# undef  HAS_TM_ZONE
# define HAS_STRFTIME
# undef  HAS_TZNAME
# define HAS_STRCASECMP
# undef  HAS_STRICMP
# define HAS_STRERROR
# define DO_BINARY
# undef  DO_DOSIFY
# undef  HAS_SYSLOG		/* syslog(), vsyslog() not supported */
# undef  HAS_SNPRINTF		/* in stdio.h, but not in libraries */
# undef  HAS_HARDLINKS
# undef  HAS_POSIX_REGEX
#endif

/* Reset some #define's based on system config */
#ifndef HAS_HARDLINKS
# undef  NFS_SAFE_LOCK_FILES
#endif

/***** End of configuration *************************************************/



/***** ^AMSGID/Message-ID configuration *************************************/

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
#define PACKET_MODE	0600		/* Mode for outbound packets */
#define BSY_MODE	0644		/* Mode for BSY files */
#define FLO_MODE	0644		/* Mode for FLO files */
#define DATA_MODE	0600		/* Mode for ffx data files */
#define DIR_MODE	0755		/* Mode for directories */
#define CONF_MODE	0644		/* Mode for written config files */

/*
 * RFC headers recognized at beginning of FTN message body
 */
#define FTN_RFC_HEADERS \
    "From:", "Reply-To:", "UUCPFROM:", "To:", "Cc:", "Bcc:", \
    "Newsgroups:", "Sender:", "Content-Transfer-Encoding:", \
    "Header-To:", "Header-Cc:"

/*
 * RFC headers output for ^ARFC level 1 (partial RFC headers)
 */
#define RFC_LVL_1_HEADERS \
    "From", "Reply-To", "To", "Cc", "Newsgroups", "Date", \
    "Sender", "Resent-From", "Return-Path", \
    "Next-Attachment"

/*
 * Open modes for fopen(), binary for system requiring this.
 */
#ifdef DO_BINARY
# define R_MODE		"rb"
# define W_MODE		"wb"
# define A_MODE		"ab"
# define RP_MODE	"r+b"
# define WP_MODE	"w+b"
# define AP_MODE	"a+b"
#else
# define R_MODE		"r"
# define W_MODE		"w"
# define A_MODE		"a"
# define RP_MODE	"r+"
# define WP_MODE	"w+"
# define AP_MODE	"a+"
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
#define DATE_VIA	"%a %b %d %Y at %H:%M:%S %Z"
#define DATE_SPLIT	"%d %b %y %H:%M:%S"

/*
 * Product code for packets generated by FIDOGATE, 0xfe is used because
 * this code is reserved for new products, when the code numbers ran out.
 */
#define PRODUCT_CODE	0xfe

/*
 * Hard limits compiled into FIDOGATE
 */
#define MAXADDRESS	32		/* Max. # of FTN address in CONFIG */

#define MAXDOSDRIVE	16		/* Max. # of DOS drives in CONFIG */

#ifndef MAXPATH				/* Already defined by DJGPP */
# define MAXPATH	128		/* Max. size of path names */
#endif

#define MAXINETADDR	128		/* Max. size of an Internet address */

#define MAXUSERNAME	128		/* Max. size of an user name */

#define MAXOPENFILES	10		/* Max. # of open packet files used
					 * by ftntoss/ftnroute, this value
					 * should work on all supported
					 * systems, it can be incremented with
					 * ftntoss/ftnroute's -M option */
