
/* Name of this package (needed by automake) */
#undef PACKAGE

/* Version of this package (needed by automake) */
#undef VERSION

#undef DEFAULT_MAILER

/* Define if you have the gettimeofday function.  */
#undef HAVE_GETTIMEOFDAY

/* Define if you have the link function.  */
#undef HAVE_LINK

/* Define if you have the mkdir function.  */
#undef HAVE_MKDIR

/* Define if you have the mktime function.  */
#undef HAVE_MKTIME

/* Define if you have the sprintf function.  */
#undef HAVE_SPRINTF

/* Define if you have the strcasecmp function.  */
#undef HAVE_STRCASECMP

/* Define if you have the strerror function.  */
#undef HAVE_STRERROR

/* Define if you have the stricmp function.  */
#undef HAVE_STRICMP

/* Define if you have the strspn function.  */
#undef HAVE_STRSPN

/* Define if you have the strstr function.  */
#undef HAVE_STRSTR

/* Define if you have the syslog function.  */
#undef HAVE_SYSLOG

/* Define if you have the <dirent.h> header file.  */
#undef HAVE_DIRENT_H

/* Define if you have the <fcntl.h> header file.  */
#undef HAVE_FCNTL_H

/* Define if you have the <limits.h> header file.  */
#undef HAVE_LIMITS_H

/* Define if you have the <malloc.h> header file.  */
#undef HAVE_MALLOC_H

/* Define if you have the <ndir.h> header file.  */
#undef HAVE_NDIR_H

/* Define if you have the <sys/dir.h> header file.  */
#undef HAVE_SYS_DIR_H

/* Define if you have the <sys/mount.h> header file.  */
#undef HAVE_SYS_MOUNT_H

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/param.h> header file.  */
#undef HAVE_SYS_PARAM_H

/* Define if you have the <sys/statfs.h> header file.  */
#undef HAVE_SYS_STATFS_H

/* Define if you have the <sys/statvfs.h> header file.  */
#undef HAVE_SYS_STATVFS_H

/* Define if you have the <sys/time.h> header file.  */
#undef HAVE_SYS_TIME_H

/* Define if you have the <sys/vfs.h> header file.  */
#undef HAVE_SYS_VFS_H

/* Define if you have the <sysexits.h> header file.  */
#undef HAVE_SYSEXITS_H

/* Define if you have the <syslog.h> header file.  */
#undef HAVE_SYSLOG_H

/* Define if you have the <time.h> header file.  */
#undef HAVE_TIME_H

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H


/***** General configuration *************************************************/

/*
 * Generate local FTN addresses, e.g.
 *     user_name%p.f.n.z@host.domain
 * instead of
 *     user_name@p.f.n.z.domain
 */
#undef LOCAL_FTN_ADDRESSES

/*
 * Create Binkley-style BSY files for all outbound operations
 */
#undef DO_BSY_FILES

/*
 * Create lock files / BSY files in an NFS-safe way (see man 2 open)
 */
#undef NFS_SAFE_LOCK_FILES

/*
 * Default max. message size for FIDO. Due to some more brain damage
 * in FIDONET programs we have to split larger messages into several
 * smaller ones. May be set with the -M option in AREAS or MaxMsgSize
 * in DEFAULT_CONFIG_GATE.
 */
/* < 16 K */
/* #define MAXMSGSIZE	14000 */
/* < 32 K */
/* #define MAXMSGSIZE	30000 */
#undef MAXMSGSIZE
/*
 * Domain for invalid FTN addresses
 */
#undef FTN_INVALID_DOMAIN

/*
 * syslog facility used for logging if logfile == "syslog"
 * (only for HAS_SYSLOG defined)
 */
#undef FACILITY

/*
 * Default assumed charset for Fido messages (see also DefaultCharset)
 */
#undef CHARSET_STDFTN

/*
 * Default assumed charset for RFC messages if without MIME headers
 */
#undef CHARSET_STDRFC

/*
 * Default charset for RFC messages with forced 7bit encoding
 */
#undef CHARSET_STD7BIT


/***** ftn2rfc configuration ************************************************/

/*
 * Rewrite addresses found in ALIASES so that the sender's address is the
 * gateway address. The reverse direction requires suitable MTA aliases.
 */
#undef ALIASES_ARE_LOCAL

/***** rfc2ftn configuration ************************************************/

/** Passthru operation for NetMail: FIDO->Internet->FIDO **/
#undef PASSTHRU_NETMAIL
/** Passthru operation for EchoMail: FIDO->Internet->FIDO **/
/*
 * Implemented, but requires ftntoss run after rfc2ftn to sort SEEN-BY
 */
