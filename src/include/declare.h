/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 *
 * Declaration header for not-so-ANSI systems
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
 * errno might not be declared extern in errno.h
 */
extern int errno;

#ifdef __sun__ /**************************************************************/
#ifndef __svr4__
/*
 * SUNOS 4.1.x, GNU gcc 2.x
 */

/*
 * Non-standard functions
 */
/* sys/time.h */
int gettimeofday( /*struct timeval *, struct timezone * */ );

/* unistd.h */
int lockf(int, int, long);

/*
 * Standard-C functions undeclared in header files
 */

/* stdio.h */
int _filbuf();                  /* Internal, no standard */
int _flsbuf();                  /* Internal, no standard */

int fclose(FILE *);
int fflush(FILE *);
int fprintf(FILE *, const char *, ...);
int fputs(const char *, FILE *);
size_t fread(void *, size_t, size_t, FILE *);
int fseek(FILE *, long, int);
size_t fwrite(const void *, size_t, size_t, FILE *);
void perror(const char *);
int printf(const char *, ...);
int puts(const char *);
int rename(const char *, const char *);
void rewind(FILE *);
int sscanf(char *, const char *, ...);
int ungetc(int, FILE *);
int vfprintf(FILE *, const char *, va_list);
int vsprintf(char *, const char *, va_list);
/* Not ANSI-C or POSIX but missing anyway ... */
int pclose(FILE *);

/* string.h */
/* Not ANSI-C or POSIX */
int strcasecmp(char *, char *);
int strncasecmp(char *, char *, int);

/* stdlib.h */
int system(const char *);

/* time.h */
time_t time(time_t *);
size_t strftime(char *, size_t, const char *, const struct tm *);
/* Looks like SUN's timelocal() is the same as Standard-C mktime() */
#ifndef HAVE_MKTIME
#define mktime  timelocal
#endif                          /* HAVE_MKTIME */

#endif  /**svr4***************************************************************/
#endif  /**sun****************************************************************/
