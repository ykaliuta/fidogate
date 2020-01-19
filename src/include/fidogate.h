/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway software UNIX <-> FIDO
 *
 * $Id: fidogate.h,v 5.4 2007/01/10 00:10:37 anray Exp $
 *
 * Common header file
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

/*
 * Configuration header files
 */
#include "config.h"
#include "paths.h"              /* Automatically generated by Makefile */
#include "constants.h"

/*
 * heavy includin' ...
 */

/***** System *****/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif                          /* HAVE_UNISTD_H */
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif                          /* HAVE_FCNTL_H */
#ifdef OS2
#include <io.h>
#include <process.h>
#endif
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif                          /* HAVE_TIME_H */
#include <errno.h>
#include <dirent.h>

/***** (MSDOS, OS/2, Windows) ************************************************/
#ifdef MSDOS                    /* MSDOS, DJGPP GNU gcc */
#define  DO_DOSIFY
#endif
#ifdef __EMX__                  /* OS/2, EMX GNU gcc */
#define  DO_DOSIFY
#endif
#ifdef __CYGWIN32__             /* GNU-Win32 Beta 20.1 */
#define  DO_DOSIFY
#endif

/* Reset some #define's based on system config */
#ifndef  HAVE_LINK
#undef  NFS_SAFE_LOCK_FILES
#endif

#ifndef USE_SYSLOG
#undef  HAVE_SYSLOG
#endif

#if defined(HAVE_SYS_MOUNT_H) || defined(HAVE_SYS_PARAM_H)
#undef  HAVE_SYS_VFS_H
#undef  HAVE_SYS_STATFS_H
#undef  HAVE_SYS_STATVFS_H
#endif

/*
 * We supply our own version of the toupper()/tolower()
 * macros, because the exact behaviour of those in
 * <ctype.h> varies among systems.
 */
#undef _toupper
#undef _tolower
#undef toupper
#undef tolower

#define _toupper(c) ((c)-'a'+'A')
#define _tolower(c) ((c)-'A'+'a')
#define toupper(c)  (islower(c) ? _toupper(c) : (c))
#define tolower(c)  (isupper(c) ? _tolower(c) : (c))

#define MIN(a,b) ({				\
	typeof(a) __a = a;			\
	typeof(b) __b = b;			\
	__a < __b ? __a : __b;			\
		})

/*
 * Function declarations for systems like SUNOS, where the standard
 * header files don't have them.
 */
#include "declare.h"

/*
 * Data types
 */
#include "node.h"               /* Node, LON definitions */
#include "packet.h"             /* Packet, Message definitions */
#include "structs.h"

/*
 * Function prototypes
 */
#include "prototypes.h"
#include "cf_funcs.h"

#ifdef HAVE_ICONV
#include <iconv.h>
#endif