#undef PASSTHRU_ECHOMAIL

/** Support newsserver SN ***************************************************/
/*
 * Enable SN-style active_lookup
 */
#undef SN

/***** AI patches configuration (see README.ai) *****************************/
/*
 * Add -a option to HOSTS entries, useful only with PASSTHRU_NET/ECHOMAIL
 */
#undef AI_1

/*
 * Enable best AKA select
 */
#undef BEST_AKA

/*
 * Enable ACL for FTN links
 */
#undef FTN_ACL

/*
 * Put file descriptions into `.desc/%s.desc' file instead of files.bbs
 */
 // .../fileecho/.desc/file.zip.desc
// #define DESC_DIR	".desc"
// .../fileecho/file.zip.desc
// #define DESC_DIR	""

#undef DESC_DIR

/*
 * Add `X-FTN-From' header in gated echomail.
 */
#undef X_FTN_FROM_ECHOMAIL

/*
 * Added the feature "spy forwards".
 */
#undef SPYES

/*
 * Restricted access to areafix command `%listall'
 */
#undef AF_LISTALL_RESTRICTED

/*
 * Recode file descriptions
 */
#undef RECODE_FILE_DESC

/*
 * Enable areafix command `%avail', change behaviour of areafix
 * command `%list'
 */
#undef AF_AVAIL

/*
 * Enable passthrought filearea
 */
#undef FECHO_PASSTHROUGHT

/*
 * Enable if use fileboxes for fileecho
 */
#undef USE_FILEBOX

/*
 * Enable if no ftntick CRC control
 */
#undef FTNTICK_NOCRC

/*
 * Enable if no insert Origin line if it absence
 */
#undef NOINSERT_ORGANIZATION

/*
 * If Origin line is not present, include link address
 */
#undef INSERT_ORIGIN

/*
 * Don't put insecure mail into dupe database
 */
#undef INSECURE_DONT_PUT_INTO_DUPE_DB

/*
 * For the security reason enabled additional addresses checking in the  
 * .pkt header, message header, origin and echomail ^aPATH.
 * If the last address in PATH is not equal to address in the pkt header, 
 * then the address from the pkt header is inserted into PATH.
 */
#define SECURITY

/*
 * Do'nt put info about create to log if echo created by forward reques.
 */
#undef CREATE_LOG_FORWREQ

/*
 * Change ^AVia format from:
 *   ^AVia FIDOGATE/ftntoss 2:5030/1229.0, 20010405.164323.MSK
 * to:
 *   ^AVia 2:5030/1229.0 @20010405.164323.MSK FIDOGATE/ftntoss
 */
#define FTS_VIA

/*
 * Change areafix answer when subscibing to area from `subscribe' to `ok'.
 */
#undef ANSWER_OK

/*
 * Set fido-style msgid
 */
#undef FIDO_STYLE_MSGID

/*
 * Set msgid correspond data base
 */
#undef DBC_HISTORY

/*
 * Delete MIME if rfc status set to 2
 */
#undef DEL_MIME_IF_RFC2

/*
 * Enable areafix send echo status
 */
#undef AFSEND_ECHO_STATUS

/*
 * Enable subscribe autorization only if node listed in zonegate
 */
#undef SUBSCRIBE_ZONEGATE

/*
 * Paste domain name to Origin message line
 */
#undef DOMAIN_TO_ORIGIN

/*
 * Enable paste ^PID (mail or news reader name) and ^TID (fidogate version)
 * kludge
 */
#undef PID_READER_TID_GTV

/*
 * Enable tic dupe check
 */
#undef TIC_HISTORY

/*
 * Disable tosting netmail and set directory for incoming and outgoing pkt's
 */
#undef DO_NOT_TOSS_NETMAIL 

/*
 * Ignore `From:' field if REPLYTO or REPLYADDR was found
 */
#undef IGNORE_FROM_IF_REPLY

/*
 * Separate outouing message to individual pkt file
 */
#undef SEP_RFC2FTN_PKT

/*
 * Enable set limit for subscribe areas
 */
#define SUB_LIMIT

/*
 * Enable overwrite hardlink if file already exist
 */
#undef OVERWRITEN_FECHO_FILE_TO_LINK

/*
 * Enable lookup to INN active file
 */
#undef ACTIVE_LOOKUP

/*
 * Recode all RFC headers
 */
#define RECODE_ALL_RFC

/*
 * Enable use old batcher in runinc program
 */
#undef OLD_BATCHER

/*
 * Enable use send_fidogate function in runinc program
 */
#undef USE_RUNINC_SFGT

