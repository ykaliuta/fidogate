/*:ts=8*/
/*****************************************************************************
 * FIDOGATE --- Gateway software UNIX Mail/News <-> FIDO NetMail/EchoMail
 *
 * Global version number, patch level
 *****************************************************************************/

#ifndef FIDOGATE_VERSION_H_
#define FIDOGATE_VERSION_H_

#include <config.h>
#define VERSION_MAJOR	5
#define VERSION_MINOR	8

#ifdef GIT_HASH_STR
#define EXTRAVERSION	"-g" GIT_HASH_STR
#else
#define EXTRAVERSION	""
#endif

#define STATE		"unstable"

#endif