/*
 * Enable if you want use syslog for log messages
 */
#undef USE_SYSLOG

/*
 * Enable old style tosting
 */
#undef OLD_TOSS

/*
 * Fix bad year in pkt's
 */
#undef FIX_BAD_PKT_YEAR

/***** System dependend configuration ***************************************
 *
 *   HAVE_FCNTL_LOCK		Do you have file locking with fcntl()
 *
 *   HAVE_GETTIMEOFDAY	        Do you have gettimeofday()?
 *
 *   HAVE_TM_GMTOFF		Does your (struct tm) have a tm_gmtoff field?
 *
 *   HAVE_SYSEXITS_H             Do you have sysexits.h?
 *
 *   HAVE_TM_ZONE		Does your (struct tm) have a tm_zone field?
 *
 *   HAVE_STRFTIME		Do you have strftime()?
 *
 *   HAVE_TZNAME			Do you have extern char *tzname[2]?
 *
 *   HAVE_STRCASECMP		Do you have strcasecmp(), strncasecmp()?
 *
 *   HAVE_STRICMP		Do you have stricmp(), strnicmp()?
 *
 *
 *   HAVE_STRERROR               Do you have strerror()?
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
 *   HAVE_SPRINTF               sprintf(), vsprintf() supported
 *
 *   HAVE_SYSLOG		syslogd, syslog(), vsyslog() supported
 *
 *   HAVE_HARDLINKS		hardlinks supported by link() and filesystem
 *
 *   HAS_POSIX_REGEX		POSIX regcomp(), regexec() etc. supported
 *
 *   HAVE_SYS_VFS_H		Do you have statfs() in sys/vfs.h
 *
 *   HAVE_SYS_STATFS_H		Do you have statfs() in sys/statfs.h
 *
 *   HAVE_SYS_STATVFS_H		Do you have statfs() in sys/statvfs.h
 *
 *   HAVE_SYS_MOUNT_H		Do you have statfs() in sys/mount.h
 */

/***** Unix and derivates ***************************************************/

# undef  HAVE_FCNTL_H
# undef  HAVE_GETTIMEOFDAY
# undef  HAVE_SYSEXITS_H
# undef  HAVE_STRCASECMP
# undef  HAVE_STRICMP
# undef  HAVE_STRERROR
# undef  HAVE_SPRINTF
# undef  HAVE_TM_GMTOFF
# undef  HAVE_STRFTIME
# undef  HAVE_TM_ZONE
# undef  HAVE_TZNAME
#ifdef USE_SYSLOG
# undef  HAVE_SYSLOG
#endif
# undef  HAVE_SYS_MOUNT_H
# undef  HAVE_SYS_PARAM_H
#if !defined(HAVE_SYS_MOUNT_H) && !defined(HAVE_SYS_PARAM_H)
# undef  HAVE_SYS_VFS_H
# undef  HAVE_SYS_STATFS_H
# undef  HAVE_SYS_STATVFS_H
#endif
# undef  HAVE_TIME_H
# undef  HAVE_SYS_TIME_H

/* Standard config: POSIX UNIX */
# undef HAVE_LINK

/***** End of configuration *************************************************/

#undef AREAFIXMAXSTR

/* Define if using iconv */
#undef HAVE_ICONV

/* Define if using alloca.c.  */
#undef C_ALLOCA

/* Define to empty if the keyword does not work.  */
#undef const

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
#undef CRAY_STACKSEG_END

/* Define if you have alloca, as a function or macro.  */
#undef HAVE_ALLOCA

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#undef HAVE_ALLOCA_H

/* Define if you don't have vprintf but do have _doprnt.  */
#undef HAVE_DOPRNT

/* Define if you have the strftime function.  */
#undef HAVE_STRFTIME

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#undef HAVE_SYS_WAIT_H

/* Define if your struct tm has tm_zone.  */
#undef HAVE_TM_ZONE

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#undef HAVE_TZNAME

/* Define if utime(file, NULL) sets file's timestamp to the present.  */
#undef HAVE_UTIME_NULL

/* Define if you have the vprintf function.  */
#undef HAVE_VPRINTF

/* Define to `long' if <sys/types.h> doesn't define.  */
#undef off_t

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define as the return type of signal handlers (int or void).  */
#undef RETSIGTYPE

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
#undef STACK_DIRECTION

/* Define if you have the ANSI C header files.  */
#undef STDC_HEADERS

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#undef TIME_WITH_SYS_TIME

/* Define if your <sys/time.h> declares struct tm.  */
#undef TM_IN_SYS_TIME

#undef DEFAULT_MAILER

#undef HAVE_LIBDL
